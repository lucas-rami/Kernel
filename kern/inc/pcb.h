/** @file pcb.h
 *  @brief This file contains the declaration for the pcb_t data structure.
 *  @author akanjani, lramire1
 */

#ifndef _PCB_H_
#define _PCB_H_

#define TASK_RUNNING 0
#define TASK_ZOMBIE 1

typedef struct pcb {

  /* The task's kernel issued id */
  int tid;

  /* The task's return status */
  int return_status;

  /* Task's current state: may be one of [RUNNING, ZOMBIE] */
  int task_state;

} pcb_t;

pcb_t pcb;

#endif /* _PCB_H_ */
