/** @file thr_exit.c
 *  @brief This file contains the definition for the thr_exit() function
 *  @author akanjani, lramire1
 */

#include <global_state.h>
#include <stdlib.h>
#include <syscall.h>
#include <thr_internals.h>
#include <cond.h>

/** @brief Exit the thread with an exit status
 *
 *  @param status The thread's exit status
 *
 *  @return Do not return
 */

void thr_exit(void *status) {

  // Get the thread's TCB
  tcb_t *tcb = get_tcb();

  // Should not happen
  if (tcb == NULL) {
    // panic("thr_exit(): Thread's TCB does not exist !\n");
  }

  // Set return status
  tcb->return_status = status;

  mutex_lock(&tcb->mutex_state);

  if (tcb->thread_state == WAITING_ON) {
    // Some other thread has called thr_join() on this thread
    cond_signal(&tcb->cond_var_state);
  } else {
    // Set the thread's state to EXITED
    tcb->thread_state = EXITED;
  }

  mutex_unlock(&tcb->mutex_state);

  // Vanish the current thread
  vanish();
}
