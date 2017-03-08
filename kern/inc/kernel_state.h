/** @file kernel_state.h
 *  @brief This file contains the declaration for the kernel_state_t data
 *  structure.
 *  @author akanjani, lramire1
 */

#ifndef _KERNEL_STATE_H_
#define _KERNEL_STATE_H_

#include <pcb.h>
#include <tcb.h>

typedef struct kernel {

  /* Holds the TCB of the thread currently running,
   * only works for kernel running on uniprocessor */
  tcb_t* current_thread;

  /* Holds the PCB of the task currently running,
   * only works for kernel running on uniprocessor */
  pcb_t* current_task;

  /* Hold the task id that should be assigned to the first
   * task created, the value is incremented each time a task is created */
  int task_id;

  /* Hold the thread id that should be assigned to the first
   * thread created, the value is incremented each time a thread is created */
  int thread_id;

} kernel_t;

kernel_t kernel;

#endif /* _KERNEL_STATE_H_ */
