/** @file rwlock.c
 *
 *  @brief This file contains the definitions for reader writer functions
 *   It implements rwlock_init, rwlock_lock, rwlock_unlock, rwlock_destroy
 *   and rwlock_downgrade which can be used by applications for synchronization
 *   The lock implemented here gives the writers priority and no reader is 
 *   allowed to start reading if the user is waiting for the lock. This 
 *   implementation can result in starvation of readers for now.
 *
 *  @author akanjani, lramire1
 */

#include <rwlock.h>
#include <mutex.h>
#include <cond.h>
#include <simics.h>
#include <assert.h>
#include <rwlock_helper.h>

/** @brief A state of a reader writer lock meaning that a rwlock_destroy has
 *   not been called after a rwlock_init as of now.
 */
#define RWLOCK_INITIALIZED 1

/** @brief A state of a reader writer lock meaning that a rwlock_init has
 *   not been called after a rwlock_destroy as of now.
 */
#define RWLOCK_UNINITIALIZED 0

/** @brief A type of the current operation being performed by threads holding
 *   the lock. This value is set when rwlock_init has been called and no 
 *   threads currently hold any locks
 */
#define RWLOCK_INVALID 2

/** @brief Initializes a reader writer lock
 *
 *  This function initializes the reader writer lock pointed to by rwlock.
 *  The effects of using a reader writer lock before it has been initialized,
 *  or of initializing it when it is already initialized and in use
 *  are undefined.
 *
 *  @param rwlock A pointer to the reader writer lock to initialize
 *
 *  @return Zero on success, a negative number on error
 */
int rwlock_init(rwlock_t *rwlock) {

  if (!rwlock) {
    // Invalid parameter
    return -1;
  }

  if (mutex_init(&rwlock->lock) < 0) {
    // Mutex init failed
    return -1;
  }

  // Lock the mutex to change state
  mutex_lock(&rwlock->lock);

  if (cond_init(&rwlock->read_cvar) < 0 ||
      cond_init(&rwlock->write_cvar) < 0) {
    // Condition variable init failed
    return -1;
  }

  // Initialize the state of the reader writer lock
  rwlock->init = RWLOCK_INITIALIZED;
  rwlock->waiting_readers = rwlock->active_readers = 0;
  rwlock->waiting_writers = rwlock->active_writers = 0;
  rwlock->curr_op = RWLOCK_INVALID;

  // Unlock the mutex. We are done
  mutex_unlock(&rwlock->lock);

  return 0;
}

/** @brief Takes a lock to access the resource based on its type.
 *
 *  The type parameter is required to be either RWLOCK READ (for a shared 
 *  lock) or RWLOCK WRITE (for an exclusive lock). This function blocks 
 *  the calling thread until it has been granted the requested form of access.
 *
 *  @param rwlock A pointer to the reader writer lock
 *  @param type RWLOCK READ for a reader and RWLOCK WRITE for a writer
 *
 *  @return void
 */
void rwlock_lock(rwlock_t *rwlock, int type) {

  if (!rwlock || (type != RWLOCK_READ && type != RWLOCK_WRITE)) {
    // Invalid parameter(s)
    return;
  }

  // Assert that the rwlock is initialized when this function is called
  assert(rwlock->init == RWLOCK_INITIALIZED);

  if (type == RWLOCK_READ) {
    // The thread wants to read
    start_read(rwlock);
  } else {
    // The thread wants to write
    start_write(rwlock);
  }
}

/** @brief This function indicates that the calling thread is done using the 
 *   locked state in whichever mode it was granted access for. 
 *
 *  If the current lock being given up is an exclusive lock( writer was 
 *  runnning), then we check if another writer is waiting for this lock or
 *  not. If yes, we give the lock to that writer. Otherwise, we make all the
 *  reader threads waiting on this reader writer lock runnable
 *
 *  If the current lock being given up is an shared lock( reader was 
 *  running), then we check if this is the last of the running threads or not.
 *  If it isn't, we do not do anything. Otherwise, we check if any writer is
 *  waiting and if that's true, we give the writer the lock. Otherwise, we 
 *  continue;
 *
 *  @param rwlock A pointer to the reader writer lock
 *
 *  @return void
 */
