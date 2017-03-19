/** @file thr_init.c
 *  @brief This file contains the definition for the thr_init() function
 *  @author akanjani, lramire1
 */

#include <global_state.h>
#include <hash_table.h>
#include <page_fault_handler.h>
#include <stdlib.h>
#include <syscall.h>
#include <thread.h>
#include <cond.h>

/** @brief Number of buckets for the hash table containing the TCBs
 */
#define NB_BUCKETS_TCB 32

/** @brief Initialize the thread library
 *
 *  This function should be called exactly once before any other
 *  function from the thread library.
 *
 *  @param size The amount of stack space which will be available for
 *  each thread using the thread library
 *
 *  @return Zero on success, a negative number on erro
 */
int thr_init(unsigned int size) {

  // Check validity of argument
  if (size == 0) {
    return -1;
  }

  int mod = size % PAGE_SIZE;
  if (mod != 0) {
    size += (PAGE_SIZE - mod);
  }

  // Initialize the task's global state

  // Initialize data structures
  if (queue_init(&task.stack_queue) < 0 || hash_table_init(&task.tcbs,
      NB_BUCKETS_TCB, find_tcb, hash_function_tcb) < 0) {
    return -1;
  }

  // Initialize mutexes
  if (mutex_init(&task.state_lock) < 0) {
    return -1;
  }

  // Create TCB for current task
  tcb_t *tcb = malloc(sizeof(tcb_t));
  if (tcb == NULL) {
    return -1;
  }

  // Initialize the TCB
  tcb->return_status = NULL;
  tcb->kernel_tid = gettid();
  tcb->library_tid = 0;
  tcb->stack_low = task.stack_lowest;
  tcb->stack_high = task.stack_highest;
  tcb->thread_state = RUNNING;

  // Initialize the TCB's mutex and  condition variable
  if (cond_init(&tcb->cond_var_state) < 0 ||
      mutex_init(&tcb->mutex_state) < 0 ||
      cond_init(&tcb->cond_var_kernel_tid) < 0 ||
      mutex_init(&tcb->mutex_kernel_tid) < 0) {
    free(tcb);
    return -1;
  }

  // Add the current thread's TCB to the hash table
  if (hash_table_add_element(&task.tcbs, tcb) < 0) {
    free(tcb);
    return -1;
  }

  // Finish to initialize the task's global state
  task.stack_size = size;
  task.tid = 1;
  task.stack_highest_childs = (unsigned int*)((unsigned int)task.stack_lowest
                               - PAGE_SIZE);
  task.root_tcb = tcb;

  // Register new exception handler (no automatic stack growth)
  swexn(exception_handler_stack, multithread_handler, NULL, NULL);

  return 0;
}
