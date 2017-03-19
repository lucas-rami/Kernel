/** @file thr_yield.c
 *  @brief This file contains the definition for thethr_yield() function
 *  @author akanjani, lramire1
 */

#include <syscall.h>
#include <thr_internals.h>

/** @brief Defers execution of the invoking thread to a later time in
 *  favor of the thread with ID tid
 *
 *  If the thread with ID tid is not runnable, or doesn’t exist, then an
 *  integer error code less than zero is returned.
 *
 *  @param tid  The library issued thread id of the thread we wish to yield to.
 *  -1 makes the function yield to some unspecified thread
 *
 *  @return 0 on success, a negative number on error
 */
int thr_yield(int tid) {

  if (tid == -1) {
    return yield(-1);
  }

  // Get kernel id of the thread
  int kernel_tid = thr_get_kernel_id(tid);

  // There do not exist any thread with this library issued id
  if (kernel_tid == -1) {
    return -1;
  }

  return yield(kernel_tid);
}
