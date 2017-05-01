/** @file sw_exception.c
 *  @brief  This file contains the definition for the function used to create
 *          the stack for a user-registered exception handler 
 *  @author akanjani, lramire1
 */

#include <kernel_state.h>
#include <stddef.h>
#include <simics.h>
#include <ureg.h>
#include <cr.h>
#include <context_switch_asm.h>
#include <eflags.h>
#include <assert.h>
#include <string.h>

/** @brief  Creates the exception stack for a user defined exception handler
 *
 *  The function first checks whether there is an exception handler registered
 *  for this thread. If there is one, then the function manually crafts an
 *  exception stack in user-space for the exception handler to run on. The
 *  function also crafts a trap frame on the kernel stack of the invoking thread
 *  so that IRET returns on the exception stack and executes the user defined
 *  handler.
 *
 *  @param  cause     The cause for the exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler
 *
 *  @return 0 if there is no used-defined exception handler registered, does
 *          not return otherwise
 */
int create_stack_sw_exception(unsigned int cause, char *stack_start) {
  
  // Check if there is an exception handler registered
  if (kernel.current_thread->swexn_values.esp3 == NULL ||
      kernel.current_thread->swexn_values.eip == NULL) {
    return 0;
  }

  const unsigned int unsigned_int_size = sizeof(unsigned int);
  const unsigned int ureg_size = sizeof(ureg_t);
  const unsigned int pointer_size = sizeof(void*);
  char *stack_ptr = kernel.current_thread->swexn_values.esp3;

  char *ureg_start = stack_ptr - ureg_size;

  /* ----- Craft the exception stack for the handler ----- */

  // Push a ureg data stucture
  *(unsigned int *)(ureg_start) = cause;
  *(unsigned int *)(ureg_start + unsigned_int_size) = (unsigned int)get_cr2();
  memcpy(ureg_start + (2 * unsigned_int_size), stack_start, ureg_size - (8 * unsigned_int_size));
  
  // Set esp in pusha to 0
  // *((unsigned int*)(ureg_start + 9 * unsigned_int_size)) = 0;  
  
  stack_start += ureg_size - (8 * unsigned_int_size);
  stack_start += pointer_size;
  memcpy(ureg_start + (14 * unsigned_int_size), stack_start, 6 * unsigned_int_size);

  // Create a stack frame
  stack_ptr = ureg_start - pointer_size;
  *(unsigned int *)stack_ptr = (unsigned int) ureg_start;
  stack_ptr -= pointer_size;
  *(unsigned int *)stack_ptr = 
                        (unsigned int) kernel.current_thread->swexn_values.arg;
  stack_ptr -= pointer_size;
  *(unsigned int *)stack_ptr = 0;

  // Entry point for exception handler
  unsigned int *sw_eip = (unsigned int *)kernel.current_thread->swexn_values.eip;

  // Deregister the handler
  kernel.current_thread->swexn_values.esp3 = NULL;
  kernel.current_thread->swexn_values.eip = NULL;
  kernel.current_thread->swexn_values.arg = NULL;

  // Make the exception handler run now by creating a trap frame
  run_first_thread((uint32_t)sw_eip, (uint32_t)stack_ptr, (uint32_t)get_eflags());

  assert(0);

  return 0;
}
