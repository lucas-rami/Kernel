/** @file mutex.h
 *  @brief This file defines the type for mutex.
 *  @author akanjani, lramire1
 */

#ifndef _EFF_MUTEX_H_
#define _EFF_MUTEX_H_

#define MUTEX_LOCKED 1
#define MUTEX_UNLOCKED 0

#include <stack_queue.h>

/** @brief A mutex implementation using a waiting queue */
typedef struct eff_mutex {
  
  /** @brief A waiting queue for threads waiting for the mutex to be unlocked */
  stack_queue_t mutex_queue;

  /** @brief The mutex's state, either MUTEX_LOCKED or MUTEX_UNLOCKED */
  int state;

  /** @brief The tid of the mutex's owner */
  int owner;

} eff_mutex_t; 

int eff_mutex_init(eff_mutex_t *mp);
void eff_mutex_destroy(eff_mutex_t *mp);
void eff_mutex_lock(eff_mutex_t *mp);
void eff_mutex_unlock(eff_mutex_t *mp);

#endif /* _EFF_MUTEX_H_ */
