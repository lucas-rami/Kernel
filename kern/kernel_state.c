/** @file kernel_state.h
 *  @brief This file contains the definitions for the kernel_state_t data
 *  structure and the functions to act on it.
 *  @author akanjani, lramire1
 */

#include <assert.h>
#include <cond_var.h>
#include <kernel_state.h>
#include <stdlib.h>
#include <page.h>
#include <asm.h>
#include <common_kern.h>
#include <virtual_memory_defines.h>
#include <syscalls.h>
#include <cr.h>
#include <stack_queue.h>

/* Debugging */
#include <simics.h>

#define QUEUE_SIZE 32
#define NB_BUCKETS 8

static void idle() {
  lprintf("\tidle(): Idle task running");
  enable_interrupts();
  while (1) {
    continue;
  }
}

// Conditional variable for malloc wrappers
extern cond_t cond_malloc;
extern mutex_t mutex_malloc;

/** @brief Initialize the kernel state
 *
 *  @return 0 on success, a negative number on error
 */
int kernel_init() {

  // Set various fields of the state to their initial value
  kernel.kernel_ready = KERNEL_READY_FALSE;
  kernel.current_thread = NULL;
  kernel.task_id = 1;
  kernel.thread_id = 1;
  kernel.cpu_idle = CPU_IDLE_TRUE;
  kernel.free_frame_count = machine_phys_frames() - NUM_KERNEL_FRAMES;
  kernel.zeroed_out_frame = 0;

  // Initialize the runnable thread queue
  stack_queue_init(&kernel.runnable_queue);

  // Initialize the mutex for the functions in malloc_wrappers.c
  if (eff_mutex_init(&kernel.malloc_mutex) < 0) {
    lprintf("kernel_init(): Failed to initialize mutex for malloc_wrappers.c");
    return -1;
  }
  lprintf("Kernel malloc mutex is %p", &kernel.malloc_mutex);

  // Initialize the condition variable for the functions in malloc_wrappers.c
  /*if (cond_init(&cond_malloc) < 0) {
    lprintf("kernel_init(): Failed to initialize conditional variable for "
            "malloc_wrappers.c");
    return -1;
  }*/

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

  // Initialize the kernel mutex
  if (eff_mutex_init(&kernel.mutex) < 0) {
    lprintf("kernel_init(): Failed to initialize mutex");
    return -1;
  }
  lprintf("Kernel mutex is %p", &kernel.mutex);

  // Configure the idle task
  kernel.idle_thread = create_idle_thread();
  if (kernel.idle_thread == NULL) {
    lprintf("kernel_init(): Failed to create idle thread");
  }
  kernel.current_thread = kernel.idle_thread;

  // Mark the kernel state as initialized
  kernel.init = KERNEL_INIT_TRUE;

  return 0;
}

tcb_t *create_idle_thread() {

  // Allocate space for the new PCB
  pcb_t *new_pcb = malloc(sizeof(pcb_t));

  // Initialize the mutex on this PCB
  if (eff_mutex_init(&new_pcb->mutex) < 0) {
    lprintf("create_idle_thread(): Failed to initialize PCB mutex");
    free(new_pcb);
    return NULL;
  }

  // Set various fields to their initial value
  new_pcb->return_status = 0;
  new_pcb->task_state = TASK_RUNNING;
  new_pcb->tid = 0; // Okay since no other task is allowed to have this tid

  // Allocate space for the new PCB
  tcb_t *new_tcb = malloc(sizeof(tcb_t));

  // Initialize the mutex on this PCB
  if (eff_mutex_init(&new_tcb->mutex) < 0) {
    lprintf("create_idle_thread(): Failed to initialize TCB mutex");
    free(new_pcb);
    free(new_tcb);
    return NULL;
  }

  // Set various fields to their initial value
  new_tcb->task = new_pcb;
  new_tcb->thread_state = THR_RUNNING;
  new_pcb->num_of_frames_requested = 0;  
  new_tcb->tid = 0; // Fine since no other thread is allowed to have this tid

  // Allocate a kernel stack for the root thread
  void *stack_kernel = malloc(PAGE_SIZE);
  if (stack_kernel == NULL) {
    lprintf("create_idle_thread(): Failed to allocate kernel stack");
    return NULL;
  }

  new_tcb->esp0 = (uint32_t)(stack_kernel) + PAGE_SIZE;
  unsigned int * stack_addr = (unsigned int *) new_tcb->esp0;
  --stack_addr;
  *stack_addr = (unsigned int) idle;
  new_tcb->esp = (unsigned int) stack_addr;
  new_tcb->cr3 = get_cr3();

  return new_tcb;

}

