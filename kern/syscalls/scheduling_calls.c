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

  kern_mutex_lock(&kernel.mutex);

  tcb_t *current_thread = kernel.current_thread;

  kern_mutex_lock(&current_thread->mutex);

  if (tid == -1) {
    make_runnable_and_switch(current_thread);
  } else if (tid >= 0) {
    tcb_t *next_thread = NULL; // TODO: should get the TCB
    if (next_thread != NULL && next_thread->thread_state == THR_RUNNABLE) {
      force_next_thread(current_thread, next_thread);
    } else {
      kern_mutex_unlock(&current_thread->mutex);
      kern_mutex_unlock(&kernel.mutex);
      return -1;
    }
  }

  kern_mutex_unlock(&current_thread->mutex);
  kern_mutex_unlock(&kernel.mutex);
  return 0;
}

int sys_deschedule(int *reject) {

  // TODO: check that reject is a valid pointer

  // Lock the mutex on the kernel
  kern_mutex_lock(&kernel.mutex);

  // Get the invoking thread's TCB
  tcb_t *current_thread = kernel.current_thread;

  // Lock the mutex on the thread
  kern_mutex_lock(&current_thread->mutex);

  // Atomically checks the integer pointed to by reject
  int r = atomic_exchange(reject, *reject);

  if (r == 0) {
    current_thread->descheduled = THR_DESCHEDULED_TRUE;
    block_and_switch(current_thread); // context_switch() will release the mutex
  } else {
    kern_mutex_unlock(&current_thread->mutex);
    kern_mutex_unlock(&kernel.mutex);
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
  if (tcb == NULL) {
    return -1;
  }

  kern_mutex_lock(&kernel.mutex);
  kern_mutex_lock(&tcb->mutex);

  // If the thread exists and has been descheduled make it runnable again
  if (tcb->thread_state == THR_BLOCKED &&
      tcb->descheduled == THR_DESCHEDULED_TRUE) {
    add_runnable_thread(tcb);
    kern_mutex_unlock(&tcb->mutex);
    kern_mutex_unlock(&kernel.mutex);
    return 0;
  }
  kern_mutex_unlock(&tcb->mutex);
  kern_mutex_unlock(&kernel.mutex);
  return -1;
}
