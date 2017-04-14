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
#include <asm.h>
#include <simics.h>

/** @brief Performs a context switch between two threads
 *
 *  @param to The TCB of the thread we are switching to
 *
 *  @return void
 */
void context_switch(tcb_t* to) {

  assert(kernel.current_thread != NULL && to != NULL);

  tcb_t *me = kernel.current_thread;

  // Context switch to the other thread
  context_switch_asm(&kernel.current_thread->esp, &to->esp);

  // Update the running thread state and the kernel state
  init_thread(me);

}

/** @brief Update the running thread state and the kernel state
 *
 *  The function unlocks the kernel state mutex before returning.
 *
 *  @param to The invoking thread TCB
 *
 *  @return void
 */
void init_thread(tcb_t* to) {

  // Update the kernel state
  kernel.current_thread = to;
  kernel.cpu_idle = (to == kernel.idle_thread) ? CPU_IDLE_TRUE : CPU_IDLE_FALSE;

  // Update the thread's state
  to->thread_state = THR_RUNNING;

  // Update cr3 and esp0 registers
  set_cr3(to->cr3);
  set_esp0(to->esp0);

  enable_interrupts();
}
