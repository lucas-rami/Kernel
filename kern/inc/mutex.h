/** @file mutex.h
 *  @brief This file defines the type for mutex.
 *  @author akanjani, lramire1
 */

#ifndef _MUTEX_H
#define _MUTEX_H

/** @brief The structure of a mutex
 */
typedef struct mutex {

  /** @brief An int which stores the ticket number of the thread which ran last
   */
  int prev;

  /** @brief An int which stores the ticket number which should be given to the
   *   next thread which tries to acquire this lock
   */
  int next_ticket;

  /** @brief An int which stores whether the miutex has been initialized or not
   */
  int init;

  /* @brief Hold the tid of the thread holding the mutex*/
  int tid_owner;

} mutex_t;

int mutex_init( mutex_t *mp );
void mutex_destroy( mutex_t *mp );
void mutex_lock( mutex_t *mp );
void mutex_unlock( mutex_t *mp );

#endif /* _MUTEX_H */
