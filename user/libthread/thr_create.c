/** @file thr_create.c
 *  @brief This file contains the definition for the thr_create() function
 *  @author akanjani, lramire1
 */

#include <global_state.h>
#include <hash_table.h>
#include <page_fault_handler.h>
#include <stdlib.h>
#include <syscall.h>
#include <thr_internals.h>
#include <thread.h>
#include <cond.h>
#include <simics.h>

/** @brief A macro to consider 1 as true
 */
#define TRUE 1

/** @brief A macro to consider 0 as false
 */
#define FALSE 0

/** @brief Create a new thread to run func(arg)
 *
 *  This function allocates a stack for the new thread and
 *  then invoke the thread_fork system call in an appropriate way.
 *  The function also create a TCB for the child thread, and put it in the
 *  TCBs hash table.
 *
 *  @param func The function to be ran by the new thread
 *  @param arg The argument to the function
 *
 *  @return The ID of the child thread on success. A negative number on error.
 */
int thr_create(void *(*func)(void *), void *arg) {

  // Check validity of arguments
  if (func == NULL) {
    return -1;
  }

  // Boundaries of child's stack
  unsigned int *child_stack_low, *child_stack_high;

  // Create new TCB for child thread
  tcb_t *tcb = malloc(sizeof(tcb_t));
  if (tcb == NULL) {
    return -1;
  }

  // Initialize the TCB's mutex and  condition variable
  if (cond_init(&tcb->cond_var_state) < 0 ||
      mutex_init(&tcb->mutex_state) < 0 ||
      cond_init(&tcb->cond_var_kernel_tid) < 0 ||
      mutex_init(&tcb->mutex_kernel_tid) < 0) {
    free(tcb);
    return -1;
  }

  tcb->return_status = NULL;
  tcb->kernel_tid = -1;
  tcb->thread_state = RUNNING;

  // Try to find space for a new stack in the queue
  child_stack_high = queue_delete_node(&task.stack_queue);

  int allocated = TRUE;
  // Define child_stack_low/high and update global state if necessary
  mutex_lock(&task.state_lock);

  if (child_stack_high == NULL) {
    allocated = FALSE;
    child_stack_high = (unsigned int *)((unsigned int) task.stack_lowest - 
                       PAGE_SIZE);

    // Need space for : 1 stack + 1 page (exception stack) + 1 page (guardpage)
    task.stack_lowest = (unsigned int *)((unsigned int)task.stack_lowest -
                        (task.stack_size + 2 * PAGE_SIZE));
  }

  child_stack_low = (unsigned int *)((unsigned int)child_stack_high -
                    task.stack_size - PAGE_SIZE);

  // Give the child thread a library tid
  tcb->library_tid = task.tid;
  ++task.tid;

  mutex_unlock(&task.state_lock);

  // Keep track of stack boundaries in child's TCB
  tcb->stack_low = child_stack_low;
  tcb->stack_high = child_stack_high;

  // Allocate the stack for the child
  if (allocated == FALSE && new_pages(child_stack_low, 
      (unsigned int)child_stack_high - (unsigned int)child_stack_low) < 0) {

    // If we could not allocate stack space, put them in the queue
    queue_insert_node(&task.stack_queue, child_stack_high);

    // Free child's TCB
    free(tcb);

    return -1;
  }

  // Put the child's TCB in the hash table
  hash_table_add_element(&task.tcbs, tcb);

  // Initialize the child's stack (at lower addresses than the exception stack)
  unsigned int *child_esp =
      (unsigned int *)((unsigned int)child_stack_high - PAGE_SIZE) - 1;

  *child_esp = (unsigned int)tcb; // Address of child's TCB
  --child_esp;

  *child_esp = (unsigned int)child_stack_high; // Address of exception stack
  --child_esp;

  *child_esp = (unsigned int)arg;
  --child_esp;

  *child_esp = (unsigned int)func;
  --child_esp;

  // Empty space for "virtual" stub's calling function
  --child_esp;
  *child_esp = (unsigned int)stub; // Address of stub function

  // Create the child thread with thread_fork()
  int child_tid;
  if ((child_tid = thread_fork(child_esp)) < 0) {

    // De-allocate stack space
    remove_pages(child_stack_high);

    // Put stack space in the queue
    queue_insert_node(&task.stack_queue, child_stack_high);

    // Free child's TCB and remove it from hash table
    hash_table_remove_element(&task.tcbs, tcb);

    free(tcb);

    return -1;
  }

  // Update the child's TCB with its kernel issued TID
  mutex_lock(&tcb->mutex_kernel_tid);

  // If the value has not been updated by some other thread, signal that
  // we have updated it to everyone waiting
  if (tcb->kernel_tid == -1) {
    tcb->kernel_tid = child_tid;
    cond_broadcast(&tcb->cond_var_kernel_tid);
  }
  mutex_unlock(&tcb->mutex_kernel_tid);

  return tcb->library_tid;
}

/** @brief Stub function for newly created thread
 *
 *  This function serves as a safeguard against threads returning from
 *  their body function instead of calling thr_exit(). It also allows
 *  us to register an exception handler for the newly created thread.
 *
 *  @param func The function to be ran by the new thread
 *  @param arg The argument to the function
 *  @param addr_exception_stack The address of the child thread's exception
 *   handler space
 *
 *  @return Do not return
 */
void stub(void *(*func)(void *), void *arg, void *addr_exception_stack) {

  swexn(addr_exception_stack, multithread_handler, NULL, NULL);

  void *ret = func(arg);
  thr_exit(ret);
}
