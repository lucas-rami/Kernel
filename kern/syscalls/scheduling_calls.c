/** @file scheduling_calls.c
 *  @brief This file contains the declaration for the system calls that directly
 *  affect thread scheduling.
 *  @author akanjani, lramire1
 */

#include <atomic_ops.h>
#include <kernel_state.h>
#include <scheduler.h>
#include <stdlib.h>

int sys_yield(int tid) {

  tcb_t *current_thread = kernel.current_thread;

  if (tid == -1) {
    add_runnable_thread(current_thread);
    run_next_thread(current_thread);
  } else if (tid >= 0) {
    tcb_t *next_thread = NULL; // TODO: should get the TCB
    if (next_thread != NULL && next_thread->thread_state == THR_RUNNABLE) {
      add_runnable_thread(current_thread);
      force_next_thread(current_thread, next_thread);
    } else {
      return -1;
    }
  }

  return 0;
}

int sys_deschedule(int *reject) {

  // TODO: check that reject is a valid pointer

  // Atomically checks the integer pointed to by reject
  int r = atomic_exchange(reject, *reject);

  if (r == 0) {
    tcb_t *current_thread = kernel.current_thread;

    current_thread->descheduled = THR_DESCHEDULED_TRUE;
    add_blocked_thread(current_thread);
    run_next_thread(current_thread);
  }

  return 0;
}

int sys_make_runnable(int tid) {

  // If the tid is less than 0, we return immediately
  if (tid < 0) {
    return -1;
  }

  // Try to get the TCB with the given tid
  tcb_t *tcb = NULL; // TODO; should get the TCB

  // If the thread exists and has been descheduled make it runnable again
  if (tcb != NULL && tcb->thread_state == THR_BLOCKED &&
      tcb->descheduled == THR_DESCHEDULED_TRUE) {
    add_runnable_thread(tcb);
    return 0;
  }

  return -1;
}