// TODO: doc
pcb_t *create_new_pcb() {

  assert(kernel.init == KERNEL_INIT_TRUE);

  // Allocate space for the new PCB
  pcb_t *new_pcb = malloc(sizeof(pcb_t));

  // Initialize the mutex on this PCB
  if (eff_mutex_init(&new_pcb->mutex) < 0) {
    lprintf("create_new_pcb(): Failed to initialize mutex");
    free(new_pcb);
    return NULL;
  }

  // Initialize the list_mutex on this PCB
  if (eff_mutex_init(&new_pcb->list_mutex) < 0) {
    lprintf("create_new_pcb(): Failed to initialize list_mutex");
    free(new_pcb);
    return NULL;
  }

  // Initialize the allocations list
  if (linked_list_init(&new_pcb->allocations, find_alloc) < 0) {
    lprintf("create_new_pcb(): Failed to initialize linked list");
    free(new_pcb);
    return NULL;
  }

  // Initialize the running children queue
  if (linked_list_init(&new_pcb->running_children, find_pcb_ll) < 0) {
    lprintf("create_new_pcb(): Failed to initialize running_children");
    free(new_pcb);
    return NULL;
  }

  // Initialize the zombie children and waiting children queues
  stack_queue_init(&new_pcb->zombie_children);
  stack_queue_init(&new_pcb->waiting_threads);

  // Set various fields to their initial value
  new_pcb->return_status = 0;
  new_pcb->task_state = TASK_RUNNING;
  new_pcb->num_of_frames_requested = 0;
  new_pcb->num_of_threads = 1;
  new_pcb->parent = NULL;
  new_pcb->original_thread_id = 0;
  new_pcb->num_running_children = 0;
  new_pcb->num_waiting_threads = 0;

  // Assign a unique id to the PCB
  eff_mutex_lock(&kernel.mutex);
  new_pcb->tid = kernel.task_id;
  if (++kernel.task_id < 0) {
    kernel.task_id = 1;
  }
  eff_mutex_unlock(&kernel.mutex);

  // Add the new PCB to the hash table
  if (hash_table_add_element(&kernel.pcbs, new_pcb) < 0) {
    lprintf("create_new_pcb(): Failed to add new PCB to hash table");
    free(new_pcb);
    return NULL;
  }

  return new_pcb;
}

// TODO: doc
tcb_t *create_new_tcb(pcb_t *pcb, uint32_t esp0, uint32_t cr3) {

  assert(kernel.init == KERNEL_INIT_TRUE && pcb != NULL);

  // Allocate space for the new PCB
  tcb_t *new_tcb = malloc(sizeof(tcb_t));

  // Initialize the mutex on this PCB
  if (eff_mutex_init(&new_tcb->mutex) < 0) {
    lprintf("create_new_tcb(): Failed to initialize mutex");
    free(new_tcb);
    return NULL;
  }

  // Set various fields to their initial value
  new_tcb->task = pcb;
  new_tcb->thread_state = THR_BLOCKED; // NOTE: not really, just not ready...
  new_tcb->esp = 0; // NOTE: should be modified when the stack is crafted
  new_tcb->esp0 = esp0;
  new_tcb->cr3 = cr3;
  new_tcb->num_of_frames_requested = 0;
  new_tcb->swexn_values.esp3 = NULL;
  new_tcb->swexn_values.eip = NULL;
  new_tcb->swexn_values.arg = NULL;

  // TODO: Change this into an atomic op.
  // No need for a mutex
  // Assign a unique id to the TCB
  eff_mutex_lock(&kernel.mutex);
  new_tcb->tid = kernel.thread_id;
  if (++kernel.thread_id < 0) {
    kernel.thread_id = 1;
  }
  eff_mutex_unlock(&kernel.mutex);

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
int find_pcb(void *pcb1, void *pcb2) {

  if (((pcb_t*)pcb1)->tid == ((pcb_t*)pcb2)->tid) {
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
int find_tcb(void *tcb1, void *tcb2) {
  
  if (((tcb_t*)tcb1)->tid == ((tcb_t*)tcb2)->tid) {
    return 1;
  }
  
  return 0;
}

/** @brief Find an allocation made by new_pages() by its base address
 *
 *  @param alloc An allocation made by new_pages()
 *  @param base  A base address
 *
 *  @reutrn 1 if the allocation is the good one (the base addresses match), 
 *    0 otherwise
 */
int find_alloc(void* alloc, void* base) {
  alloc_t * allocation = alloc;
  if (base == allocation->base) {
    return 1;
  }
  return 0;
}

int find_pcb_ll(void *pcb1, void *pcb2) {
  return (uint32_t)pcb1 == (uint32_t)pcb2;
}
