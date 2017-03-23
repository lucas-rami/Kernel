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

  // Initialize the waiting queue
  queue_init(&cv->waiting_queue);

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

  // Illegal Operation. Destroy on an uninitalized cvar
  assert(cv->init == CVAR_INITIALIZED);

  // Illegal Operation. Destroy on a cvar for which thread(s) are waiting for
  assert(is_queue_empty(&cv->waiting_queue) == 1);

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

  // Illegal Operation. cond_wait on an uninitialized cvar
  assert(cv->init == CVAR_INITIALIZED);

  // Add this thread to the waiting queue of this condition variable
  queue_insert_node(&cv->waiting_queue, (void*) kern_gettid());

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

  // Illegal operation. cond_signal on an uninitialized cvar
  assert(cv->init == CVAR_INITIALIZED);

  // Pop the head element from the queue
  void *tid = queue_delete_node(&cv->waiting_queue);

  // Check that the queue was not empty
  if (tid != NULL) {

    // Start the thread which was just dequed from the waiting queue
    while (kern_make_runnable((int)tid) < 0) {
      // The thread hasn't descheduled yet. Yield to it so that it can
      // deschedule
      kern_yield((int)tid);
    }
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

  void* tid = NULL;

  // Loop through the whole waiting queue
  while ((tid = queue_delete_node(&cv->waiting_queue)) != NULL) {
    while (kern_make_runnable((int)tid) < 0) {
      // The thread hasn't descheduled yet. Yield to it so that it can
      // deschedule
      kern_yield((int)(tid));
    }
  }
}
