/** @file tcb.h
 *  @brief This file contains the declaration for the tcb_t data structure.
 *  @author akanjani, lramire1
 */

#ifndef _TCB_H_
#define _TCB_H_

#include <mutex.h>
#include <pcb.h>
#include <ureg.h>
#include <stdint.h>

/* Thread's lifecycle possible states */
#define THR_RUNNABLE 0
#define THR_RUNNING 1
#define THR_BLOCKED 2
#define THR_ZOMBIE 3

typedef void (*swexn_handler_t)(void *arg, ureg_t *ureg);

typedef struct {
  void *esp3;
  swexn_handler_t eip;
  void *arg;
} swexn_struct_t;

typedef struct tcb {

  /* @brief The PCB of the task containing this thread */
  pcb_t *task;

  /* @brief The thread's kernel issued TID */
  int tid;

  /* @brief Thread's current state: may be one of [RUNNABLE, RUNNING, BLOCKED,
   * ZOMBIE] */
  int thread_state;

  /* @brief Hold the value of %esp for this thread when context switching */
  uint32_t esp;

  /* @brief Hold the value of %cr3 for this thread when context switching */
  uint32_t cr3;

  /* @brief Hold the value of %esp0 for this thread when context switching */
  uint32_t esp0;

  /* @brief Holds the number of frames requested by this thread(includes 
   *  the ones that have not been allocated yet as well */
  uint32_t num_of_frames_requested;

  /* @brief Holds the software exception handler information
   */
  swexn_struct_t swexn_values;

  /* @brief A pointer to the pcb structure of the task which will be reaped
   *  by this thread
   */
  pcb_t *reaped_task;

  /** @brief Mutex used to ensure atomicity when changing the thread state */
  mutex_t mutex;

} tcb_t;

#endif /* _TCB_H_ */