void rwlock_unlock(rwlock_t *rwlock) {

  if (!rwlock) {
    // Invalid parameter
    return;
  }

  // Assert that the rwlock is initialized when this function is called
  assert(rwlock->init == RWLOCK_INITIALIZED);

  // Take the lock to check state
  mutex_lock(&rwlock->lock);

  // Assert that at least one thread is actively running with this lock
  assert(rwlock->active_readers != 0 || rwlock->active_writers != 0);

  // Unlock the lock as we are done accessing state
  mutex_unlock(&rwlock->lock);

  if (rwlock->curr_op == RWLOCK_READ) {
    // This thread was a reader
    stop_read(rwlock);
  } else {
    // This thread was a writer
    stop_write(rwlock);
  }
}

/** @brief Deactivates the reader writer lock pointed to by rwlock.
 *
 *  It is illegal for an application to use a readers/writers lock after it 
 *  has been destroyed (unless and until it is later re-initialized). 
 *  It is illegal for an application to invoke rwlock destroy() on a lock 
 *  while the lock is held or while threads are waiting on it.
 *
 *  @param rwlock A pointer to the reader writer lock
 *
 *  @return void
 */
void rwlock_destroy(rwlock_t *rwlock) {

  if (!rwlock) {
    // Invalid parameter
    return;
  }

  // Assert that the rwlock is initialized when this function is called
  assert(rwlock->init == RWLOCK_INITIALIZED);

  // Lock the mutex to check/modify state
  mutex_lock(&rwlock->lock);

  // Assert that there is no thread currently waiting/running for this rwlock
  assert(rwlock->active_readers == 0 && rwlock->active_writers == 0); 
  assert(rwlock->waiting_readers == 0 && rwlock->waiting_writers == 0);

  // Destroy the condition variables
  cond_destroy(&rwlock->read_cvar);
  cond_destroy(&rwlock->write_cvar);
  
  // Reset the initialized state
  rwlock->init = RWLOCK_UNINITIALIZED;

  // Unlock the mutex as we are done
  mutex_unlock(&rwlock->lock);

  // Destroy the mutex
  mutex_destroy(&rwlock->lock);
}

/** @brief A thread may call this function only if it already holds the lock 
 *   in RWLOCK WRITE mode at a time when it no longer requires exclusive 
 *   access to the protected resource. When the function returns: no threads 
 *   hold the lock in RWLOCK WRITE mode; the invoking thread, and possibly 
 *   some other threads, hold the lock in RWLOCK READ mode; previously blocked 
 *   or newly arriving writers must still wait for the lock to be released 
 *   entirely. During the transition from RWLOCK WRITE mode to RWLOCK READ 
 *   mode the lock should at no time be unlocked. 
 *
 *  @param rwlock A pointer to the reader writer lock
 *
 *  @return void
 */
void rwlock_downgrade(rwlock_t *rwlock) {

  if (!rwlock) {
    // Invalid parameter
    return;
  }

  if (rwlock->active_writers <= 0) {
    // No writer is running
    return;
  }

  // Lock the mutex to modify state
  mutex_lock(&rwlock->lock);

  // Decrement the number of active_writers
  rwlock->active_writers--;

  // Assert that there is now no active_writer
  assert(rwlock->active_writers == 0);

  // Increment the number of active readers
  rwlock->active_readers++;

  // Set the current operation being performed as read
  rwlock->curr_op = RWLOCK_READ;

  // Broadcast to all reader threads waiting for the read_cvar
  cond_broadcast(&rwlock->read_cvar);

  // Unlock the mutex. We are done
  mutex_unlock(&rwlock->lock);
}
