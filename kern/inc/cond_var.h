/** @file cond_var.h
 *  @brief This file defines the type for condition variables.
 *  @author akanjani, lramire1
 */

#ifndef _COND_VAR_H
#define _COND_VAR_H

#include <mutex.h>
#include <stack_queue.h>

/** @brief A structure representing a condition variable
 */
typedef struct cond {

  /** @brief Stores the current state of the condition variable
   *   It is either CVAR_INITIALIZED when cond_init has been called
   *   or CVAR_UNINITIALIZED if a cond_destroy has been called
   */
  int init;

  /** @brief Waiting queue */
  stack_queue_t waiting_queue;

  /** @brief Mutex to modify the waiting queue atomically */
  mutex_t mp;

} cond_t;

int cond_init( cond_t *cv );
void cond_destroy( cond_t *cv );
void cond_wait( cond_t *cv, mutex_t *mp );
void cond_signal( cond_t *cv );
void cond_broadcast( cond_t *cv );

#endif /* _COND_VAR_H */