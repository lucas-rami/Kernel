/** @file exception_handlers.c
 *  @brief  This file contains the definitions for all the exception handlers
 *          and the function used to register the handlers in the IDT
 *  @author akanjani, lramire1
 */

#include <idt.h>
#include <syscalls.h>
#include <seg.h>
#include <interrupts.h>
#include <page_fault_handler.h>
#include <exception_handlers.h>
#include <simics.h>
#include <kernel_state.h>
#include "exception_handlers_asm.h"
#include <cr.h>

/** @brief  Registers all the exception handlers in the IDT
 *
 *  In case of error, the number of handlers registered when the function 
 *  returns is undefined.
 *
 *  @return 0 on success, a negative number on error
 */
int exception_handlers_init() {

  // Register the page fault handler
  if (page_fault_init() < 0) {
    lprintf("Failed to register page fault handler");
    return -1;
  }

  // Array of wrappers for exception handlers
  uintptr_t exception_handlers[] = {(uintptr_t)divide_handler, 
                                    (uintptr_t)debug_handler, 
                                    (uintptr_t)breakpoint_handler,
                                    (uintptr_t)overflow_handler,
                                    (uintptr_t)boundcheck_handler,
                                    (uintptr_t)opcode_handler, 
                                    (uintptr_t)nofpu_handler,
                                    (uintptr_t)segfault_handler,
                                    (uintptr_t)stackfault_handler,
                                    (uintptr_t)protfault_handler,
                                    (uintptr_t)fpufault_handler,
                                    (uintptr_t)alignfault_handler,
                                    (uintptr_t)simdfault_handler};

  // Array of indices in the IDT
  uint32_t exception_idt_indices[] = {IDT_DE, IDT_DB, IDT_BP, IDT_OF, IDT_BR,
                                      IDT_UD, IDT_NM, IDT_NP, IDT_SS, IDT_GP,
                                      IDT_MF, IDT_AC, IDT_XF};

  int nb_exceptions = sizeof(exception_handlers) / sizeof(uintptr_t);
  int i;

  // Register all handlers
  for (i = 0; i < nb_exceptions; i++) {
    if (register_handler((uintptr_t)exception_handlers[i], TRAP_GATE,
        exception_idt_indices[i], USER_PRIVILEGE_LEVEL, SEGSEL_KERNEL_CS) <0) {
      return -1;
    }
  }
  return 0;

}

/** @brief  Generic exception handler for all non-page-fault exceptions
 *
 *  If the thread has a software handler registered, then the user program has
 *  a chance to resolve the situation. If no exception handler is registered
 *  or the registered handler does not resolve the issue, then the kernel sets
 *  the current task's exit status to -2 and kills the faulting thread.
 *
 *  @param  cause       A number uniquely representing the type of exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *
 *  @return Does not return
 */
void generic_exception_handler(int cause, char *stack_ptr) {
  lprintf("Exception other than Page fault. Cause %d at address %p", 
          cause, (char*)get_cr2());
  
  // Execute the user-registered handler if one exists
  create_stack_sw_exception(cause, stack_ptr);
  
  // Set the task's exist status and kill the thread
  kern_set_status(EXCEPTION_EXIT_STATUS);
  kern_vanish();
}

/** @brief  Exception handler for divide exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *  @return Does not return
 */
void divide_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_DIVIDE, stack_ptr);
}

/** @brief  Exception handler for debug exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *  @return Does not return
 */
void debug_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_DEBUG, stack_ptr);
}

/** @brief  Exception handler for breakpoint exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *  @return Does not return
 */
void breakpoint_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_BREAKPOINT, stack_ptr);
}

/** @brief  Exception handler for overflow exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *  @return Does not return
 */
void overflow_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_OVERFLOW, stack_ptr);
}

/** @brief  Exception handler for boundcheck exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *  @return Does not return
 */
void boundcheck_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_BOUNDCHECK, stack_ptr);
}

/** @brief  Exception handler for opcode exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *  @return Does not return
 */
void opcode_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_OPCODE, stack_ptr);
}

/** @brief  Exception handler for nofpu exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *  @return Does not return
 */
void nofpu_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_NOFPU, stack_ptr);
}

/** @brief  Exception handler for segfault exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *  @return Does not return
 */
void segfault_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_SEGFAULT, stack_ptr);
}

/** @brief  Exception handler for stackfault exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *  @return Does not return
 */
void stackfault_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_STACKFAULT, stack_ptr);
}

/** @brief  Exception handler for protfault exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *  @return Does not return
 */
void protfault_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_PROTFAULT, stack_ptr);
}

/** @brief  Exception handler for fpufault exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *  @return Does not return
 */
void fpufault_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_FPUFAULT, stack_ptr);
}

/** @brief  Exception handler for alignfault exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *  @return Does not return
 */
void alignfault_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_ALIGNFAULT, stack_ptr);
}

/** @brief  Exception handler for simdfault exception
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *  @return Does not return
 */
void simdfault_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_SIMDFAULT, stack_ptr);
}
