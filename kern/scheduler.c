/** @file scheduler.c
 *  @brief This file contains the definition for the functions related to
 *  thread scheduling
 *  @author akanjani, lramire1
 */

#include <context_switch.h>
#include <kernel_state.h>
#include <scheduler.h>
#include <static_queue.h>
#include <stdlib.h>
#include <tcb.h>

#include <assert.h>

/** @brief Run the next thread in the queue of runnable threads
 *
 *  The function performs a context switch to the first thread present in
 *  the queue. If the queue is empty, then the "idle" task is ran.
 *
 *  @param current_tcb The invoking thread's tcb_t
 *
 *  @return void
 */
void run_next_thread(tcb_t *current_tcb) {

  // Check if the kernel state is initialized
  assert(kernel.init == KERNEL_INIT_TRUE);

  tcb_t *next_tcb = static_queue_dequeue(&kernel.runnable_threads);

  if (next_tcb != NULL) {
    context_switch(current_tcb, next_tcb);
  } else {
    // TODO: Run idle task when there is nothing to run
  }
}

/** @brief Make a thread runnable
 *
 *  @param tcb The TCB of the thread we want to make runnable
 *
 *  @return void
 */
void add_runnable_thread(tcb_t *tcb) {

  assert(tcb != NULL && kernel.init == KERNEL_INIT_TRUE);

  if (static_queue_enqueue(&kernel.runnable_threads, tcb) < 0) {
    panic("add_runnable_thread: Failed to add thread to runnable queue\n");
  }

  tcb->thread_state = THR_RUNNABLE;

}

/** @brief Block a thread
 *
 *  If the TCB passed as parameter is not the one of the invoking thread, then
 *  the kernel's behavior is undefined.
 *
 *  @param tcb The TCB of the thread we want to block
 *
 *  @return void
 */
void add_blocked_thread(tcb_t *tcb) {

  assert(tcb != NULL && kernel.init == KERNEL_INIT_TRUE);

  tcb->thread_state = THR_BLOCKED;

}

/** @brief Forces the kernel to run a particular thread
 *
 *  The function performs a context switch to a particular thread given as
 *  argument, overriding the scheduler's normal behavior. 
 *
 *  @param current_tcb    The invoking thread's TCB
 *  @param force_next_tcb The TCB of the thread we want to run next
 *
 *  @return void
 */
void force_next_thread(tcb_t *current_tcb, tcb_t *force_next_tcb) {

  assert(current_tcb != NULL && force_next_tcb != NULL &&
         kernel.init == KERNEL_INIT_TRUE);

  if (static_queue_remove(&kernel.runnable_threads, force_next_tcb) != 0) {
    panic("force_next_thread(): Error when calling static_queue_remove()\n");
  }

  context_switch(current_tcb, force_next_tcb);

}
