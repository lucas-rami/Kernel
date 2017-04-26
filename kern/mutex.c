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


/** @brief A state of the mutex which means that mutex_init hasn't been called
 *   after a mutex_destroy
 */
#define MUTEX_UNINITIALIZED 0

/** @brief A state of the mutex which means that mutex_destroy hasn't been called
 *   after a mutex_init
 */
#define MUTEX_INITIALIZED 1

/** @brief Initialize a mutex
 *
 *  This function initializes the mutex pointed to by mp.
 *  The effects of using a mutex before it has been initialized,
 *  or of initializing it when it is already initialized and in use
 *  are undefined.
 *
 *  @param mp The mutex to initialize
 *
 *  @return Zero on success, a negative number on error
 */
int mutex_init(mutex_t *mp) {

  if (!mp) {
    // Invalid parameter
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

/** @brief Deactivate a mutex
 *
 *  This function deactivates the mutex pointed to by mp.
 *  It is illegal for an application to use a mutex after it has been destroyed.
 *  It is illegal for an application to attempt to destroy a mutex while it is
 *  locked or threads are trying to acquire it.
 *
 *  @param mp The mutex to deactivate
 *
 *  @return void
 */
void mutex_destroy(mutex_t *mp) {

  // Invalid parameter
  assert(mp);

  // Illegal Operation. Destroy on an uninitalized mutex
  assert(mp->init == MUTEX_INITIALIZED);

  int j = 1;

  // Generate a new ticket for this thread
  int my_ticket = atomic_add_and_update(&mp->next_ticket, j);

  // Ensure that no other thread is locked or trying to lock this mutex
  assert((mp->prev + 1) == my_ticket);

  // Reset the mutex state
  mp->init = MUTEX_UNINITIALIZED;
}

/** @brief Acquire the lock on a mutex
 *
 *  A call to this function ensures mutual exclusion in the
 *  region between itself and a call to mutex unlock().
 *
 *  @param mp The mutex holding the lock we want to acquire
 *
 *  @return void
 */
void mutex_lock(mutex_t *mp) {

  if (mp->init != MUTEX_INITIALIZED) {
    lprintf("Mutex not init. %p", mp);
  }
  // Validate parameter and the fact that the mutex is initialized
  assert(mp && mp->init == MUTEX_INITIALIZED);

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

/** @brief Gives up the lock on a mutex
 *
 *  The calling thread gives up its claim to the lock. It is illegal
 *  for an application to unlock a mutex that is not locked.
 *
 *  @param mp The mutex to which the lock belongs
 *
 *  @return void
 */
void mutex_unlock(mutex_t *mp) {
  if (mp->init != MUTEX_INITIALIZED) {
    lprintf("Mutex not init. %p", mp);
  }

  // Validate parameter and the fact that the mutex is initialized
  assert(mp && mp->init == MUTEX_INITIALIZED);
  
  // We don't own the mutex anymore, make the tid -1 for yield()
  mp->tid_owner = -1;

  // Increment the prev value which stores the ticket of the last run thread
  mp->prev++;
  
}


int eff_mutex_init(eff_mutex_t *mp) {

  if (!mp) {
    // Invalid parameter
    return -1;
  }

  stack_queue_init(&mp->mutex_queue);

  mp->state = MUTEX_UNLOCKED;
  return 0;
}


void eff_mutex_destroy(eff_mutex_t *mp) {

  // Invalid parameter
  assert(mp);

  disable_interrupts();
  assert(is_stack_queue_empty(&mp->mutex_queue));
  stack_queue_destroy(&mp->mutex_queue);
  enable_interrupts();

}


void eff_mutex_lock(eff_mutex_t *mp) {
  if (kernel.kernel_ready != KERNEL_READY_TRUE) {
    return;
  }

  // lprintf("eff_mutex_lock");
  // MAGIC_BREAK;
  // Validate parameter and the fact that the mutex is initialized
  assert(mp);
  disable_interrupts();
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
  enable_interrupts();
}


void eff_mutex_unlock(eff_mutex_t *mp) {
  if (kernel.kernel_ready != KERNEL_READY_TRUE) {
    return;
  }
  assert(mp);
  // disable_interrupts();
  generic_node_t *tmp = stack_queue_dequeue(&mp->mutex_queue);
  if (tmp) {
    kern_make_runnable(((tcb_t*)tmp->value)->tid);
  }
  mp->state = MUTEX_UNLOCKED;
  // enable_interrupts();
}
