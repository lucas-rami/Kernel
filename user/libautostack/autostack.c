/** @file autostack.c
 *
 *  @brief This file contains the implementation of the install_autostack
 *   function which initializes the global state registers the software
 *   exception handler.
 *
 *  @author akanjani, lramire1
 */

#include <global_state.h>
#include <syscall.h>
#include <page_fault_handler.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

/** @brief Initializes the highest and lowest values of stack in the global
 *   state. Registers the exception handler
 *
 *  @param stack_high A void pointer to the highest address on the stack
 *  @param stack_low A void pointer to the lowest address on the stack
 *
 *  @return void
 */
void install_autostack(void *stack_high, void *stack_low) {

  // Initialize the global state
  task.stack_lowest = stack_low;
  task.stack_highest = stack_high;

  // Register the exception handler for page fault
  if (swexn(exception_handler_stack + PAGE_SIZE,
       singlethread_handler, NULL, NULL) < 0) {
    // Can't register the handler
    assert(0);
  }

}
