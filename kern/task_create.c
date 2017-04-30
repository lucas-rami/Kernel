/** @file task_create.c
 *  @brief This file contains the definitions for functions related to task
 *  creation from executable files
 *  @author akanjani, lramire1
 */

#include <task_create.h>

#include <stdlib.h>

#include <context_switch.h>
#include <context_switch_asm.h>
#include <cr.h>
#include <eflags.h>
#include <elf_410.h>
#include <kernel_state.h>
#include <malloc.h>
#include <scheduler.h>
#include <syscall.h>
#include <virtual_memory.h>
#include <hash_table.h>
#include <syscalls.h>
#include <virtual_memory_defines.h>
#include <common_kern.h>
#include <asm.h>
#include <string.h>

// Debugging
#include <simics.h>

// NOTE: Temporary
#define ESP 0xFFFFFFFF
#define TRUE 1
#define FALSE 0

/* Number of registers poped during a popa instruction */
#define NB_REGISTERS_POPA 8

#define INCREASE_FRAME_COUNT eff_mutex_lock(&kernel.mutex);\
                             kernel.free_frame_count += num_frames_requested;\
                             eff_mutex_unlock(&kernel.mutex)

/** @brief  Creates a task from an executable, should only be used for the first
 *          task created in the kernel
 *
 *  The function creates evertying necessary for the first task in the kernel
 *  to run. The function get the ELF header for the given task name, allocates
 *  a kernel stack and user-space memory, create a new PCB/TCB and manually 
 *  craft an initial stack for the first task.
 *
 *  @param  task_name    A string specifying the name of the program to be 
 *                       loaded
 *
 *  @return 0 on success, a negative number on error
 */
int create_task_from_executable(char *task_name) {

  simple_elf_t elf;

  // Populate the simple_elf_t data structure
  if (load_elf_file(task_name, &elf) < 0) {
    lprintf("Error loading ELF file");
  }

  // Request the number of frames needed for this task
  unsigned num_frames_requested;
  if ((num_frames_requested = request_frames_needed_by_program(&elf)) == 0) {
    lprintf("The program %s needs more memory than is available", task_name);
    return -1;
  }

  // Allocate a kernel stack for the root thread
  void *stack_kernel = malloc(PAGE_SIZE);
  if (stack_kernel == NULL) {
    lprintf("Could not allocate kernel stack for task's root thread");
    return -1;
  }

  // Setup virtual memory for this task
  unsigned int *cr3;
  if ((cr3 = setup_vm(&elf, FIRST_TASK_TRUE)) == NULL) {
    lprintf("Task creation failed for task \"%s\"", task_name);
    free(stack_kernel);
    INCREASE_FRAME_COUNT;
    return -1;
  }
  
  // Doing this for the first user task of the system
  uint32_t esp0 = (uint32_t)(stack_kernel) + PAGE_SIZE;

  // Create new PCB for the task
  pcb_t *new_pcb = create_new_pcb();
  if (new_pcb == NULL) {
    lprintf("create_task_from_executable(): PCB initialization failed");
    free(stack_kernel);
    INCREASE_FRAME_COUNT;
    return -1;
  }

  // Create new TCB for the root thread
  tcb_t *new_tcb = create_new_tcb(new_pcb, esp0, (uint32_t)cr3, NULL, 
                                  ROOT_THREAD_TRUE);
  if (new_tcb == NULL) {
    lprintf("create_task_from_executable(): TCB initialization failed");
    free(stack_kernel);
    hash_table_remove_element(&kernel.pcbs, new_pcb);
    INCREASE_FRAME_COUNT;
    return -1;
  }

  // Loads the arguments for the new task
  char *argv[2];
  argv[0] = task_name;
  argv[1] = NULL;
  uint32_t stack_top = (uint32_t) load_args_for_new_program(argv, (unsigned int *)get_cr3(), 1);

  lprintf("\ttask_create(): Setting the init_cr3 as %p", cr3);

  // Set the created task as the kernel's init task
  kernel.init_cr3 = (uint32_t)cr3;
  kernel.init_task = new_pcb;

  // Set the number of frames requested by the task and root thread
  new_tcb->num_of_frames_requested = num_frames_requested;
  new_tcb->task->num_of_frames_requested = num_frames_requested;

  // Create EFLAGS for the user task
  uint32_t eflags = get_eflags();
  eflags |= EFL_RESV1;       // Set bit 1
  eflags &= ~EFL_AC;         // Alignment checking off
  eflags |= EFL_IF;          // Enable interrupts

  // Manually craft the stack
  unsigned int * stack_addr = (unsigned int *) esp0;
  --stack_addr;
  *stack_addr = eflags;
  --stack_addr;
  *stack_addr = stack_top;
  --stack_addr;
  *stack_addr = elf.e_entry;
  --stack_addr;
  *stack_addr = (unsigned int) new_tcb;
  --stack_addr;
  *stack_addr = (unsigned int) run_first_thread;
  --stack_addr;
  *stack_addr = (unsigned int) init_thread;
  stack_addr -= NB_REGISTERS_POPA;

  // Save stack pointer value in TCB
  new_tcb->esp = (uint32_t) stack_addr;

  // Mark the thread as runnable
  new_tcb->thread_state = THR_RUNNABLE;
  generic_node_t new_tail = {new_tcb, NULL};
  generic_node_t * node_addr = (generic_node_t *)(new_tcb->esp0 - PAGE_SIZE);
  *(node_addr) = new_tail;

  // Enqueue the current thread
  stack_queue_enqueue(&kernel.runnable_queue, node_addr);

  return 0;
}

