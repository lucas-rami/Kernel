  /** @file scheduler.c
 *  @brief This file contains the definition for the functions related to
 *  thread scheduling
 *  @author akanjani, lramire1
 */

#include <asm.h>
#include <context_switch.h>
#include <kernel_state.h>
#include <scheduler.h>
#include <static_queue.h>
#include <stdlib.h>
#include <tcb.h>

#include <assert.h>
#include <simics.h>

/** @brief Run the next thread in the queue of runnable threads
 *
 *  The function performs a context switch to the first thread present in
 *  the queue. If the queue is empty, then the "idle" task is ran.
 *
 *  @param current_tcb The invoking thread's tcb_t
 *
 *  @return void
 */
void run_next_thread() {

  //lprintf("Run next thread() called\n");
  // Check if the kernel state is initialized
  assert(kernel.current_thread != NULL && kernel.init == KERNEL_INIT_TRUE);

  tcb_t *next_tcb = static_queue_dequeue(&kernel.runnable_threads);

  //lprintf("Run next thread() called. Dequed thread with tid %d\n", next_tcb->tid);
  if (next_tcb != NULL) {
    kernel.cpu_idle = CPU_IDLE_FALSE;

    if (next_tcb == kernel.current_thread) {
      lprintf("run_next_thread(): Just avoided context switching to myself !");
      enable_interrupts();
      return;
    }

    //lprintf("run_next_thread(): Running new thread !");

    context_switch(next_tcb);
  } else {

    if (kernel.cpu_idle) {
      lprintf("run_next_thread(): Just avoided context switching to myself "
              "(idle thread) !");
      enable_interrupts();
      return;
    }

    //lprintf("run_next_thread(): Nothing to run, idling now...");

    kernel.cpu_idle = CPU_IDLE_TRUE;
    context_switch(kernel.idle_thread);
  }
}

/** @brief Change the invoking thread's state to runnable and then context
 *  switch to another thread
 *
 *  @return void
 */
void make_runnable_and_switch() {

  //lprintf("Make runnable and switch() called\n");
  assert(kernel.current_thread != NULL && kernel.init == KERNEL_INIT_TRUE);

  disable_interrupts();

  if (kernel.cpu_idle == CPU_IDLE_FALSE) {
    if (static_queue_enqueue(&kernel.runnable_threads, kernel.current_thread) <
        0) {
      panic("make_runnable_and_switch(): Failed to add thread to runnable "
            "queue\n");
    }
  }
  kernel.current_thread->thread_state = THR_RUNNABLE;

  //lprintf("Calling run next thread\n");
  run_next_thread();
}

/** @brief Block the invoking thread and then context switch to another thread
 *
 *  @return void
 */
void block_and_switch() {

  assert(kernel.init == KERNEL_INIT_TRUE);

  if (kernel.current_thread != NULL && kernel.cpu_idle == CPU_IDLE_FALSE) {
    kernel.current_thread->thread_state = THR_BLOCKED;
    kernel.current_thread->descheduled = THR_DESCHEDULED_TRUE;
  }

  run_next_thread();
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
    panic("add_runnable_thread(): Failed to add thread to runnable queue\n");
  }

  tcb->thread_state = THR_RUNNABLE;
  tcb->descheduled = THR_DESCHEDULED_FALSE;
}

/** @brief Forces the kernel to run a particular thread
 *
 *  The function performs a context switch to a particular thread given as
 *  argument, overriding the scheduler's normal behavior. The invoking thread is
 *  put in the queue of runnable threads.
 *
 *  @param force_next_tcb The TCB of the thread we want to run next
 *
 *  @return void
 */
void force_next_thread(tcb_t *force_next_tcb) {

  assert(kernel.current_thread != NULL && force_next_tcb != NULL &&
         kernel.init == KERNEL_INIT_TRUE);

  if (static_queue_remove(&kernel.runnable_threads, force_next_tcb) != 0) {
    panic("force_next_thread(): Error when calling static_queue_remove()\n");
  }

  kernel.current_thread->thread_state = THR_RUNNABLE;

  mutex_unlock(&kernel.current_thread->mutex);

  context_switch(force_next_tcb);
}
