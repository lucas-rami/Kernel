#include <stddef.h>
#include <virtual_memory.h>
#include <virtual_memory_defines.h>
#include <simics.h>
#include <string.h>
#include <kernel_state.h>
#include <common_kern.h>
#include <seg.h>
#include <eflags.h>

#define NUM_ARGS 4

#include <syscalls.h>

/** @brief  (De)Registers an exception handler
 *
 *  If esp3 and/or eip are zero, de-register an exception handler if one is 
 *  currently registered.
 *  If a single invocation of the swexn() system call attempts to register or 
 *  de-register a handler, and also attempts to specify new register values, 
 *  and either request cannot be carried out, neither request will be.
 *  It is not an error to register a new handler if one was previously 
 *  registered or to de-register a handler when one was not registered.
 *
 *  esp3 should lie in writable user-space memory to be considered valid.
 *  eip should lie in read_only user-space memory to be considered valid.
 *  See code for newureg validity specifications
 *
 *  @param  esp3    Specifies an exception stack; it points to an address one
 *                  word higher than the first address that the kernel should
 *                  use to push values onto the exception stack.
 *  @param  eip     Points to the first instruction of the handler function
 *  @param  arg     Opaque void* argument passed to exception handler
 *  @param  newureg Data structure holding a snapshot of general purpose
 *                  registers, data segment selectors, IRET exception stack with
 *                  error code, cr2 register, and exception cause 
 *
 *  @return A negative number if either one of the argument is invalid (in that
 *          case no exception handler has been registered/unregistered).
 *          If newureg is non-zero, the kernel adopts the specified register
 *          values before returning.
 *          0 if the invocation is valid and newureg is zero
 */
int kern_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg) {

  char *stack_pointer = (char*)&esp3;
  stack_pointer += ((NUM_ARGS) * sizeof(void*));

  int ret = 0;

  // Validation for esp3
  if (esp3 != NULL) {

    unsigned int exception_stack = (unsigned int)esp3;
    int len = 8 + sizeof(ureg_t);

    if (exception_stack < USER_MEM_START) {
      return -1;
    }

       /* When creating the exception stack in create_stack_sw_exception(), we
  //    * write 8 + sizeof(ureg_t) bytes on the exception stack.
  //    * We have to check that this space is writable */
    exception_stack -= len;

    if (is_buffer_valid(exception_stack, len, READ_WRITE) < 0) {
  //     // Reject call if esp3 is invalid
      return -1;
    }
  }

  // // Validation for eip
  if (eip != NULL) {

    if (is_buffer_valid((unsigned int)eip, sizeof(uintptr_t), READ_ONLY) < 0) {
  //     // Reject call if eip is invalid
      return -1;
    }

  }

  // Validation for newureg
  if (newureg != NULL) {

    // Check that the ureg_t structure is in user address space
    if (is_buffer_valid((unsigned int)newureg, sizeof(ureg_t), READ_WRITE) < 0){
      return -1;
    }

    // Check segment selectors
    if (newureg->ds != SEGSEL_USER_DS || newureg->es != SEGSEL_USER_DS || 
        newureg->fs != SEGSEL_USER_DS || newureg->gs != SEGSEL_USER_DS ||
        newureg->ss != SEGSEL_USER_DS || newureg->cs != SEGSEL_USER_CS) {
      return -1;
    }

    // Check EFLAGS 
    uint32_t eflags = newureg->eflags;
    if ( !(eflags & EFL_RESV1) || (eflags & EFL_AC) ||
        eflags & EFL_IOPL_RING3 || !(eflags & EFL_IF) ) {
      return -1;
    }

  }

  if (newureg != NULL) {
    // If the user specified new registers, copy them on the kernel stack 
    // before returning to user space
    ret = (int)newureg->eax;
    memcpy(stack_pointer, &newureg->ds, 
            (sizeof(ureg_t) - (2 * sizeof(unsigned int))));
  }

  // Store the current esp of the exception stack and the eip to restore in
  // case something goes bad
  eff_mutex_lock(&kernel.current_thread->mutex);
  if (esp3 == NULL || eip == NULL) {
    // Deregister the current exception handler
    kernel.current_thread->swexn_values.esp3 = NULL;
    kernel.current_thread->swexn_values.eip = NULL;
    kernel.current_thread->swexn_values.arg = NULL;
  } else {
    // Register this handler
    kernel.current_thread->swexn_values.esp3 = esp3;
    kernel.current_thread->swexn_values.eip = eip;
    kernel.current_thread->swexn_values.arg = arg;
  }
  eff_mutex_unlock(&kernel.current_thread->mutex);
  
  return ret;
} 
