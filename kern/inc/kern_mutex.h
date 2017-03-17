/** @file kern_mutex.h
 *  @brief This file defines the type and functions for the kern_mutex_t
 *  structure.
 *  @author akanjani, lramire1
 */

#ifndef _KERN_MUTEX_H_
#define _KERN_MUTEX_H_

/** @brief The structure of a mutex
 */
typedef struct kern_mutex {

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

} kern_mutex_t;

int kern_mutex_init(kern_mutex_t* mp);
void kern_mutex_destroy(kern_mutex_t* mp);

void kern_mutex_lock(kern_mutex_t* mp);
void kern_mutex_unlock(kern_mutex_t* mp);

#endif /* _KERN_MUTEX_H_ */
