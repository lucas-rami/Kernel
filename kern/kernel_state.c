/** @file kernel_state.h
 *  @brief This file contains the definitions for the kernel_state_t data
 *  structure and the functions to act on it.
 *  @author akanjani, lramire1
 */

#include <assert.h>
#include <kernel_state.h>
#include <stdlib.h>
#include <cond_var.h>

/* Debugging */
#include <simics.h>

#define QUEUE_SIZE 32
#define NB_BUCKETS 8

// Conditional variable for malloc wrappers
extern cond_t cond_malloc;
extern mutex_t mutex_malloc;

/** @brief Initialize the kernel state
 *
 *  @return 0 on success, a negative number on error
 */
int kernel_init() {

  // Set various fields of the state to their initial value
  kernel.current_thread = NULL;
  kernel.task_id = 1337;
  kernel.thread_id = 1337;

  // Initialize the queue for runnable threads
  if (static_queue_init(&kernel.runnable_threads, QUEUE_SIZE) < 0) {
    lprintf("kernel_init(): Failed to initialize runnable threads queue");
    return -1;
  }

  // Initialize the PCBs hash table
  if (hash_table_init(&kernel.pcbs, NB_BUCKETS, find_pcb, hash_function_pcb) <
      0) {
    lprintf("kernel_init(): Failed to initialize hash table for PCBs");
    return -1;
  }

  // Initialize the TCBs hash table
  if (hash_table_init(&kernel.tcbs, NB_BUCKETS, find_tcb, hash_function_tcb) <
      0) {
    lprintf("kernel_init(): Failed to initialize hash table for TCBs");
    return -1;
  }

  if (mutex_init(&kernel.mutex) < 0) {
    lprintf("kernel_init(): Failed to initialize mutex");
    return -1;
  }

  // Initialize the mutex for the functions in malloc_wrappers.c
  if (mutex_init(&mutex_malloc) < 0) {
    lprintf("kernel_init(): Failed to initialize mutex for malloc_wrappers.c");
    return -1;
  }

  // Initialize the condition variable for the functions in malloc_wrappers.c
  if (cond_init(&cond_malloc) < 0) {
    lprintf("kernel_init(): Failed to initialize conditional variable for malloc_wrappers.c");
    return -1;
  }

  // Mark the kernel state as initialized
  kernel.init = KERNEL_INIT_TRUE;

  return 0;
}

// TODO: doc
pcb_t *create_new_pcb() {

  assert(kernel.init == KERNEL_INIT_TRUE);

  // Allocate space for the new PCB
  pcb_t *new_pcb = malloc(sizeof(pcb_t));

  // Initialize the mutex on this PCB
  if (mutex_init(&new_pcb->mutex) < 0) {
    lprintf("create_new_pcb(): Failed to initialize mutex");
    free(new_pcb);
    return NULL;
  }

  // Set various fields to their initial value
  new_pcb->return_status = 0;
  new_pcb->task_state = TASK_RUNNING;

  // Assign a unique id to the PCB
  mutex_lock(&kernel.mutex);
  new_pcb->tid = kernel.task_id;
  if (++kernel.task_id < 0) {
    kernel.task_id = 1;
  }
  mutex_unlock(&kernel.mutex);

  // Add the new PCB to the hash table
  if (hash_table_add_element(&kernel.pcbs, new_pcb) < 0) {
    lprintf("create_new_pcb(): Failed to add new PCB to hash table");
    free(new_pcb);
    return NULL;
  }

  return new_pcb;
}

// TODO: doc
tcb_t *create_new_tcb(const pcb_t *pcb, uint32_t esp0, uint32_t cr3) {

  assert(kernel.init == KERNEL_INIT_TRUE && pcb != NULL);

  // Allocate space for the new PCB
  tcb_t *new_tcb = malloc(sizeof(tcb_t));

  // Initialize the mutex on this PCB
  if (mutex_init(&new_tcb->mutex) < 0) {
    lprintf("create_new_tcb(): Failed to initialize mutex");
    free(new_tcb);
    return NULL;
  }

  // Set various fields to their initial value
  new_tcb->task = pcb;
  new_tcb->thread_state = THR_BLOCKED; // NOTE: not really, just not ready...
  new_tcb->descheduled = THR_DESCHEDULED_FALSE;
  new_tcb->esp = 0; // NOTE: should be modified when the stack is crafted
  new_tcb->esp0 = esp0;
  new_tcb->cr3 = cr3;

  // Assign a unique id to the TCB
  mutex_lock(&kernel.mutex);
  new_tcb->tid = kernel.thread_id;
  if (++kernel.thread_id < 0) {
    kernel.task_id = 1;
  }
  mutex_unlock(&kernel.mutex);

  // Add the new PCB to the hash table
  if (hash_table_add_element(&kernel.tcbs, new_tcb) < 0) {
    lprintf("create_new_tcb(): Failed to add new TCB to hash table");
    free(new_tcb);
    return NULL;
  }

  return new_tcb;
}

/** @brief Hash function for PCBs hash table
 *
 *  @param pcb The PCB to hash
 *  @param nb_buckets The number of buckets in the hash table
 *
 *  @return The PCB's hashed value
 */
unsigned int hash_function_pcb(void *pcb, unsigned int nb_buckets) {
  pcb_t *p = pcb;
  return p->tid % nb_buckets;
}

/** @brief Find a PCB by its tid
 *
 *  @param pcb A thread's PCB
 *  @param tid A process tid
 *
 *  @return 1 if the PCB is the good one (its tid is tid), 0 otherwise
 */
int find_pcb(void *pcb, void *tid) {
  pcb_t *p = pcb;
  int *id = tid;

  if (p->tid == *id) {
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
  return t->tid % nb_buckets;
}

/** @brief Find a TCB by its tid
 *
 *  @param tcb A thread's TCB
 *  @param tid A thread tid
 *
 *  @return 1 if the TCB is the good one (its tid is tid), 0 otherwise
 */
int find_tcb(void *tcb, void *tid) {
  tcb_t *t = tcb;
  int *id = tid;

  if (t->tid == *id) {
    return 1;
  }
  return 0;
}
