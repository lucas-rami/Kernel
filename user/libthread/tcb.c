/** @file tcb.c
 *  @brief This file contains the definition for functions related to TCB
 * manipulation
 *  @author akanjani, lramire1
 */

#include <global_state.h>
#include <stdlib.h>
#include <thr_internals.h>

/** @brief Get the current thread's TCB
 *
 *  The pointer to a thread's TCB is stored by the parent
 *  thread at the highest address of the child thread's stack.
 *  This is done even before the child thread is created, which
 *  guarantees that the address is there when we get try to get it.
 *
 *  @return Current thread's TCB
 */
tcb_t *get_tcb() {

  // Get our esp register's value
  unsigned int esp = get_esp();

  if (esp >= (unsigned int)task.stack_highest_childs) {
    // The root thread is calling the function
    return task.root_tcb;

  } else {
    unsigned int size = task.stack_size + 2 * PAGE_SIZE;
    unsigned int thr_stack =
        ((unsigned int)task.stack_highest_childs - esp) / size;

    tcb_t **tcb = (tcb_t **)((unsigned int)task.stack_highest_childs -
                             thr_stack * size - PAGE_SIZE - 4);
    return *tcb;
  }

}

/** @brief Find a TCB by its library_tid
 *
 *  @param tcb A thread's TCB
 *  @param tid A library tid
 *
 *  @return 1 if the TCB is the good one (its library_tid is tid), 0 otherwise
 */
int find_tcb(void *tcb, void *tid) {
  tcb_t *t = tcb;
  int *library_tid = tid;

  if (t->library_tid == *library_tid) {
    return 1;
  }
  return 0;
}

/** @brief Hash function for TCBs hash table
 *
 *  @param tcb The TCB to hash
 *  @param nb_buckets The number of buckets in the hash table
 *
 *  @return The TCB's hashed value
 */
unsigned int hash_function_tcb(void *tcb, unsigned int nb_buckets) {
  tcb_t *t = tcb;
  return t->library_tid % nb_buckets;
}
