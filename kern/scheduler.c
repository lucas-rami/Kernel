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

  generic_node_t *next_thread = kernel.runnable_head;
  if (next_thread == NULL) {
    return kernel.idle_thread;
  }

  kernel.runnable_head = next_thread->next;
  return next_thread->value;

}

/** @brief Change the invoking thread's state to runnable and then context
 *  switch to another thread
 *
 *  @return void
 */
void make_runnable_and_switch() {

  assert(kernel.current_thread != NULL && kernel.init == KERNEL_INIT_TRUE);

  generic_node_t new_tail = {kernel.current_thread, NULL};

  disable_interrupts();

  kernel.current_thread->thread_state = THR_RUNNABLE;

  if (kernel.cpu_idle == CPU_IDLE_TRUE) {
    context_switch(next_thread());
    return;
  }

  if(kernel.runnable_head != NULL) {
    kernel.runnable_tail->next = &new_tail;
    kernel.runnable_tail = &new_tail;
  } else {
    kernel.runnable_head = (kernel.runnable_tail = &new_tail);
  }

  context_switch(next_thread());
  
}

/** @brief Block the invoking thread and then context switch to another thread
 *
 *  @param is_descheduled Indicates whether the thread is being descheduled
 *    because of a system call to deschedule()
 *
 *  @return void
 */
void block_and_switch(int is_descheduled) {

  lprintf("BLock and switch");
  assert(kernel.current_thread != NULL && kernel.init == KERNEL_INIT_TRUE);
  assert(is_descheduled == THR_DESCHEDULED_TRUE ||
          is_descheduled == THR_DESCHEDULED_FALSE);

  disable_interrupts();

  mutex_unlock(&kernel.current_thread->mutex);

  if (kernel.cpu_idle == CPU_IDLE_TRUE) {
    context_switch(next_thread());
    return;
  }

  kernel.current_thread->thread_state = THR_BLOCKED;
  kernel.current_thread->descheduled = is_descheduled;

  lprintf("Context switching to next thread");
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

  generic_node_t new_tail = {tcb, NULL};

  lprintf("Adding runnable thread %d", tcb->tid);
  disable_interrupts();

  // Should not happen
  if (tcb == kernel.idle_thread) {
    enable_interrupts();    
    return;
  }

  tcb->thread_state = THR_RUNNABLE;
  tcb->descheduled = THR_DESCHEDULED_FALSE;

  generic_node_t * node_addr = (generic_node_t *)(tcb->esp0 - PAGE_SIZE);
  *(node_addr) = new_tail;
  
  if(kernel.runnable_head != NULL) {
    kernel.runnable_tail->next = node_addr;
    kernel.runnable_tail = node_addr;
  } else {
    kernel.runnable_head = (kernel.runnable_tail = node_addr);
  }

  lprintf("Added runnable thread %d", tcb->tid);
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

  generic_node_t new_tail = {kernel.current_thread, NULL};

  disable_interrupts();    
  mutex_unlock(&force_next_tcb->mutex);  

  // Should not happen 
  if (kernel.cpu_idle == CPU_IDLE_TRUE) {
    context_switch(force_next_tcb);
    return;
  }

  if(kernel.runnable_head != NULL) {
    kernel.runnable_tail->next = &new_tail;
    kernel.runnable_tail = &new_tail;
  } else {
    kernel.runnable_head = (kernel.runnable_tail = &new_tail);
  }

  context_switch(force_next_tcb);

}
