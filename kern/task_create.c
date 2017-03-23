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

// Debugging
#include <simics.h>

// NOTE: Temporary
#define ESP 0xFFFFFFFF

// Number of registers poped during a popa instruction
#define NB_REGISTERS_POPA 8


// TODO: doc
int create_task_from_executable(const char *task_name) {

  if (task_name == NULL) {
    lprintf("Invalid argument to function create_task_from_executable()ç");
    return -1;
  }

  // Check that the ELF header exists and is valid
  if (elf_check_header(task_name) == ELF_NOTELF) {
    lprintf("Could not find ELF header for task \"%s\"", task_name);
    return -1;
  }

  // Hold information about the ELF header
  simple_elf_t elf;

  // Populate the simple_elf_t data structure
  if (elf_load_helper(&elf, task_name) == ELF_NOTELF) {
    lprintf("ELF header is invalid for task \"%s\"", task_name);
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
  if ((cr3 = setup_vm(&elf)) == NULL) {
    lprintf("Task creation failed for task \"%s\"", task_name);
    free(stack_kernel);
    return -1;
  }

  // Highest address of kernel stack
  uint32_t esp0 = (uint32_t)(stack_kernel) + PAGE_SIZE;

  // Create new PCB for the task
  pcb_t *new_pcb = create_new_pcb();
  if (new_pcb == NULL) {
    lprintf("create_task_from_executable(): PCB initialization failed");
    free(stack_kernel);
    return -1;
  }

  // Create new TCB for task's root thread
  tcb_t *new_tcb = create_new_tcb(new_pcb, esp0, (uint32_t)cr3);
  if (new_tcb == NULL) {
    lprintf("create_task_from_executable(): TCB initialization failed");
    free(stack_kernel);
    hash_table_remove_element(&kernel.pcbs, new_pcb);
    return -1;
  }

  // Create EFLAGS for the user task
  uint32_t eflags = get_eflags();
  eflags |= EFL_RESV1;       // Set bit 1
  eflags &= !EFL_AC;         // Alignment checking off
  eflags &= !EFL_IOPL_RING3; // Clear current privilege level
  eflags |= EFL_IOPL_RING3;  // Set privilege level to 3
  eflags |= EFL_IF;          // Enable interrupts

  // Craft the kernel stack for first context switch to this thread
  unsigned int * stack_addr = (unsigned int *) esp0;
  --stack_addr;
  *stack_addr = eflags;
  --stack_addr;
  *stack_addr = ESP;
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

  // Make the thread runnable
  mutex_lock(&kernel.mutex);
  add_runnable_thread(new_tcb);
  mutex_unlock(&kernel.mutex);

  return elf.e_entry;
}
