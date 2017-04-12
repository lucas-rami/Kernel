#include <stddef.h>
#include <virtual_memory.h>
#include <simics.h>
#include <string.h>
#include <kernel_state.h>
#include <common_kern.h>

#define NUM_ARGS 4

#include <syscalls.h>

int kern_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg) {

  char *stack_pointer = get_esp();
  int ret = 0;
  // Move the stack pointer to the 
  stack_pointer += (NUM_ARGS + 1) * sizeof(void*);
  if ((unsigned)eip < USER_MEM_START || (unsigned)esp3 < USER_MEM_START) {
    // Invalid args
    // TODO: More validation. Check if eip is in text section
    // Check if esp3 looks like the correct value
    return -1;
  }

  // PRECHECKS
  if (newureg != NULL) {
    if (is_buffer_valid((unsigned int)newureg, sizeof(ureg_t)) < 0) {
      lprintf("Buffer is not valid");
    } else {
      ret = (int)newureg->eax;
      memcpy(stack_pointer, newureg, (sizeof(ureg_t) - (2 * sizeof(unsigned int))));
    }
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

  /*
  if (newureg != NULL) {
    // The values on return should be that in the newureg
  }
  */
  return ret;
} 
