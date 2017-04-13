#include <stddef.h>
#include <virtual_memory.h>
#include <simics.h>
#include <string.h>
#include <kernel_state.h>
#include <common_kern.h>

#define NUM_ARGS 4

#include <syscalls.h>

int kern_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg) {

  lprintf("swexn esp %p, eip %p, arg %p, ureg %p", esp3, eip, arg, newureg);
  lprintf("cause %p, cr2 %p, ds %p, es %p, fs %p, gs %p, esp %p", (uint32_t*)newureg->cause, (uint32_t*)newureg->cr2, (uint32_t*)newureg->ds, (uint32_t*)newureg->es, (uint32_t*)newureg->fs, (uint32_t*)newureg->gs, (uint32_t*)newureg->esp);
  char *stack_pointer = (char*)&esp3;
  lprintf("sp %p", stack_pointer);
  int ret = 0;
  // Move the stack pointer to the 
  stack_pointer += ((NUM_ARGS) * sizeof(void*));
  lprintf("New sp %p", stack_pointer);
  if ((eip && (unsigned)eip < USER_MEM_START) || (esp3 && (unsigned)esp3 < USER_MEM_START)) {
    // Invalid args
    // TODO: More validation. Check if eip is in text section
    // Check if esp3 looks like the correct value
    return -1;
  }

  // PRECHECKS
  if (newureg != NULL) {
    /*if (is_buffer_valid((unsigned int)newureg, sizeof(ureg_t)) < 0) {
      lprintf("Buffer is not valid");
    } else {
    */  ret = (int)newureg->eax;
      lprintf("Memcpy");
      lprintf("swexn esp %p, eip %p, arg %p, ureg %p", esp3, eip, arg, newureg);
      lprintf("Newureg ds address %p", &newureg->ds);
      memcpy(stack_pointer, &newureg->ds, (sizeof(ureg_t) - (2 * sizeof(unsigned int))));
      // memcpy(stack_pointer + (13 * sizeof(unsigned int)), &newureg->eip, 5 * sizeof(unsigned int));
      lprintf("Memcpy donw");
      lprintf("swexn esp %p, eip %p, arg %p, ureg %p", esp3, eip, arg, newureg);
    // }
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
  lprintf("Returning from kern_swexn");
  // MAGIC_BREAK;
  return ret;
} 
