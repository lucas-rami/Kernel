/** @file cond_var.c
 *  @brief This file contains the definitions for condition variable functions
 *         It implements cvar_init, cvar_wait, cvar_signal and cvar_broadcast
 *         which can be used by applications for synchronization
 *  @author akanjani, lramire1
 */

#include <assert.h>
#include <mutex.h>
#include <stdio.h>
#include <syscalls.h>
#include <cond_var.h>
#include <stack_queue.h>
#include <simics.h>
#include <asm.h>
#include <kernel_state.h>

/** @brief The state of a condition variable which means that a cond_init has
 *         been called but cond_destroy hasn't been called after that */
#define CVAR_INITIALIZED 1

/** @brief The state of a condition variable which means that a cond_destroy
 *         has been called but cond_init hasn't been called after that */
#define CVAR_UNINITIALIZED 0

/** @brief The parameter to deschedule to put a thread from the running queu to
 *         the waiting queue of the scheduler */
#define DONT_RUN 0

/** @brief  Initializes a condition variable
 *
 *  This function initializes the condition variable pointed to by cv.
 *  The effects of using a condition variable before it has been initialized,
 *  or of initializing it when it is already initialized and in use
 *  are undefined.
 *
 *  @param  cv  The condition variable to initialize
 *
 *  @return Zero on success, a negative number on error
 */
int cond_init(cond_t *cv) {

  // Check parameter
  if (cv == NULL) {
    return -1;
  }
  
  // Initialize the cvar mutex
  if (mutex_init(&cv->mp) < 0) {
    return -1;
  }

  // Initialize the waiting queue 
  stack_queue_init(&cv->waiting_queue);

  // Initialize the cvar state
  cv->init = CVAR_INITIALIZED;

  return 0;
}

/** @brief  Destroys a condition variable
 *
 *  This function deactivates the condition variable pointed to by cv.
 *  It is illegal for an application to use a condition variable after it has
 *  been destroyed (unless and until it is later re-initialized). It is
 *  illegal for an application to invoke cond destroy() on a condition
 *  variable while threads are blocked waiting on it.
 *
 *  @param  cv A pointer to the condition variable to deactivate
 *
 *  @return void
 */
void cond_destroy(cond_t *cv) {

  // Invalid parameter
  assert(cv != NULL && cv->init == CVAR_INITIALIZED);

  // Destroy the stack queue
  stack_queue_destroy(&cv->waiting_queue);

  // Destroy the cvar mutex
  mutex_destroy(&cv->mp);

  // Reset the state
  cv->init = CVAR_UNINITIALIZED;
  
}

/** @brief  Waits for a condition to be true associated with cv
 *
 *  It allows a thread to wait for a condition and release the associated
 *  mutex that it needs to hold to check that condition. The calling thread
 *  blocks, waiting to be signaled. The blocked thread may be awakened by a
 *  cond signal() or a cond broadcast(). Upon return from cond wait(), *mp
 *  has been re-acquired on behalf of the calling thread.
 *
 *  @param  cv  A pointer to the condition variable
 *  @param  mp  A pointer to the mutex held by the thread
 *
 *  @return void
 */
void cond_wait(cond_t *cv, mutex_t *mp) {

  // Invalid parameter
  assert(cv != NULL && mp != NULL && cv->init == CVAR_INITIALIZED &&
          mp->init == MUTEX_INITIALIZED);

  // Lock the cvar mutex
  mutex_lock(&cv->mp);

  // Add this thread to the waiting queue of this condition variable
  generic_node_t new_tail = {(void*) kernel.current_thread->tid, NULL};
  stack_queue_enqueue(&cv->waiting_queue, &new_tail);

  // Release the mutex so that other threads can run now
  mutex_unlock(mp);

  disable_interrupts();
  // Unlock the cvar mutex
  mutex_unlock(&cv->mp);  

  // Tell the scheduler to not run this thread
  int dont_run = DONT_RUN;
  kern_deschedule(&dont_run);

  // Take the mutex before leaving cvar_wait
  mutex_lock(mp);
}

/** @brief  Wakes up a thread waiting on the condition variable
 *
 *  If no thread is waiting on the condition variable, then the function has no
 *  effect.
 *
 *  @param  cv  A pointer to the condition variable
 *
 *  @return void
 */
void cond_signal(cond_t *cv) {

  // Invalid parameter
  assert(cv != NULL && cv->init == CVAR_INITIALIZED);

  generic_node_t* elem;

  // Lock the cvar mutex
  mutex_lock(&cv->mp);

  if ((elem = stack_queue_dequeue(&cv->waiting_queue)) != NULL) {

    // Unlock the cvar mutex
    mutex_unlock(&cv->mp);  

    int tid = (int) elem->value;    

    // Make the dequeue thread runnable again
    while (kern_make_runnable(tid) < 0) {
      kern_yield(tid);
    }

  } else {
    // Unlock the cvar mutex
    mutex_unlock(&cv->mp);  
  }

}

/** @brief  Wakes up all threads waiting on the condition variable
 *
 *  If no thread is waiting on the condition variable, then the function has no
 *  effect.
 *
 *  @param  cv  A pointer to the condition variable
 *
 *  @return void
 */
void cond_broadcast(cond_t *cv) {

  // Invalid parameter
  assert(cv != NULL && cv->init == CVAR_INITIALIZED);

  generic_node_t* elem;  

  // Lock the cvar mutex
  mutex_lock(&cv->mp);

  // Loop through the whole waiting queue
  while ((elem = stack_queue_dequeue(&cv->waiting_queue)) != NULL){  

    // Unlock the cvar mutex
    mutex_unlock(&cv->mp);  

    int tid = (int) elem->value;    

    // Wake up the descheduled thread 
    while (kern_make_runnable(tid) < 0) {
      kern_yield(tid);
    }

    // Lock the cvar mutex
    mutex_lock(&cv->mp);

  }

  // Unlock the cvar mutex
  mutex_unlock(&cv->mp);

}
