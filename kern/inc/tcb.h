/** @file tcb.h
 *  @brief This file contains the declaration for the tcb_t data structure.
 *  @author akanjani, lramire1
 */

#ifndef _TCB_H_
#define _TCB_H_

#include <mutex.h>
#include <pcb.h>
#include <stdint.h>

/* Thread's lifecycle possible states */
#define THR_RUNNABLE 0
#define THR_RUNNING 1
#define THR_BLOCKED 2
#define THR_ZOMBIE 3

/* Constants for descheduled field */
#define THR_DESCHEDULED_FALSE 0
#define THR_DESCHEDULED_TRUE 1

typedef struct tcb {

  /* @brief The PCB of the task containing this thread */
  pcb_t *task;

  /* @brief The thread's kernel issued TID */
  int tid;

  /* @brief Thread's current state: may be one of [RUNNABLE, RUNNING, BLOCKED,
   * ZOMBIE] */
  int thread_state;

  /* @brief Indicate whether the thread is blocked due to a call to
   * deschedule() */
  char descheduled;

  /* @brief Hold the value of %esp for this thread when context switching */
  uint32_t esp;

  /* @brief Hold the value of %cr3 for this thread when context switching */
  uint32_t cr3;

  /* @brief Hold the value of %esp0 for this thread when context switching */
  uint32_t esp0;

  /* @brief Holds the number of frames requested by this thread(includes 
   *  the ones that have not been allocated yet as well */
  uint32_t num_of_frames_requested;

  /** @brief Mutex used to ensure atomicity when changing the thread state */
  mutex_t mutex;

} tcb_t;

#endif /* _TCB_H_ */
