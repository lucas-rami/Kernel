/** @file tcb.h
 *  @brief This file contains the declaration for the tcb_t data structure.
 *  @author akanjani, lramire1
 */

#ifndef _TCB_H_
#define _TCB_H_

#include <pcb.h>

#define THR_RUNNABLE 0
#define THR_RUNNING 1
#define THR_BLOCKED 2
#define THR_ZOMBIE 3

typedef struct tcb {

  /* The PCB of the task containing this thread */
  pcb_t* task;

  /* The thread's kernel issued TID */
  int tid;

  /* Thread's current state: may be one of [RUNNABLE, RUNNING, BLOCKED, ZOMBIE] */
  int thread_state;

} tcb_t;

tcb_t tcb;

#endif /* _TCB_H_ */
