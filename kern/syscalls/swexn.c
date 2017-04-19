/** @file swexn.c
 *  @brief This file contains the definition for the kern_swexn() system call.
 *  @author akanjani, lramire1
 */

#include <stddef.h>
#include <virtual_memory.h>
#include <simics.h>
#include <string.h>
#include <kernel_state.h>
#include <common_kern.h>
#include <seg.h>
#include <eflags.h>

#define NUM_ARGS 4

#include <syscalls.h>

int kern_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg) {

  // lprintf("swexn esp %p, eip %p, arg %p, ureg %p", esp3, eip, arg, newureg);
  // lprintf("cause %p, cr2 %p, ds %p, es %p, fs %p, gs %p, esp %p", (uint32_t*)newureg->cause, (uint32_t*)newureg->cr2, (uint32_t*)newureg->ds, (uint32_t*)newureg->es, (uint32_t*)newureg->fs, (uint32_t*)newureg->gs, (uint32_t*)newureg->esp);
  char *stack_pointer = (char*)&esp3;
  int ret = 0;
  // lprintf("sp %p", stack_pointer);
  // Move the stack pointer to the 
  stack_pointer += ((NUM_ARGS) * sizeof(void*));
  // lprintf("New sp %p", stack_pointer);
  if ((eip && (unsigned)eip < USER_MEM_START) || (esp3 && (unsigned)esp3 < USER_MEM_START)) {
    // Invalid args
    // TODO: More validation. Check if eip is in text section
    // Check if esp3 looks like the correct value
    return -1;
  }

  // Check validity of ureg data structure
  if (newureg != NULL) {

    // Chech that the ureg_t structure is in user address space
    // TODO: check ureg not in kernel space
    if (is_buffer_valid((unsigned int)newureg, sizeof(ureg_t)) < 0) {
      lprintf("\tkern_swexn(): Invalid newureg buffer");
      return -1;
    }

    // Check segment selectors
    if (newureg->ds != SEGSEL_USER_DS || newureg->es != SEGSEL_USER_DS || 
        newureg->fs != SEGSEL_USER_DS || newureg->gs != SEGSEL_USER_DS ||
        newureg->ss != SEGSEL_USER_DS || newureg->cs != SEGSEL_USER_CS) {
      lprintf("\tkern_swexn(): Invalid segments");          
      return -1;
    }

    // Check EFLAGS 
    // TODO: IOPL ?
    uint32_t eflags = newureg->eflags;
    if ( !(eflags & EFL_RESV1) || (eflags & EFL_AC) ||
        (eflags & EFL_IOPL_RING3) != EFL_IOPL_RING3 || !(eflags & EFL_IF) ) {
      lprintf("\tkern_swexn(): Invalid EFLAGS ");          
      return -1;
    }

  }

  if (newureg != NULL) {

    ret = (int)newureg->eax;

    // lprintf("Memcpy");
    // lprintf("swexn esp %p, eip %p, arg %p, ureg %p", esp3, eip, arg, newureg);
    // lprintf("Newureg ds address %p", &newureg->ds);
    // lprintf("Memcpy donw");
    // lprintf("swexn esp %p, eip %p, arg %p, ureg %p", esp3, eip, arg, newureg);

    // memcpy(stack_pointer, &newureg->ds, 12 * sizeof(unsigned int));
    // stack_pointer += 12;
    // memcpy(stack_pointer, &newureg->eip, 5 * sizeof(unsigned int));
    
    memcpy(stack_pointer, &newureg->ds, (sizeof(ureg_t) - (2 * sizeof(unsigned int))));

  }

  // Store the current esp of the exception stack and the eip to restore in
  // case something goes bad
  eff_mutex_lock(&kernel.current_thread->mutex);
  if (esp3 == NULL || eip == NULL) {
    // Deregister the current handler
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

  lprintf("\tkern_swexn(): Returning");
  
  return ret;
} 
