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

#define ERR_INVALID_ARGS -1
// TODO: What should this value be? How about max no of args
#define ARGS_MAX_SIZE 128
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

  // Validate all arguments
  if (is_valid_string(execname) == FALSE) {
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
    if (is_valid_string(argvec[i]) == FALSE || strlen(argvec[i]) > 
         ARGS_MAX_SIZE) {
      lprintf("Invalid args");
      return ERR_INVALID_ARGS;
    }
    i++;
  }

  unsigned int *old_cr3 = (unsigned int*)get_cr3();

  if (create_task_from_executable(execname, TRUE, argvec, i) == 0) {
    // Error creating the new task
    return -1;
  }

  // Overwrite the cr3 value with the new one
  kernel.current_thread->cr3 = (uint32_t)get_cr3();

  // TODO: Why is this needed?
  disable_interrupts();
  free_address_space(old_cr3, KERNEL_AND_USER_SPACE);

  // The setup of the second program is complete. Time to switch to it and 
  // start execution.
  switch_esp(kernel.current_thread->esp);

  // SHOULD NEVER RETURN HERE
  // assert(0);
  lprintf("EXEC RETURNED TO THE CALLING PROGRAM. FATAL ERROR");
  return 0;
  // TODO: Think of a reasonable limit on the number of args in argvec
  // TODO: On failure, clean up whatever space we have allocated for this 
  // new task
  // and return a negative value. Should be handled in task_create
  // TODO: Garbage collect the kernel stack
  // TODO: No software exception handler should be registered. Do something to 
  // make
  // that happen
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
  char **args_addr = malloc(sizeof(char*) * (count));
  int i = 0, len;
  
  set_cr3((uint32_t)old_ptd);
  while (argvec[i] != NULL) {
    // TODO: Is this a TOCTOU problem?
    len = strlen(argvec[i]);
    memcpy(buf, argvec[i], len + 1);
    stack_addr -= (len + 1);
    args_addr[i] = stack_addr;
    set_cr3((uint32_t)new_ptd);
    memcpy(stack_addr, buf, len + 1);
    set_cr3((uint32_t)old_ptd);
    i++;
  }

  stack_addr -= (sizeof(char*) * (count));
  char *start_of_argv = stack_addr;
  set_cr3((uint32_t)new_ptd);
  memcpy(stack_addr, args_addr, (sizeof(char*) * (count)));
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
  
