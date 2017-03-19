/** @file pcb.h
 *  @brief This file contains the declaration for the pcb_t data structure.
 *  @author akanjani, lramire1
 */

#ifndef _PCB_H_
#define _PCB_H_

#include <mutex.h>

#define TASK_RUNNING 0
#define TASK_ZOMBIE 1

typedef struct pcb {

  /* @brief The task's kernel issued id */
  int tid;

  /* @brief The task's return status */
  int return_status;

  /* @brief Task's current state: may be one of [RUNNING, ZOMBIE] */
  int task_state;

  /** @brief Mutex used to ensure atomicity when changing the task state */
  mutex_t mutex;

} pcb_t;

pcb_t pcb;

#endif /* _PCB_H_ */
