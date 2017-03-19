/** @file rwlock_type.h
 *  @brief This file defines the type for reader/writer locks.
 *  @author akanjani, lramire1
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <mutex_type.h>
#include <cond_type.h>

/** @brief A structure of a reader writer lock
 */
typedef struct rwlock {

  /** @brief An int storing the number of threads waiting to acquire the
   *   read lock
   */
  int waiting_readers;

  /** @brief An int storing the number of threads which currently have the
   *   read lock
   */
  int active_readers;

  /** @brief An int storing the number of threads waiting to acquire the
   *   write lock
   */
  int waiting_writers;

  /** @brief An int storing the number of threads which currently have the
   *   write lock
   */
  int active_writers;

  /** @brief An int storing the current operation being performed by threads 
   *   holding this lock. It can be RWLOCK_READ when they are reading,
   *   RWLOCK_WRITE when they are writing or RWLOCK_INVALID if no lock has 
   *   been acquired yet 
   */
  int curr_op;

  /** @brief An int which stores the current state of the rwlock. It can be
   *   RWLOCK_INITIALIZED when the lock has been initialized, or 
   *   RWLOCK_UNINITIALIZED when the lock has been destroyed
   */
  int init;

  /** @brief A mutex for this read write lock to ensure atomicity and mutual
   *   exclusion amongst the various rwlock functions
   */ 
  mutex_t lock;

  /** @brief A condition variable for the threads which want to take a read 
   *   lock
   */
  cond_t read_cvar;

  /** @brief A condition variable for the threads which want to take a write
   *   lock
   */
  cond_t write_cvar;

} rwlock_t;

#endif /* _RWLOCK_TYPE_H */
