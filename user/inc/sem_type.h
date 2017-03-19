/** @file sem_type.h
 *  @brief This file defines the type for semaphores.
 */

#ifndef _SEM_TYPE_H
#define _SEM_TYPE_H

#include <cond_type.h>
#include <mutex_type.h>

/** @brief A structure of a semaphore
 */
typedef struct sem {

  /** @brief An int which stores the current state of the semaphore. It can be
   *   SEM_INITIALIZED when the semaphore has been initialized, or 
   *   SEM_UNINITIALIZED when the semaphore has been destroyed
   */
  int init;

  /** @brief An int storing the number of threads that have been made woken up
   *   by the semaphore but haven't started executing yet
   */
  int wokenup_waiting;

  /** @brief An int storing the number of resources available for this 
   *   semaphore which also means it is the maximum number of threads 
   *   which can run in paralled while holding this semaphore
   */
  int available_resources;

  /** @brief A condition variable that waits for the state to change so that
   *   a thread can run if the number of available resources becomes greater
   *   than zero
   */
  cond_t cvar;

  /** @brief A mutex which ensures atomicity and mutual exclusion amongst the
   *   various semapahore functions
   */
  mutex_t lock;

} sem_t;

#endif /* _SEM_TYPE_H */
