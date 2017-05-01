/** @file scheduling_calls.c
 *  @brief  This file contains the definitions for the yield(), make_runnable()
 *          and deschedule() system calls.
 *  @author akanjani, lramire1
 */

#include <kernel_state.h>
#include <scheduler.h>
#include <stdlib.h>
#include <asm.h>
#include <virtual_memory.h>
#include <virtual_memory_defines.h>

/* For debugging */
#include <simics.h>

/** @brief  Defers execution of the invoking thread to a time determined by the
 *          scheduler, in favor of the thread with ID tid. If tid is -1, the 
 *          scheduler may determine which thread to run next.
 *
 *  @param  tid   A thread ID
 *
 *  @return If the thread with ID tid does not exist, is awaiting an external 
 *          event in a system call, or has been suspended via a system call, 
 *          then an integer error code less than zero is returned. Zero is 
 *          returned on success
 */
int kern_yield(int tid) {

  if (tid == -1) {
    
    make_runnable_and_switch();
  
  } else if (tid >= 0) {

    tcb_t tmp;
    tmp.tid = tid;
    tcb_t *next_thread = hash_table_get_element(&kernel.tcbs, &tmp);

    if (next_thread != NULL) {
      eff_mutex_lock(&next_thread->mutex);

      if (next_thread->thread_state == THR_RUNNABLE) {
        // If the thread exists and is in the THR_RUNNABLE state, run it
        // The mutex will be unlocked in force_next_thread()       
        force_next_thread(next_thread);
      } else {
        eff_mutex_unlock(&next_thread->mutex);  
        return -1;    
      }

    } else {
      return -1;
    }
  }

  return 0;

}

/** @brief  Atomically checks the integer pointed to by reject and acts on it. 
 *          If the integer is non-zero, the call returns immediately with return
 *          value zero. If the integer pointed to by reject is zero, then the 
 *          calling thread will not be run by the scheduler until a 
 *          make_runnable() call is made specifying the deschedule()’d thread, 
 *          at which point deschedule() will return zero  
 *
 *  This system call is atomic with respect to make runnable(): the process of 
 *  examining reject and suspending the thread will not be interleaved with any
 *  execution of make runnable() specifying the thread calling deschedule().
 *
 *  @param  reject  Pointer to an integer whose value determine whether the
 *                  thread will be descheduled by the function
 *
 *  @return An integer error code less than zero if reject is not a valid 
 *          pointer, 0 otherwise
 */
int kern_deschedule(int *reject) {

  // TODO: Check that reject is a valid pointer
  // if (is_buffer_valid((unsigned int)reject, sizeof(int), AT_LEAST_READ) < 0) {
  //   return -1;
  // }

  // Lock the mutex on the thread
  eff_mutex_lock(&kernel.current_thread->mutex);

  // Atomically checks the integer pointed to by reject
  int r = *reject;

  if (r == 0) {    
    // The mutex will be unlocked in block_and_switch
    block_and_switch(HOLDING_MUTEX_TRUE, &kernel.current_thread->mutex);
  } else {
    eff_mutex_unlock(&kernel.current_thread->mutex);
  }

  return 0;
}

/** @brief  Makes the deschedule()d thread with ID tid runnable by the scheduler
 *
 *  @param  tid   A thread ID
 *
 *  @return An integer error code less than zero unless tid is the ID of a 
 *          thread which exists but is currently non-runnable due to a call to 
 *          deschedule(). Zero on success
 */
int kern_make_runnable(int tid) {

  // If the tid is less or equal to 0, we return immediately
  if (tid <= 0) {
    return -1;
  }

  tcb_t tmp;
  tmp.tid = tid;

  // Try to get the TCB with the given tid
  tcb_t *tcb = hash_table_get_element(&kernel.tcbs, &tmp);
  if (tcb == NULL) {
    return -1;
  }

  eff_mutex_lock(&tcb->mutex);

  // If the thread exists and has been descheduled make it runnable again
  if (tcb->thread_state == THR_BLOCKED) {
    add_runnable_thread(tcb);
    eff_mutex_unlock(&tcb->mutex);
    return 0;
  }

  eff_mutex_unlock(&tcb->mutex);
  return -1;
}
