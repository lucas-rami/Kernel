/** @file rwlock_helper.c
 *
 *  @brief This file contains the definitions for helper functions for 
 *   the reader writer locks
 *
 *  @author akanjani, lramire1
 */

#include <rwlock.h>
#include <mutex.h>
#include <cond.h>
#include <assert.h>
#include <rwlock_helper.h>

/** @brief A macro for considering 1 as true
 */
#define TRUE 1

/** @brief A macro for considering 0 as false
 */
#define FALSE 0

/** @brief Entry point for a thread trying to get a read lock. 
 *
 *  The function blocks until the current thread can run according to the
 *  writers priority case of reader writer problem. Increments the number
 *  of waiting_readers to indicate another thread is waiting to read on this
 *  lock. When the condition variable is signaled and we are allowed to run,
 *  we decrement the number of waiting readers and increment the number of 
 *  active readers. We also set the current operation to read.
 *
 *  @param rwlock A pointer to the reader writer lock
 *
 *  @return void
 */
void start_read(rwlock_t *rwlock) {

  // Take a mutex before modifying state
  mutex_lock(&rwlock->lock);

  // Increment the number of waiting readers
  rwlock->waiting_readers++;

  while(wait_for_read(rwlock) == TRUE) {
    // We can't take the lock right now.
    // Wait for the state to change and try again
    cond_wait(&rwlock->read_cvar, &rwlock->lock);
  }

  // Update the new state
  rwlock->waiting_readers--;
  rwlock->active_readers++;
  rwlock->curr_op = RWLOCK_READ;

  // Release the mutex. We are done
  mutex_unlock(&rwlock->lock);
}

/** @brief Checks if the current reader thread has to wait to get the lock. 
 *
 *  The functions checks if there is any current active writer threads
 *  running or waiting on this lock. If yes, we return TRUE as the writers
 *  get priority and no reader can run in this case.
 *
 *  Otherwise, we return FALSE
 *
 *  @param rwlock A pointer to the reader writer lock
 *
 *  @return TRUE if the reader thread has to wait. Otherwise, FALSE
 */
int wait_for_read(rwlock_t *rwlock) {

  if (rwlock->active_writers > 0 || rwlock->waiting_writers > 0) {
    return TRUE;
  }
  return FALSE;
}

/** @brief Entry point for a thread trying to get a write lock. 
 *
 *  The function blocks until the current thread can run according to the
 *  writers priority case of reader writer problem. Increments the number
 *  of waiting_writers to indicate another thread is waiting to write on this
 *  lock. When the condition variable is signaled and we are allowed to run,
 *  we decrement the number of waiting writers and increment the number of 
 *  active writers. We also set the current operation to write.
 *
 *  @param rwlock A pointer to the reader writer lock
 *
 *  @return void
 */
void start_write(rwlock_t *rwlock) {

  // Take a mutex before modifying state
  mutex_lock(&rwlock->lock);

  // Increment the number of waiting writers
  rwlock->waiting_writers++;

  while(wait_for_write(rwlock) == TRUE) {
    // We can't take the lock right now.
    // Wait for the state to change and try again
    cond_wait(&rwlock->write_cvar, &rwlock->lock);
  }

  // Update the new state
  rwlock->waiting_writers--;
  rwlock->active_writers++;
  rwlock->curr_op = RWLOCK_WRITE;

  // Release the mutex. We are done
  mutex_unlock( &rwlock->lock );
}

/** @brief Checks if the current writer thread has to wait to get the lock. 
 *
 *  The functions checks if there is any current active writer threads
 *  or reader threads for this lock. If yes, we return TRUE as the writer
 *  cannot run in this case. We will have to wait either for the 
 *  writer thread or multiple reader threads to finish before we can get
 *  the lock
 *
 *  Otherwise, we return FALSE
 *
 *  @param rwlock A pointer to the reader writer lock
 *
 *  @return TRUE if the writer thread has to wait. Otherwise, FALSE
 */
int wait_for_write(rwlock_t *rwlock) {

  if (rwlock->active_readers > 0 || rwlock->active_writers > 0) {
    return TRUE;
  }
  return FALSE;
}

/** @brief Entry point for a thread trying to give up a read lock. 
 *
 *  The function decrements the number of active readers and then
 *  checks if there are no other active readers and there is a thread 
 *  waiting to get a write lock, we make the thread runnable by signalling
 *  the condition variable.  
 *
 *  @param rwlock A pointer to the reader writer lock
 *
 *  @return void
 */
void stop_read(rwlock_t *rwlock) {

  // Take a mutex before modifying state
  mutex_lock(&rwlock->lock);

  // Decrement the number of active readers
  rwlock->active_readers--;

  if (rwlock->active_readers == 0 && rwlock->waiting_writers > 0) {
    // If no other readers are running and there is at least one thread
    // waiting for a write lock, we signal the write_cvar so that the writer
    // gets to run
    cond_signal(&rwlock->write_cvar);
  }

  // Release the mutex. We are done
  mutex_unlock(&rwlock->lock);
}

/** @brief Entry point for a thread trying to give up a write lock. 
 *
 *  The function decrements the number of active writers and then
 *  checks if there are other threads waiting to get a write lock. If that
 *  is the case, we make one of those waiting threads runnable in a FIFO 
 *  fashion. Otherwise, we make all the threads waiting to get a read lock
 *  runnable.
 *
 *  @param rwlock A pointer to the reader writer lock
 *
 *  @return void
 */
void stop_write(rwlock_t *rwlock) {

  // Take a mutex before modifying state
  mutex_lock(&rwlock->lock);

  // Decrement the number of active writers
  rwlock->active_writers--;

  // Assert that no writer is active now.
  assert(rwlock->active_writers == 0);

  if (rwlock->waiting_writers > 0) {
    // At least one thread is waiting to acquire a write lock.
    // We should let that thread run
    cond_signal(&rwlock->write_cvar);
  } else {
    // No writer thread in contention. We should let the readers run
    cond_broadcast(&rwlock->read_cvar);
  }

  // Release the mutex. We are done
  mutex_unlock(&rwlock->lock);
}
