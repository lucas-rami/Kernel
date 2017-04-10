#include <syscalls.h>

int kern_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg) {

  if ((unsigned)eip < USER_MEM_START || (unsigned)esp3 < USER_MEM_START) {
    // Invalid args
    return -1;
  }

  // PRECHECKS
  if (newureg != NULL) {
    // Check if memory address is valid for the whole structure. If yes,
    // copy it to our stack
    // Otherwise, throw an error
  }

  // Store the current esp of the exception stack and the eip to restore in
  // case something goes bad
  mutex_lock(&kernel.current_thread->mutex);
  if (esp3 == NULL || eip == NULL) {
    // Deregister the current handlewexn_handler_t eip
    kernel.current_thread->swexn_values.esp3 = NULL;
    kernel.current_thread->swexn_values.eip = NULL;
    kernel.current_thread->swexn_values.arg = NULL;
  } else {
    // Register this handler
    kernel.current_thread->swexn_values.esp3 = esp3;
    kernel.current_thread->swexn_values.eip = eip;
    kernel.current_thread->swexn_values.arg = arg;
  }
  mutex_unlock(&kernel.current_thread->mutex);

  if (newureg != NULL) {
    // The values on return should be that in the newureg
  }
  return 0;
} 
