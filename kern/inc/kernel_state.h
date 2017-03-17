/** @file kernel_state.h
 *  @brief This file contains the declaration for the kernel_state_t data
 *  structure.
 *  @author akanjani, lramire1
 */

#ifndef _KERNEL_STATE_H_
#define _KERNEL_STATE_H_

#include <static_queue.h>
#include <tcb.h>
#include <kern_mutex.h>

#define KERNEL_INIT_FALSE 0
#define KERNEL_INIT_TRUE 1

typedef struct kernel {

  /* @brief Holds the TCB of the thread currently running,
   * only works for kernel running on uniprocessor */
  tcb_t* current_thread;

  /* @brief Hold the task id that should be assigned to the next
   * task created, the value is incremented each time a task is created */
  int task_id;

  /* @brief Hold the thread id that should be assigned to the next
   * thread created, the value is incremented each time a thread is created */
  int thread_id;

  /* @brief Queue containing the list of runnable threads, in the order
   * that they should be run in */
  static_queue_t runnable_threads;

  /** @brief Indicate whether the kernel state is initialized or not.
    * The queue should be initialized by calling kernel_init() */
  char init;

  /** @brief Mutex used to ensure atomicity when changing the kernel state */
  kern_mutex_t mutex;

} kernel_t;

/* Hold the kernel state*/
kernel_t kernel;


int kernel_init();


#endif /* _KERNEL_STATE_H_ */
