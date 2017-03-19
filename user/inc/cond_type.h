/** @file cond_type.h
 *  @brief This file defines the type for condition variables.
 *  @author akanjani, lramire1
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include <queue.h>
#include <mutex_type.h>

/** A structure of a condition variable
 */
typedef struct cond {

  /** @brief Stores the current state of the condition variable
   *   It is either CVAR_INITIALIZED when cond_init has been called
   *   or CVAR_UNINITIALIZED if a cond_destroy has been called
   */
  int init;

  /** @brief A generic_queue_t type member which is used to store the
   *   the TIDs of all the thread waiting for this condition variable
   */
  generic_queue_t waiting_queue;
} cond_t;

#endif /* _COND_TYPE_H */
