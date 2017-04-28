/** @file exec.c
 *  @brief This file contains the definition for the kern_exec() system call.
 *   It also has the definition of the load_args_for_new_program function
 *   which copies the argument vector and the strings from the address space
 *   of the first program to the program passed as a parameter to exec
 *
 *  @author akanjani, lramire1
 */

#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <syscalls.h>
#include <stddef.h>
#include <string.h>
#include <kernel_state.h>
#include <cr.h>
#include <task_create.h>
#include <malloc.h>
#include <simics.h>
#include <scheduler.h>
#include <asm.h>
#include "exec_helper.h"
#include <virtual_memory_defines.h>
#include <eflags.h>
#include <context_switch_asm.h>

#define ERR_INVALID_ARGS -1
#define ARGS_MAX_SIZE 256
#define STACK_TOP 0xFFFFFFFF
#define STACK_START_ADDR 0xfffff000
#define TRUE 1
#define FALSE 0

/** @brief The C function that handles the exec system call.
 *   Replaces the program currently running in the invoking task with the 
 *   program stored in the file named execname. The number of strings in 
 *   the vector and the vector itself will be transported into the memory
 *   of the new program where they will serve as the first and second 
 *   arguments of the the new program’s main(), respectively. Before the 
 *   new program begins, %EIP will be set to the “entry point” (the first 
 *   instruction of the main() wrapper, as advertised by the ELF linker).
 *   The kernel does as much validation as possible of the exec() request 
 *   before deallocating the old program’s resources.
 *
 *   After a successful exec() the thread that begins execution of the new 
 *   program has no software exception handler registered.
 *
 *  @param execname A string specifying the name of the program to be loaded
 *  @param argvec   An array of strings to be passed as an argument to the 
 *                  main() wrapper of the new program
 *
 *  @return int On success, this system call does not return to the invoking 
 *              program, since it is no longer running. If something goes 
 *              wrong, an integer error code less than zero will be returned.
 */
int kern_exec(char *execname, char **argvec) {
  lprintf("exec");

  int count = 0;
  // Validate all arguments
  if ((count = exec_prechecks(execname, argvec)) < 0) {
    return -1;
  }

  unsigned int *old_cr3 = (unsigned int*)get_cr3();

  simple_elf_t elf;
  if (load_elf_file(execname, &elf) <0) {
    lprintf("Error loading %p from the elf file", execname);
    return -1;
  }

  unsigned num_frames_requested;
  if ((num_frames_requested = request_frames_needed_by_program(&elf)) == 0) {
    lprintf("The program %s needs more memory than is available", execname);
    return -1;
  }
 
  lprintf("Setting up vm for tid %d", kernel.current_thread->tid);
  unsigned int *cr3 = NULL;
  if ((cr3 = setup_vm(&elf, FIRST_TASK_FALSE)) == NULL) {
    lprintf("VM setup failed for task \"%s\"", execname);
    eff_mutex_lock(&kernel.mutex);
    kernel.free_frame_count += num_frames_requested;
    eff_mutex_unlock(&kernel.mutex);
    return -1;
  }
  lprintf("Set up vm complete for tid %d", kernel.current_thread->tid);

  char *new_stack_addr = load_args_for_new_program(argvec, old_cr3, count);
  lprintf("Loaded args for new program");


  tcb_t *curr_tcb = kernel.current_thread;
  curr_tcb->num_of_frames_requested = num_frames_requested;
  curr_tcb->task->num_of_frames_requested = num_frames_requested;
  curr_tcb->swexn_values.esp3 = NULL;
  curr_tcb->swexn_values.eip = NULL;
  curr_tcb->swexn_values.arg = NULL;


  lprintf("Calling free address space");
  free_address_space(old_cr3, KERNEL_AND_USER_SPACE);
  lprintf("Returned from free address space. Should go to execed code");


  run_first_thread(elf.e_entry, (uint32_t)new_stack_addr, get_eflags());
  lprintf("SHOULDNT PRINT");


  lprintf("SHOULD NEVER RETURN HERE!!");
  return 0;
}

/** @brief Transports the number of strings in the argvec and the vector itself
 *   into the memory of the new program where they will serve as the first and 
 *   second arguments of the the new program’s main(), respectively. 
 *
 *  @param argvec  A null-terminated vector of null-terminated string arguments
 *  @param old_ptd The cr3 value of the invoking thread
 *  @param count   The number of strings in argvec
 *
 *  @return char* A pointer to the top of the stack of the new program
 */
char *load_args_for_new_program(char **argvec, unsigned int *old_ptd, 
    int count) {
  unsigned int *new_ptd = (unsigned int *)get_cr3();
  char *stack_addr = (char *)STACK_TOP;
  char *buf = malloc(sizeof(char) * (ARGS_MAX_SIZE + 1));
  char **args_addr = malloc(sizeof(char*) * (count + 1));
  int i = 0, len;
  
  kernel.current_thread->cr3 = (uint32_t)old_ptd;
  set_cr3((uint32_t)old_ptd);
  while (argvec[i] != NULL) {
    // TODO: Is this a TOCTOU problem?
    len = strlen(argvec[i]);
    memcpy(buf, argvec[i], len + 1);
    stack_addr -= (len + 1);
    args_addr[i] = stack_addr;
    kernel.current_thread->cr3 = (uint32_t)new_ptd;
    set_cr3((uint32_t)new_ptd);
    memcpy(stack_addr, buf, len + 1);
    kernel.current_thread->cr3 = (uint32_t)old_ptd;
    set_cr3((uint32_t)old_ptd);
    i++;
  }
  free(buf);

  args_addr[i] = 0;
  stack_addr -= (sizeof(char*) * (count+1));
  char *start_of_argv = stack_addr;
  kernel.current_thread->cr3 = (uint32_t)new_ptd;
  set_cr3((uint32_t)new_ptd);
  memcpy(stack_addr, args_addr, (sizeof(char*) * (count+1)));
  free(args_addr);
  unsigned int ptr_size = sizeof(void*);
  stack_addr -= ptr_size;
  *(char **)stack_addr = (char*)STACK_START_ADDR;
  stack_addr -= ptr_size;
  *(char **)stack_addr = (stack_addr - (ptr_size + sizeof(int)));
  stack_addr -= ptr_size;
  *(char **)stack_addr = start_of_argv;
  stack_addr -= sizeof(int);
  *(int *)stack_addr = (count);

  return (stack_addr - sizeof(uint32_t));
}
  
int exec_prechecks(char *execname, char **argvec) {
  
  if (kernel.current_thread->task->num_of_threads > 1) {
    lprintf("Exec Error: Multiple threads running while calling exec");
    return -1;
  }

  if (is_valid_string(execname) < 0) {
    lprintf("Execname not valid");
    return ERR_INVALID_ARGS;
  }

  if (strcmp(execname, argvec[0])) {
    // execname doesn't match the first parameter to argvec. Some things
    // might fail. Hence, returning error now
    lprintf("First argument should be the name of the program");
    return -1;
  }

  int i = 0;
  while (argvec[i] != NULL) {
    if (is_valid_string(argvec[i]) < 0 || strlen(argvec[i]) > 
         ARGS_MAX_SIZE) {
      lprintf("Invalid args");
      return ERR_INVALID_ARGS;
    }
    i++;
  }

  return i;
}
