/** @file cond_var.c
 *
 *  @brief This file contains the definitions for condition variable functions
 *   It implements cvar_init, cvar_wait, cvar_signal and cvar_broadcast which
 *   can be used by applications for synchronization
 *
 *  @author akanjani, lramire1
 */

#include <assert.h>
#include <mutex.h>
#include <stdio.h>
#include <syscalls.h>
#include <cond_var.h>
#include <simics.h>

/** @brief The state of a condition variable which means that a cond_init has
 *   been called but cond_destroy hasn't been called after that
 */
#define CVAR_INITIALIZED 1

/** @brief The state of a condition variable which means that a cond_destroy
 *   has been called but cond_init hasn't been called after that
 */
#define CVAR_UNINITIALIZED 0

/** @brief The parameter to deschedule to put a thread from the running queu to
 *   the waiting queue of the scheduler.
 */
#define DONT_RUN 0

/** @brief Initializes a condition variable
 *
 *  This function initializes the condition variable pointed to by cv.
 *  The effects of using a condition variable before it has been initialized,
 *  or of initializing it when it is already initialized and in use
 *  are undefined.
 *
 *  @param cv The condition variable to initialize
 *
 *  @return Zero on success, a negative number on error
 */
int cond_init(cond_t *cv) {

  if (!cv) {
    // Invalid parameter
    return -1;
  }

  // Initialize the cvar state
  cv->init = CVAR_INITIALIZED;
  cv->waiting_head = NULL;
  cv->waiting_tail = NULL;

  // Initialize the cvar mutex
  if (mutex_init(&cv->mp) < 0) {
    return -1;
  } 

  return 0;
}

/** @brief Destroys a condition variable
 *
 *  This function deactivates the condition variable pointed to by cv.
 *  It is illegal for an application to use a condition variable after it has
 *  been destroyed (unless and until it is later re-initialized). It is
 *  illegal for an application to invoke cond destroy() on a condition
 *  variable while threads are blocked waiting on it.
 *
 *  @param cv A pointer to the condition variable to deactivate
 *
 *  @return void
 */
void cond_destroy(cond_t *cv) {

  // Invalid parameter
  assert(cv);

  if (cv->init != CVAR_INITIALIZED) {
    lprintf("Not init %p", cv);
  }
  // Illegal Operation. Destroy on an uninitalized cvar
  assert(cv->init == CVAR_INITIALIZED);

  // Illegal Operation. Destroy on a cvar for which thread(s) are waiting for
  assert(cv->waiting_head == NULL);

  // Destroy the cvar mutex
  mutex_destroy(&cv->mp);

  // Reset the state
  cv->init = CVAR_UNINITIALIZED;
  
}

/** @brief Waits for a condition to be true associated with cv
 *
 *  It allows a thread to wait for a condition and release the associated
 *  mutex that it needs to hold to check that condition. The calling thread
 *  blocks, waiting to be signaled. The blocked thread may be awakened by a
 *  cond signal() or a cond broadcast(). Upon return from cond wait(), *mp
 *  has been re-acquired on behalf of the calling thread.
 *
 *  @param cv A pointer to the condition variable
 *  @param mp A pointer to the mutex held by the thread
 *
 *  @return void
 */
void cond_wait(cond_t *cv, mutex_t *mp) {

  // Invalid parameter
  assert(cv && mp);

  if (cv->init != CVAR_INITIALIZED) {
    lprintf("Not init %p", cv);
  }
  // Illegal Operation. cond_wait on an uninitialized cvar
  assert(cv->init == CVAR_INITIALIZED);

  // Add this thread to the waiting queue of this condition variable
  generic_node_t new_tail = {(void*) kern_gettid(), NULL};

  // Lock the cvar mutex
  mutex_lock(&cv->mp);

  // Enqueue the invoking thread
  if (cv->waiting_head != NULL) {
    cv->waiting_tail->next = &new_tail;
    cv->waiting_tail = &new_tail;
  } else {
    cv->waiting_head = (cv->waiting_tail = &new_tail);
  }

  // Unlock the cvar mutex
  mutex_unlock(&cv->mp);  

  // Release the mutex so that other threads can run now
  mutex_unlock(mp);

  // Tell the scheduler to not run this thread
  int dont_run = DONT_RUN;
  kern_deschedule(&dont_run);

  // Take the mutex before leaving cvar_wait
  mutex_lock(mp);
}

/** @brief Wakes up a thread waiting on the condition variable pointed to
 *   by cv, if one exists
 *
 *  @param cv A pointer to the condition variable
 *
 *  @return void
 */
void cond_signal(cond_t *cv) {

  // Invalid parameter
  assert(cv);

  if (cv->init != CVAR_INITIALIZED) {
    lprintf("Not init %p", cv);
  }
  // Illegal operation. cond_signal on an uninitialized cvar
  assert(cv->init == CVAR_INITIALIZED);

  // Lock the cvar mutex
  mutex_lock(&cv->mp);

  if (cv->waiting_head != NULL) {

    // Pop the first thread from the queue
    int tid = (int) cv->waiting_head->value;
    cv->waiting_head = cv->waiting_head->next;

    // Unlock the cvar mutex
    mutex_unlock(&cv->mp);  

    // Wake up the descheduled thread 
    while (kern_make_runnable(tid) < 0) {
      kern_yield(tid);
    }

  } else {
    // Unlock the cvar mutex
    mutex_unlock(&cv->mp);  
  }

}

/** @brief Wakes up all threads waiting on the condition variable pointed to
 *   by cv
 *
 *  @param cv A pointer to the condition variable
 *
 *  @return void
 */
void cond_broadcast(cond_t *cv) {

  // Invalid parameter
  assert(cv);

  // Illegal operation. cond_broadcast on an uninitialized cvar
  assert(cv->init == CVAR_INITIALIZED);

  int tid = -1;

  // Loop through the whole waiting queue
  do {  
    // Lock the cvar mutex
    mutex_lock(&cv->mp);

    if (cv->waiting_head != NULL) {

      // Pop the first thread from the queue
      int tid = (int) cv->waiting_head->value;
      cv->waiting_head = cv->waiting_head->next;

      // Unlock the cvar mutex
      mutex_unlock(&cv->mp);  

      // Wake up the descheduled thread 
      while (kern_make_runnable(tid) < 0) {
        kern_yield(tid);
      }

    } else {
      // Unlock the cvar mutex and set tid to -1
      mutex_unlock(&cv->mp);
      tid = -1;
    }
  } while (tid != -1);


}
