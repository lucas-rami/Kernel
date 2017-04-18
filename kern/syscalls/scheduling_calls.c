/** @file scheduling_calls.c
 *  @brief This file contains the declaration for the system calls that directly
 *  affect thread scheduling.
 *  @author akanjani, lramire1
 */

#include <atomic_ops.h>
#include <kernel_state.h>
#include <scheduler.h>
#include <stdlib.h>
#include <asm.h>

/* For debugging */
#include <simics.h>

int kern_yield(int tid) {

  if (tid == -1) {
    make_runnable_and_switch();
  
  } else if (tid >= 0) {

    tcb_t tmp;
    tmp.tid = tid;
    tcb_t *next_thread = hash_table_get_element(&kernel.tcbs, &tmp);

    if (next_thread != NULL) {
      mutex_lock(&next_thread->mutex);
      lprintf("\tkern_yield(tid = %d): Thread %d yielding", tid, kernel.current_thread->tid);
      if (next_thread->thread_state == THR_RUNNABLE) {
        // If the thread exists and is in the THR_RUNNABLE state, run it
        // The mutex will be unlocked in force_next_thread()       
        force_next_thread(next_thread);
      } else {
        mutex_unlock(&next_thread->mutex);  
        return -1;    
      }

    } else {
      return -1;
    }
  }

  return 0;

}

int kern_deschedule(int *reject) {

  // TODO: check that reject is a valid pointer
  // Lock the mutex on the thread
  mutex_lock(&kernel.current_thread->mutex);

  // Atomically checks the integer pointed to by reject
  // TODO: is this "atomically check" ?
  int r = *reject;

  if (r == 0) {    
    // The mutex will be unlocked in block_and_switch
    block_and_switch(HOLDING_MUTEX_TRUE);
  } else {
    mutex_unlock(&kernel.current_thread->mutex);
  }

  return 0;
}

int kern_make_runnable(int tid) {

  // If the tid is less than 0, we return immediately
  if (tid < 0) {
    return -1;
  }

  tcb_t tmp;
  tmp.tid = tid;
  // disable_interrupts();
  // Try to get the TCB with the given tid
  tcb_t *tcb = hash_table_get_element(&kernel.tcbs, &tmp);
  if (tcb == NULL) {
    lprintf("Can't find element in hash table");
    return -1;
  }

  mutex_lock(&tcb->mutex);

  // If the thread exists and has been descheduled make it runnable again
  if (tcb->thread_state == THR_BLOCKED) {
    add_runnable_thread(tcb);
    mutex_unlock(&tcb->mutex);
    // enable_interrupts();
    return 0;
  }

  mutex_unlock(&tcb->mutex);
  // enable_interrupts();
  return -1;
}
