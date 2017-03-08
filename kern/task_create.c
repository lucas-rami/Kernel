/** @file task_create.c
 *  @brief This file contains the definitions for functions related to task
 *  creation from executable files
 *  @author akanjani, lramire1
 */

#include <task_create.h>

#include <stdlib.h>
#include <elf_410.h>
#include <cr.h>
#include <virtual_memory.h>
#include <malloc.h>
#include <syscall.h>
#include <pcb.h>
#include <tcb.h>
#include <kernel_state.h>

// Debugging
#include <simics.h>

int create_task_executable(const char *task_name) {

  if (task_name == NULL) {
    lprintf("Invalid argument to function create_task_executable()ç\n");
    return 0;
  }

  // Check that the ELF header exists and is valid
  if (elf_check_header(task_name) == ELF_NOTELF) {
    lprintf("Could not find ELF header for task \"%s\"\n", task_name);
    return 0;
  }

  // Hold information about the ELF header
  simple_elf_t elf;

  // Populate the simple_elf_t data structure
  if (elf_load_helper(&elf, task_name) == ELF_NOTELF) {
    lprintf("ELF header is invalid for task \"%s\"\n", task_name);
    return 0;
  }

  // Allocate a kernel stack for the root thread
  void* stack_kernel = malloc(PAGE_SIZE);
  if (stack_kernel == NULL) {
    lprintf("Could not allocate kernel stack for task's root thread\n");
    return 0;
  }

  // Setup virtual memory for this task
  if (setup_vm(&elf) < 0) {
    lprintf("Task creation failed for task \"%s\"\n", task_name);
    free(stack_kernel);
    return 0;
  }

  // Set the kernel stack for the root thread
  set_esp0((uint32_t) stack_kernel);

  // Initialize new tcb_t/pcb_t data structures
  create_new_pcb();
  create_new_root_tcb();

  return elf.e_entry;
}

/* Operated on statically allocated PCB */
void create_new_pcb() {

  pcb.return_status = 0;
  pcb.task_state = TASK_RUNNING;
  pcb.tid = kernel.task_id;
  ++kernel.task_id;

}

/* Operated on statically allocated TCB */
void create_new_root_tcb() {

  tcb.task = &pcb;
  tcb.thread_state = THR_RUNNING;
  tcb.tid = kernel.thread_id;
  ++kernel.thread_id;

}
