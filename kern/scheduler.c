  /** @file scheduler.c
 *  @brief This file contains the definition for the functions related to
 *  thread scheduling
 *  @author akanjani, lramire1
 */

#include <asm.h>
#include <context_switch.h>
#include <kernel_state.h>
#include <scheduler.h>
#include <generic_node.h>
#include <stdlib.h>
#include <tcb.h>
#include <page.h>

#include <assert.h>
#include <simics.h>

/** @brief Return the next thread to run from the queue of runnable threads
 *
 *  @return The next thread's TCB
 */
tcb_t *next_thread() {

  // Check if the kernel state is initialized
  assert(kernel.current_thread != NULL && kernel.init == KERNEL_INIT_TRUE);

  generic_node_t *next_thread = stack_queue_dequeue(&kernel.runnable_queue);
  if (next_thread == NULL) {
    return kernel.idle_thread;
  }

  return next_thread->value;

}

/** @brief Change the invoking thread's state to runnable and then context
 *  switch to another thread
 *
 *  @return void
 */
void make_runnable_and_switch() {

  assert(kernel.current_thread != NULL && kernel.init == KERNEL_INIT_TRUE);

  disable_interrupts();

  kernel.current_thread->thread_state = THR_RUNNABLE;

  if (kernel.cpu_idle == CPU_IDLE_TRUE) {
    context_switch(next_thread());
    return;
  }

  lprintf("\tmake_runnable_and_switch(): Enqueueing %d", kernel.current_thread->tid);  

  generic_node_t new_tail = {kernel.current_thread, NULL};
  stack_queue_enqueue(&kernel.runnable_queue, &new_tail);

  context_switch(next_thread());
  
}

/** @brief Block the invoking thread and then context switch to another thread
 *
 *  @param holding_mutex Indicates whether the function should unlock the mutex
 *    on the invoking thread's state
 *
 *  @return void
 */
void block_and_switch(int holding_mutex) {

  assert(kernel.current_thread != NULL && kernel.init == KERNEL_INIT_TRUE);

  disable_interrupts();

  lprintf("\tblock_and_switch(): Blocking thread %d", kernel.current_thread->tid);

  if (holding_mutex == HOLDING_MUTEX_TRUE) {
    eff_mutex_unlock(&kernel.current_thread->mutex);
  }

  if (kernel.cpu_idle == CPU_IDLE_TRUE) {
    context_switch(next_thread());
    return;
  }

  kernel.current_thread->thread_state = THR_BLOCKED;

  context_switch(next_thread());

}

/** @brief Make a thread runnable
 *
 *  @param tcb The TCB of the thread we want to make runnable
 *
 *  @return void
 */
void add_runnable_thread(tcb_t *tcb) {

  assert(tcb != NULL && kernel.init == KERNEL_INIT_TRUE);

  // Reject the call if the thread is already in the runnable queue
  if (tcb->thread_state == THR_RUNNABLE) {
    return;
  }

  lprintf("\tadd_runnable_thread(): Thread %d adding %d to runnable queue", kernel.current_thread->tid, tcb->tid);

  disable_interrupts();

  // Should not happen
  if (tcb == kernel.idle_thread) {
    enable_interrupts();    
    return;
  }

  tcb->thread_state = THR_RUNNABLE;

  // Create node at the lowest address of the thread's kernel stack
  generic_node_t new_tail = {tcb, NULL};
  generic_node_t * node_addr = (generic_node_t *)(tcb->esp0 - PAGE_SIZE);
  *(node_addr) = new_tail;

  // Enqueue the thread
  stack_queue_enqueue(&kernel.runnable_queue, node_addr);

  enable_interrupts();

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

  disable_interrupts();    
  eff_mutex_unlock(&force_next_tcb->mutex);  

  lprintf("force_next_thread(): Forcing thread %d", force_next_tcb->tid);

  kernel.current_thread->thread_state = THR_RUNNABLE;

  // Should not happen 
  if (kernel.cpu_idle == CPU_IDLE_TRUE) {
    context_switch(force_next_tcb);
    return;
  }

  // Create new node for runnable queue
  generic_node_t new_tail = {kernel.current_thread, NULL};

  // Enqueue the current thread
  stack_queue_enqueue(&kernel.runnable_queue, &new_tail);

  generic_node_t *it = kernel.runnable_queue.head, *prev = NULL;
  while (it != NULL) {
    if (it->value == force_next_tcb) {
      if (prev == NULL) {
        kernel.runnable_queue.head = it->next;
      } else {
        prev->next = it->next;
      }
    }
    prev = it;
    it = it->next;
  }


  context_switch(force_next_tcb);

}
