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

// Debugging
#include <simics.h>

// NOTE: Temporary
#define ESP 0xFFFFFFFF
#define TRUE 1
#define FALSE 0

// Number of registers poped during a popa instruction
#define NB_REGISTERS_POPA 8


/** @brief Creates a task from an executable. Loads all the different
 *   sections required by the program from the ELF file. Allocated a new
 *   pcb/tcb if needed. This only happens when the first task for the system
 *   is being created. After that, all the calls are from exec and as they
 *   already have a tcb/pcb, we do not allocate new ones.
 *
 *  @param task_name     A string specifying the name of the program to be 
 *                       loaded
 *  @param is_exec       An int value specifying whether this function is 
 *                       being called from exec or not. 
 *  @param argvec        A char** to the argument vector passed to exec.
 *                       It is NULL in the case of the first task
 *  @param count         The count of arguments in argvec
 *
 *  @return unsigned int The starting instruction pointer of the program 
 *                       that is being loaded on success, 0 otherwise
 */
unsigned int create_task_from_executable(const char *task_name, int is_exec, char **argvec, int count) {

  if (task_name == NULL) {
    lprintf("Invalid argument to function create_task_from_executable()ç");
    return 0;
  }

  // Check that the ELF header exists and is valid
  if (elf_check_header(task_name) == ELF_NOTELF) {
    lprintf("Could not find ELF header for task \"%s\"", task_name);
    return 0;
  }

  // Hold information about the ELF header
  simple_elf_t elf;

  // Populate the simple_elf_t data structure
  if (elf_load_helper(&elf, task_name) == ELF_NOTELF) {
    lprintf("ELF header is invalid for task \"%s\"", task_name);
    return 0;
  }

  // Allocate a kernel stack for the root thread
  void *stack_kernel = NULL;
  stack_kernel = malloc(PAGE_SIZE);
  if (stack_kernel == NULL) {
    lprintf("Could not allocate kernel stack for task's root thread");
    return 0;
  }

  // Setup virtual memory for this task
  // TODO: Free all this virtual space if anything fails
  unsigned int *cr3, *old_cr3;
  old_cr3 = (unsigned int *)get_cr3();
  if ((cr3 = setup_vm(&elf)) == NULL) {
    lprintf("Task creation failed for task \"%s\"", task_name);
    free(stack_kernel);
    return 0;
  }

  uint32_t esp0, stack_top;
  pcb_t *new_pcb = NULL;
  tcb_t *new_tcb = NULL;
  if (is_exec != TRUE) {
    // Doing this for the first user task of the system
    esp0 = (uint32_t)(stack_kernel) + PAGE_SIZE;

   // Create new PCB/TCB for the task and its root thread
    new_pcb = create_new_pcb();
    if (new_pcb == NULL) {
      lprintf("create_task_from_executable(): PCB initialization failed");
      free(stack_kernel);
      return 0;
    }

    new_tcb = create_new_tcb(new_pcb, esp0, (uint32_t)cr3);
    if (new_tcb == NULL) {
      lprintf("create_task_from_executable(): TCB initialization failed");
      free(stack_kernel);
      hash_table_remove_element(&kernel.pcbs, new_pcb);
      return 0;
    }
    stack_top = ESP;
  } else {
    esp0 = (uint32_t)(stack_kernel) + PAGE_SIZE;
    new_tcb = kernel.current_thread;
    new_tcb->esp0 = esp0;
    char *new_stack_addr = load_args_for_new_program(argvec, old_cr3, count);
    stack_top = (uint32_t)new_stack_addr;
  }

  // Create EFLAGS for the user task
  uint32_t eflags = get_eflags();
  eflags |= EFL_RESV1;       // Set bit 1
  eflags &= !EFL_AC;         // Alignment checking off
  eflags &= !EFL_IOPL_RING3; // Clear current privilege level
  eflags |= EFL_IOPL_RING3;  // Set privilege level to 3
  eflags |= EFL_IF;          // Enable interrupts

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

  if (is_exec != TRUE) {
    // It isn't called from exec. The gerneral purpose regs won't be popped
    stack_addr -= NB_REGISTERS_POPA;
  }

  // Save stack pointer value in TCB
  new_tcb->esp = (uint32_t) stack_addr;

  // Make the thread runnable
  mutex_lock(&kernel.mutex);
  add_runnable_thread(new_tcb);
  mutex_unlock(&kernel.mutex);

  return elf.e_entry;
}
