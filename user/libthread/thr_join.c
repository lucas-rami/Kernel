/** @file thr_join.c
 *  @brief This file contains the definition for the thr_join() function
 *  @author akanjani, lramire1
 */

#include <global_state.h>
#include <hash_table.h>
#include <stdlib.h>
#include <syscall.h>
#include <thr_internals.h>
#include <cond.h>

/** @brief Cleans up after a thread, optionnaly returning the status information
 *  provided by the thread at the time of exit
 *
 *  The target thread tid may or may not have exited before thr join() is
 *  called; if it has not, the calling thread will be suspended until the target
 *  thread does exit.
 *
 *  If statusp is not NULL, the value passed to thr exit() by the joined thread
 *  will be placed in the location referenced by statusp.
 *
 *  @param tid      Target thread tid
 *  @param statusp  Pointer to the thread's exit status (void*)
 *
 *  @return 0 on success, a negative number on error
 */
int thr_join(int tid, void **statusp) {

  // Get the TCB of the thread we want to join on
  tcb_t *tcb = hash_table_get_element(&task.tcbs, &tid);

  // The thread already exited and was joined on or the tid is invalid
  if (tcb == NULL) {
    return -1;
  }

  mutex_lock(&tcb->mutex_state);

  // If another thread is already waiting, return immediatly
  if (tcb->thread_state == WAITING_ON) {
    mutex_unlock(&tcb->mutex_state);
    return -1;
  }


  // The thread is still running
  if (tcb->thread_state == RUNNING) {
    tcb->thread_state = WAITING_ON;
    cond_wait(&tcb->cond_var_state, &tcb->mutex_state);
  }
  // When we get here the thread has exited and we can clean things up
  tcb->thread_state = WAITING_ON;

  // We don't need the mutex anymore
  mutex_unlock(&tcb->mutex_state);

  // Take care of returning the status of the exited thread
  if (statusp != NULL) {
    *statusp = tcb->return_status;
  }

  // Remove TCB from hash table
  hash_table_remove_element(&task.tcbs, &tid);

  // Mark the deallocated pages as free to use for other threads if this isn't
  // the root thread
  if ((unsigned int)tcb->stack_low !=
      (unsigned int)task.stack_highest_childs + PAGE_SIZE) {

    queue_insert_node(&task.stack_queue, tcb->stack_high);
  }

  // Free the TCB data structure
  free(tcb);

  return 0;
}
