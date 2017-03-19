/** @file global_state.h
 *  @brief This file defines data structures for the task's state and thread's
 *   TCB
 *  @author akanjani, lramire1
 */

#ifndef _GLOBAL_STATE_H
#define _GLOBAL_STATE_H

#include <mutex.h>
#include <cond_type.h>
#include <syscall.h>
#include <hash_table.h>

/** @brief State of a thread which means that a thread has joined this thread
 */
#define WAITING_ON 0

/** @brief State of a thread which means that the thread has exited
 */
#define EXITED 1

/** @brief State of a thread which means that the thread is still running
 */
#define RUNNING 2

/** @brief A structure for the thread control block. It contains the library
 *   thread id, the lowest address of thread's stack space, the highest address
 *   of the thread's stack space, the thread's return status, its kernel id,
 *   two condition variables, two mutexes and the thread state.
 */
typedef struct tcb {

  /** @brief Thread's id (provided by the thread library)
   */
  int library_tid;
  /** @brief Lowest address of thread's stack space
   */ 
  unsigned int *stack_low;
  /** @brief Highest address of thread's stack space
   */ 
  unsigned int *stack_high;
  /** @brief Thread's return status
   */ 
  void* return_status;


  /* library_tid, stack_low and stack_high do not need to be protected by a
   * lock since their value is defined before the thread is created.
   *
   * return_status is written to only by the thread owning this TCB, so
   * no lock is needed for it neither. */

  /*------------------------------*/

  /** @brief Thread's id (provided by the kernel)
   */
  int kernel_tid;
  /** @brief Condition variable for kernel_tid
   */
  cond_t cond_var_kernel_tid;
  /** @brief Mutex for manipulation the kernel_tid field
   */
  mutex_t mutex_kernel_tid;

  /*------------------------------*/

  /** @brief Threads state (either WAITING_ON, EXITED, or RUNNING)
   */
  int thread_state;
  /** @brief Condition variable for thr_exit() and thr_join()
   */
  cond_t cond_var_state;
  /** @brief Mutex for manipulating the thread's state field
   */
  mutex_t mutex_state;

} tcb_t;

/** @brief A structure that represents a task
 */
typedef struct task {

  /** @brief Size for each thread's stack
   */
  unsigned int stack_size;
  /** @brief Lowest address of task's threads stacks (initialized by autostack())
   */
  unsigned int *stack_highest;
  /** @brief Highest address for (grand)child threads of first thread
   */
  unsigned int* stack_highest_childs;

  /* The above fields do not need to be protected by a lock since
   * their value does not change after thr_init() has returned */
  /*------------------------------*/

  /** @brief Lowest address of task's threads stacks (initialized by autostack())
   */
  unsigned int *stack_lowest;
  /** @brief Thread library tids
   */
  unsigned int tid;
  /** @brief Spinlock for task's state access
   */
  mutex_t state_lock;

  /*------------------------------*/

  /** @brief Queue for free stack spaces
   */
  generic_queue_t stack_queue;

  /*------------------------------*/

  /** @brief Data structure holding the TCB of all threads in the task
   */
  generic_hash_table_t tcbs;

  /*------------------------------*/

  /** @brief TCB of root thread in the task
   */ 
  tcb_t* root_tcb;

} task_t;

/** @brief GLobal state for the current task
 */
task_t task;

/** @brief Page_sized exception stack for root thread in task
 */
int exception_handler_stack[PAGE_SIZE];

#endif /* _GLOBAL_STATE_H */
