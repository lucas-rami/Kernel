
/** @file mutex.h
 *  @brief This file defines the type for mutex.
 *  @author akanjani, lramire1
 */

#ifndef _EFF_MUTEX_H_
#define _EFF_MUTEX_H_

#include <mutex.h>
#include <cond_var.h>

#define MUTEX_LOCKED 1
#define MUTEX_UNLOCKED 0

typedef struct eff_mutex {
  // mutex_t mp;
  stack_queue_t mutex_queue;
  // cond_t cv;
  int state;

} eff_mutex_t; 

int eff_mutex_init(eff_mutex_t *mp);
void eff_mutex_destroy(eff_mutex_t *mp);
void eff_mutex_lock(eff_mutex_t *mp);
void eff_mutex_unlock(eff_mutex_t *mp);

#endif
