/** @file mutex.c
 *  @brief This file contains the definitions for mutex_type.h functions
 *  @author akanjani, lramire1
 */

#include <mutex.h>
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

/** @brief  Initializes a mutex
 *
 *  This function initializes the mutex pointed to by mp.
 *  The effects of using a mutex before it has been initialized,
 *  or of initializing it when it is already initialized and in use
 *  are undefined.
 *
 *  @param  mp The mutex to initialize
 *
 *  @return 0 on success, a negative number on error
 */
int mutex_init(mutex_t *mp) {

  // Check the argument
  if (mp == NULL) {
    return -1;
  }

  // Initialize the state for the mutex
  mp->prev = 0;
  mp->next_ticket = 1;
  mp->init = MUTEX_INITIALIZED;
  mp->tid_owner = -1;

  lprintf("Init mutex %p", mp);
  return 0;
}

/** @brief  Destroys a mutex
 *
 *  This function destroys the mutex pointed to by mp.
 *  It is illegal for an application to use a mutex after it has been destroyed.
 *  It is illegal for an application to attempt to destroy a mutex while it is
 *  locked or threads are trying to acquire it.
 *
 *  @param  mp The mutex to deactivate
 *
 *  @return void
 */
void mutex_destroy(mutex_t *mp) {

  // Invalid parameter
  assert(mp != NULL && mp->init == MUTEX_INITIALIZED);

  int j = 1;

  // Generate a new ticket for this thread
  int my_ticket = atomic_add_and_update(&mp->next_ticket, j);

  // Ensure that no other thread is locked or trying to lock this mutex
  assert((mp->prev + 1) == my_ticket);

  // Reset the mutex state
  mp->init = MUTEX_UNINITIALIZED;
}

/** @brief  Acquires the lock on a mutex
 *
 *  A call to this function ensures mutual exclusion in the
 *  region between itself and a call to mutex unlock().
 *
 *  @param  mp The mutex holding the lock we want to acquire
 *
 *  @return void
 */
void mutex_lock(mutex_t *mp) {

  if (mp->init != MUTEX_INITIALIZED) {
    lprintf("Mutex not init. %p", mp);
  }

  // Check argument
  assert(mp != NULL && mp->init == MUTEX_INITIALIZED);

  int j = 1;

  // Generate a new ticket for this thread
  int my_ticket = atomic_add_and_update(&mp->next_ticket, j);

  while((mp->prev + 1) != my_ticket) {
    lprintf("Yielding as the owner is %d. Thread %d should run", mp->tid_owner, mp->prev + 1);
    kern_yield(-1);
  }

  // We own the mutex, make owner_tid our tid for yield()
  mp->tid_owner = kern_gettid();

}

/** @brief  ives up the lock on a mutex
 *
 *  The calling thread gives up its claim to the lock. It is illegal
 *  for an application to unlock a mutex that is not locked.
 *
 *  @param  mp The mutex to which the lock belongs
 *
 *  @return void
 */
void mutex_unlock(mutex_t *mp) {
  if (mp->init != MUTEX_INITIALIZED) {
    lprintf("Mutex not init. %p", mp);
  }

  // Check argument
  assert(mp != NULL && mp->init == MUTEX_INITIALIZED);
  
  // We don't own the mutex anymore, make the tid -1 for yield()
  mp->tid_owner = -1;

  // Increment the prev value which stores the ticket of the last run thread
  mp->prev++;
  
}

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
 *  This function should be called only after the eff_mutex has been initialized
 *  previously with a call to eff_mutex_init(). After this call returns, one may
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
 *  descheduled until the mutex is available for it. The invoking thread should
 *  not be holding the mutex before calling this function.
 *
 *  @param  mp  A pointer to an eff_mutex
 *
 *  @return void
 */
void eff_mutex_lock(eff_mutex_t *mp) {
  
  if (kernel.kernel_ready != KERNEL_READY_TRUE) {
    return;
  }

  // lprintf("eff_mutex_lock");
  // MAGIC_BREAK;
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
    lprintf("Blocking current thread %d for mutex %p as the owner is %d", kernel.current_thread->tid, mp, mp->owner);
    stack_queue_enqueue(&mp->mutex_queue, &tmp);
    // This call will enable interrupts
    block_and_switch(HOLDING_MUTEX_FALSE, NULL);
  }
  mp->state = MUTEX_LOCKED;
  mp->owner = kernel.current_thread->tid;
  // lprintf("Thread %d has the mutex %p", kernel.current_thread->tid, mp);
  enable_interrupts();
}

/** @brief  Releases the lock on the mutex
 *
 *  The function wakes up the next thread in the queue (if any) before returning
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
  // disable_interrupts();
  generic_node_t *tmp = stack_queue_dequeue(&mp->mutex_queue);
  if (tmp) {
    kern_make_runnable(((tcb_t*)tmp->value)->tid);
    // if (kernel.current_thread->tid != 1)
    lprintf("Made runnable thread %d for mutex %p", ((tcb_t*)tmp->value)->tid, mp);
  }
  mp->owner = -1;
  mp->state = MUTEX_UNLOCKED;
  // enable_interrupts();
}
