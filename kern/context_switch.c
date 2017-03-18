/** @file context_switch.h
 *  @brief This file contains the definitions for functions related to context
 *  switching
 *  @author akanjani, lramire1
 */

#include <cr.h>
#include <tcb.h>
#include <stdlib.h>
#include <kernel_state.h>
#include <context_switch_asm.h>
#include <assert.h>

void context_switch(tcb_t* from, tcb_t* to) {

  assert(from != NULL && to != NULL);

  // Get the value of the cr3 register (page directory address)
  uint32_t cr3 = get_cr3();

  // Context switch to the other thread
  context_switch_asm(cr3, &from->esp, to->esp);

  kernel.current_thread = to;

  // Update the thread's state
  kern_mutex_lock(&to->mutex);
  to->thread_state = THR_RUNNING;
  to->descheduled = THR_DESCHEDULED_FALSE;
  kern_mutex_unlock(&to->mutex);

  kern_mutex_unlock(&kernel.mutex);

}
