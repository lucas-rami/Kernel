/** @file scheduling_calls.c
 *  @brief This file contains the declaration for the system calls that directly
 *  affect thread scheduling.
 *  @author akanjani, lramire1
 */

#include <atomic_ops.h>
#include <kernel_state.h>
#include <scheduler.h>
#include <stdlib.h>

int kern_yield(int tid) {

  // Lock the mutex on the kernel
  mutex_lock(&kernel.mutex);
  // Lock the mutex on the thread
  mutex_lock(&kernel.current_thread->mutex);

  if (tid == -1) {
    // If the argument is -1, run the next thread in the queue
    make_runnable_and_switch();
  } else if (tid >= 0) {
    tcb_t *next_thread = hash_table_get_element(&kernel.tcbs, (void*)tid);

    if (next_thread != NULL && next_thread->thread_state == THR_RUNNABLE) {
      // If the thread exists and is in the THR_RUNNABLE state, run it
      force_next_thread(next_thread);
    } else {
      // Otherwise unlock the mutexes and return immediately with an error code
      mutex_unlock(&kernel.current_thread->mutex);
      mutex_unlock(&kernel.mutex);
      return -1;
    }
  }

  // Unlock the mutexes and return normally
  mutex_unlock(&kernel.current_thread->mutex);
  mutex_unlock(&kernel.mutex);
  return 0;
}

int kern_deschedule(int *reject) {

  // TODO: check that reject is a valid pointer

  // Lock the mutex on the kernel
  mutex_lock(&kernel.mutex);
  // Lock the mutex on the thread
  mutex_lock(&kernel.current_thread->mutex);

  // Atomically checks the integer pointed to by reject
  int r = atomic_exchange(reject, *reject);

  if (r == 0) {
    kernel.current_thread->descheduled = THR_DESCHEDULED_TRUE;
    block_and_switch();
  } else {
    mutex_unlock(&kernel.current_thread->mutex);
    mutex_unlock(&kernel.mutex);
  }

  return 0;
}

int kern_make_runnable(int tid) {

  // If the tid is less than 0, we return immediately
  if (tid < 0) {
    return -1;
  }

  // Try to get the TCB with the given tid
  tcb_t *tcb = hash_table_get_element(&kernel.tcbs, (void*)tid);
  if (tcb == NULL) {
    return -1;
  }

  mutex_lock(&kernel.mutex);
  mutex_lock(&tcb->mutex);

  // If the thread exists and has been descheduled make it runnable again
  if (tcb->thread_state == THR_BLOCKED &&
      tcb->descheduled == THR_DESCHEDULED_TRUE) {
    add_runnable_thread(tcb);
    mutex_unlock(&tcb->mutex);
    mutex_unlock(&kernel.mutex);
    return 0;
  }
  mutex_unlock(&tcb->mutex);
  mutex_unlock(&kernel.mutex);
  return -1;
}
