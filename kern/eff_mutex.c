/** @file mutex.c
 *  @brief This file contains the definitions for mutex_type.h functions
 *  @author akanjani, lramire1
 */

#include <eff_mutex.h>
#include <simics.h>
#include <atomic_ops.h>
#include <syscall.h>
#include <assert.h>
#include <syscalls.h>
#include <eff_mutex.h>
#include <asm.h>
#include <kernel_state.h>
#include <stddef.h>
#include <scheduler.h>

/** @brief  Initializes an eff_mutex
 *
 *  This function must be called once before using the eff_mutex. Doing
 *  otherwise will result in undefined behavior.
 *
 *  @param  mp  A pointer to an eff_mutex
 *
 *  @return 0 on success, a negative number on error
 */
int eff_mutex_init(eff_mutex_t *mp) {

  // Check argument
  if (mp == NULL) {
    return -1;
  }

  stack_queue_init(&mp->mutex_queue);

  mp->state = MUTEX_UNLOCKED;
  mp->owner = -1;
  return 0;
}

/** @brief  Destroys an eff_mutex
 *
 *  This function should be called only after eff_mutex has been initialized
 *  previously with a call to eff_mutex_init(). After this call returns,one may
 *  reuse this eff_mutex by calling eff_mutex_init again.
 *
 *  @param  mp  A pointer to an eff_mutex
 *
 *  @return void
 */ 
void eff_mutex_destroy(eff_mutex_t *mp) {

  // Check argument
  assert(mp != NULL);

  disable_interrupts();
  assert(is_stack_queue_empty(&mp->mutex_queue));
  stack_queue_destroy(&mp->mutex_queue);
  enable_interrupts();

}

/** @brief  Acquires the lock on an eff_mutex
 *
 *  If another thread is already holding this mutex, the invoking thread's is
 *  descheduled until the mutex is available for it.
 *
 *  @param  mp  A pointer to an eff_mutex
 *
 *  @return void
 */
void eff_mutex_lock(eff_mutex_t *mp) {
  
  if (kernel.kernel_ready != KERNEL_READY_TRUE) {
    return;
  }

  // Validate parameter and the fact that the mutex is initialized
  assert(mp != NULL);
  disable_interrupts();
  if (mp->owner == kernel.current_thread->tid) {
    enable_interrupts();
    return;
  }
  while (mp->state == MUTEX_LOCKED) {
    disable_interrupts();
    generic_node_t tmp;
    tmp.value = (void *)kernel.current_thread;
    tmp.next = NULL;
    stack_queue_enqueue(&mp->mutex_queue, &tmp);
    // This call will enable interrupts
    block_and_switch(HOLDING_MUTEX_FALSE, NULL);
  }
  mp->state = MUTEX_LOCKED;
  mp->owner = kernel.current_thread->tid;
  enable_interrupts();
}

/** @brief  Releases the lock on the mutex
 *
 *  The function wakes up the next thread in the queue(if any) before returning
 *  so that another thread may take the mutex. The invoking thread should be
 *  holding this mutex before calling the function.
 *
 *  @param  mp  A pointer to an eff_mutex
 *
 *  @return void
 */
void eff_mutex_unlock(eff_mutex_t *mp) {
  if (kernel.kernel_ready != KERNEL_READY_TRUE) {
    return;
  }
  assert(mp != NULL);
  generic_node_t *tmp = stack_queue_dequeue(&mp->mutex_queue);
  if (tmp) {
    kern_make_runnable(((tcb_t*)tmp->value)->tid);
  }
  mp->owner = -1;
  mp->state = MUTEX_UNLOCKED;
}
