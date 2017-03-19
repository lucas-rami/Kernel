/** @file thr_getid.c
 *  @brief This file contains the definition for the thr_getid() and
 * thr_get_kernel_id() function
 *  @author akanjani, lramire1
 */

#include <global_state.h>
#include <hash_table.h>
#include <stdlib.h>
#include <thr_internals.h>
#include <cond.h>
#include <assert.h>

/** @brief Returns the library level thread id of the current thread
 *
 *  @return The library level thread id of the current thread
 */
int thr_getid() {
  tcb_t *tcb = get_tcb();

  assert(tcb != NULL);

  return tcb->library_tid;
}

/** @brief Returns the kernel id of the thread with given library id
 *
 *  If the thread's TCB does not know the thread's kernel id,
 *  then a gettid() system call is made to collect it.
 *
 *  @param library_tid Library issued tid for the thread of which we
 *  want to know the kernel issued tid
 *
 *  @return The kernel level id if the requested thread exists, -1 otherwise
 */
int thr_get_kernel_id(int library_tid) {

  tcb_t *tcb = hash_table_get_element(&task.tcbs, &library_tid);

  assert(tcb != NULL);

  int kernel_tid;

  mutex_lock(&tcb->mutex_kernel_tid);

  // We need to wait for the parent thread to update the kernel_tid field
  if ((kernel_tid = tcb->kernel_tid) == -1) {
    cond_wait(&tcb->cond_var_kernel_tid, &tcb->mutex_kernel_tid);
    kernel_tid = tcb->kernel_tid;
  }

  mutex_unlock(&tcb->mutex_kernel_tid);

  return kernel_tid;
}

/** @brief Allows a thread to know its own kernel issued tid
 *
 *  If the kernel issued tid is unknwon at the time of check, then a gettid()
 *  system call is made to collect it. All the other threads (if any)
 *  waiting for the value of the kernel id of this thread are woken up after.
 *  If the kernel tid is already known, then the funtion returns rapidly.
 *
 *  @return The kernel level id of the calling thread
 */
int thr_get_my_kernel_id() {

  // Get TCB of current thread
  tcb_t* tcb = get_tcb();

  assert(tcb != NULL);

  mutex_lock(&tcb->mutex_kernel_tid);

  if (tcb->kernel_tid == -1) {
    mutex_unlock(&tcb->mutex_kernel_tid);
    // The kernel tid is still unknown, get it using gettid()
    tcb->kernel_tid = gettid();
    cond_broadcast(&tcb->cond_var_kernel_tid);
  } else {
    // We already know what the kernel issued tid is
    mutex_unlock(&tcb->mutex_kernel_tid);
  }

  return tcb->kernel_tid;
}
