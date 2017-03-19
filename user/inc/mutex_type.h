/** @file mutex_type.h
 *  @brief This file defines the type for mutex.
 *  @author akanjani, lramire1
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

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
} mutex_t;

#endif /* _MUTEX_TYPE_H */
