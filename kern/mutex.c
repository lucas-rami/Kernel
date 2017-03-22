/** @file mutex.c
 *  @brief This file contains the definitions for mutex_type.h functions
 *  @author akanjani, lramire1
 */

#include <mutex.h>
#include <simics.h>
#include <atomic_ops.h>
#include <syscall.h>
#include <assert.h>
#include <syscalls.h>

/** @brief A state of the mutex which means that mutex_init hasn't been called
 *   after a mutex_destroy
 */
#define MUTEX_UNINITIALIZED 0

/** @brief A state of the mutex which means that mutex_destroy hasn't been called
 *   after a mutex_init
 */
#define MUTEX_INITIALIZED 1

/** @brief Initialize a mutex
 *
 *  This function initializes the mutex pointed to by mp.
 *  The effects of using a mutex before it has been initialized,
 *  or of initializing it when it is already initialized and in use
 *  are undefined.
 *
 *  @param mp The mutex to initialize
 *
 *  @return Zero on success, a negative number on error
 */
int mutex_init(mutex_t *mp) {

  if (!mp) {
    // Invalid parameter
    return -1;
  }

  // Initialize the state for the mutex
  mp->prev = 0;
  mp->next_ticket = 1;
  mp->init = MUTEX_INITIALIZED;

  return 0;
}

/** @brief Deactivate a mutex
 *
 *  This function deactivates the mutex pointed to by mp.
 *  It is illegal for an application to use a mutex after it has been destroyed.
 *  It is illegal for an application to attempt to destroy a mutex while it is
 *  locked or threads are trying to acquire it.
 *
 *  @param mp The mutex to deactivate
 *
 *  @return void
 */
void mutex_destroy(mutex_t *mp) {

  // Invalid parameter
  assert(mp);

  // Illegal Operation. Destroy on an uninitalized mutex
  assert(mp->init == MUTEX_INITIALIZED);

  int j = 1;

  // Generate a new ticket for this thread
  int my_ticket = atomic_add_and_update(&mp->next_ticket, j);

  // Ensure that no other thread is locked or trying to lock this mutex
  assert((mp->prev + 1) == my_ticket);

  // Reset the mutex state
  mp->init = MUTEX_UNINITIALIZED;
}

/** @brief Acquire the lock on a mutex
 *
 *  A call to this function ensures mutual exclusion in the
 *  region between itself and a call to mutex unlock().
 *
 *  @param mp The mutex holding the lock we want to acquire
 *
 *  @return void
 */
void mutex_lock(mutex_t *mp) {

  // Validate parameter and the fact that the mutex is initialized
  assert(mp && mp->init == MUTEX_INITIALIZED);

  int j = 1;

  // Generate a new ticket for this thread
  int my_ticket = atomic_add_and_update(&mp->next_ticket, j);

  while((mp->prev + 1) != my_ticket) {
    // A thread which acquired the mutex earlier is running
    // Yield till it releases the lock
    kern_yield(-1);
  }

}

/** @brief Gives up the lock on a mutex
 *
 *  The calling thread gives up its claim to the lock. It is illegal
 *  for an application to unlock a mutex that is not locked.
 *
 *  @param mp The mutex to which the lock belongs
 *
 *  @return void
 */
void mutex_unlock(mutex_t *mp) {

  // Validate parameter and the fact that the mutex is initialized
  assert(mp && mp->init == MUTEX_INITIALIZED);

  // Increment the prev value which stores the ticket of the last run thread
  mp->prev++;
}
