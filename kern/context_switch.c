/** @file context_switch.h
 *  @brief This file contains the definitions for functions related to context
 *  switching
 *  @author akanjani, lramire1
 */

#include <cr.h>
#include <tcb.h>
#include <stdlib.h>
#include <kernel_state.h>
#include <context_switch.h>
#include <context_switch_asm.h>
#include <assert.h>

static void init_thread(tcb_t* to);

/** @brief Performs a context switch between two threads
 *
 *  @param to The TCB of the thread we are switching to
 *
 *  @return void
 */
void context_switch(tcb_t* to) {

  assert(kernel.current_thread != NULL && to != NULL);

  // Context switch to the other thread
  context_switch_asm(&kernel.current_thread->esp, to->esp);

  // Update the running thread state and the kernel state
  init_thread(to);

}

/** @brief Update the running thread state and the kernel state
 *
 *  The function unlocks the kernel state mutex before returning.
 *
 *  @param to The TCB of the thread we are switching to
 *
 *  @return void
 */
static void init_thread(tcb_t* to) {

  // Update the kernel state
  kernel.current_thread = to;

  // Update the thread's state
  mutex_lock(&to->mutex);
  to->thread_state = THR_RUNNING;
  to->descheduled = THR_DESCHEDULED_FALSE;
  mutex_unlock(&to->mutex);

  // Update cr3 and esp0 registers
  set_cr3(to->cr3);
  set_esp0(to->esp0);

  // Unlock the mutex on the kernel state
  mutex_unlock(&kernel.mutex);

}