/** @brief  Computes the number of frames that is needed for a given progam
 *
 *  The function also takes care of updating the value of frames available in
 *  user-space memory if enough frames were available before the call, if not
 *  the number of remaining frames is left unchanged.
 *
 *  @param  elf A data structure holding information (length, offset) about
 *              each section of a program
 *  
 *  @return The number of frames requested by the program if enough frames are
 *          available in user-space memory, 0 otherwise
 */
unsigned int request_frames_needed_by_program(simple_elf_t *elf) {
  if (elf == NULL) {
    lprintf("frames_needed_by_program(): Invalid argument");
    return 0;
  }
  unsigned int reduce_count = 0;
  unsigned long textstart = elf->e_txtstart;
  unsigned long textend = textstart + elf->e_txtlen;
  unsigned long rodatastart = elf->e_rodatstart;
  unsigned long rodataend = rodatastart + elf->e_rodatlen;
  unsigned long datastart = elf->e_datstart;
  unsigned long dataend = datastart + elf->e_datlen;
  unsigned long bssstart = elf->e_bssstart;
  unsigned long bssend = bssstart + elf->e_bsslen;
  if ((textstart/PAGE_SIZE) == (rodatastart/PAGE_SIZE)) {
    // Both the sections will map to the first same frame
    reduce_count++;
  } else if ((textend/PAGE_SIZE) == 
             (rodatastart/PAGE_SIZE) ||
             (textstart/PAGE_SIZE) == 
             (rodataend/PAGE_SIZE)) {
    reduce_count++;
  }

  if ((datastart/PAGE_SIZE) == (bssstart/PAGE_SIZE)) {
    // Both the sections will map to the first same frame
    reduce_count++;
  } else if ((dataend/PAGE_SIZE) == 
             (bssstart/PAGE_SIZE) ||
             (datastart/PAGE_SIZE) == 
             (bssend/PAGE_SIZE)) {
    reduce_count++;
  }

  unsigned int total_frames_reqd = 0;
  total_frames_reqd += (textend/PAGE_SIZE) + 1 - (textstart/PAGE_SIZE);
  total_frames_reqd += (rodataend/PAGE_SIZE) + 1 - (rodatastart/PAGE_SIZE);
  total_frames_reqd += (dataend/PAGE_SIZE) + 1 - (datastart/PAGE_SIZE);
  total_frames_reqd += (bssend/PAGE_SIZE) + 1 - (bssstart/PAGE_SIZE);

  // lprintf("The number of frames reqd are %u", total_frames_reqd - reduce_count + 1);
  
  // Adding one frame for the stack
  total_frames_reqd++;
  total_frames_reqd -= reduce_count;

  eff_mutex_lock(&kernel.mutex);
  if (total_frames_reqd <= kernel.free_frame_count) {
    kernel.free_frame_count -= total_frames_reqd;
  } else {
    eff_mutex_unlock(&kernel.mutex);
    return 0;
  }
  eff_mutex_unlock(&kernel.mutex);

  return total_frames_reqd;
}

/** @brief  Loads the ELF header of a given task
 *
 *  The function checks that the task exists and that the ELF header associated 
 *  to this task is valid. It returns an error if the ELF does not exists or is
 *  invalid. On success the elf argument is populated with the task sections
 *  information.
 *
 *  @param  task_name   The task's name
 *  @param  elf         A simple_elf_t data structure that will be populated by
 *                      the function
 *
 *  @return 0 on success, a negative number on error
 */
int load_elf_file(char *task_name, simple_elf_t *elf) {

  // Check validity of arguments
  if (task_name == NULL || elf == NULL) {
    lprintf("Invalid argument to function create_task_from_executable()");
    return -1;
  }

  // Check that the ELF header exists and is valid
  if (elf_check_header(task_name) == ELF_NOTELF) {
    lprintf("Could not find ELF header for task \"%s\"", task_name);
    return -1;
  }

  // Populate the simple_elf_t data structure
  if (elf_load_helper(elf, task_name) == ELF_NOTELF) {
    lprintf("ELF header is invalid for task \"%s\"", task_name);
    return -1;
  }

  return 0;
}
