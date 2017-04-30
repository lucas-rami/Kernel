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
#include <string.h>
#include <context_switch.h>
#include <atomic_ops.h>

/* Debugging */
#include <simics.h>

/* Number of buckets for hash tables*/
#define NB_BUCKETS 8

/* Number of registers poped during a popa instruction */
#define NB_REGISTERS_POPA 8

/* Static functions prototypes */
static tcb_t *create_idle_thread();
static tcb_t *create_keyboard_consumer_thread();

/** @brief  Creates the kernel's idle thread
 *
 *  The function creates a particular TCB for the idle thread which does not
 *  have an englobing task and is not added to the TCBs hash table.
 *  Only some fields of the TCB data structure are filled with meaningful data.
 *
 *  @return The idle thread's TCB on success, NULL otherwise
 */
static tcb_t *create_idle_thread() {

  // Allocate space for the new TCB
  tcb_t *new_tcb = malloc(sizeof(tcb_t));
  if (new_tcb == NULL) {
    return NULL;
  }

  // Initialize the mutex on this TCB
  if (eff_mutex_init(&new_tcb->mutex) < 0) {
    lprintf("create_idle_thread(): Failed to initialize TCB mutex");
    free(new_tcb);
    return NULL;
  }

  // Set various fields to their initial value (only the one that matters for
  // the idle thread)
  new_tcb->task = NULL;
  new_tcb->thread_state = THR_RUNNING;
  new_tcb->tid = 0; // No other thread is allowed to have this tid
  new_tcb->esp0 = get_esp0();
  new_tcb->cr3 = get_cr3();

  return new_tcb;
}

/** @brief  Creates the kernel's keyboard consumer thread
 *
 *  The function creates a particular TCB for the idle thread which does not
 *  have an englobing task and is not added to the TCBs hash table.
 *  Only some fields of the TCB data structure are filled with meaningful data.
 *
 *  @return The idle thread's TCB on success, NULL otherwise
 */
static tcb_t *create_keyboard_consumer_thread() {

  // Allocate space for the new TCB
  tcb_t *new_tcb = malloc(sizeof(tcb_t));
  if (new_tcb == NULL) {
    return NULL;
  }

  // Create a kernel stack for this thread
  void* kernel_stack = malloc(PAGE_SIZE);
  if (kernel_stack == NULL) {
    free(new_tcb);
    return NULL;
  }

  // Initialize the mutex on this TCB
  if (eff_mutex_init(&new_tcb->mutex) < 0) {
    lprintf("create_idle_thread(): Failed to initialize TCB mutex");
    free(new_tcb);
    return NULL;
  }

  // Set various fields to their initial value (only the one that matters for
  // the keyboard consumer thread)
  new_tcb->task = NULL;
  new_tcb->thread_state = THR_BLOCKED;
  new_tcb->tid = -1; // No other thread is allowed to have this tid
  new_tcb->esp0 = ((uint32_t)kernel_stack) + PAGE_SIZE;
  new_tcb->cr3 = get_cr3();

  // Craft the stack for first context switch to this thread
  unsigned int * stack_addr = (unsigned int *) new_tcb->esp0;
  --stack_addr;
  *stack_addr = (unsigned int) new_tcb;
  --stack_addr;
  *stack_addr = (unsigned int) keyboard_consumer;
  --stack_addr;  
  *stack_addr = (unsigned int) init_thread;
  stack_addr -= NB_REGISTERS_POPA;

  new_tcb->esp = (uint32_t) stack_addr;

  return new_tcb;

}

/** @brief  Initializes the kernel state
 *
 *  The function initalizes all the data structures needed by the kernel and
 *  creates the idle thread that is run when there is nothing else to run.
 *
 *  The function must be called once before using any field of the kernel_t
 *  data structure. The function should not be called more than once.
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
  
  // Initialize readline_t structure
  kernel.rl.buf = NULL;
  kernel.rl.len = 0;
  kernel.rl.caller = NULL;
  memset(kernel.rl.key_buf, 0, CONSOLE_IO_MAX_LEN);
  kernel.rl.key_index = 0; 

  // Initialize the runnable thread queue
  stack_queue_init(&kernel.runnable_queue);

  // Initialize the mutex for the functions in malloc_wrappers.c
  if (eff_mutex_init(&kernel.malloc_mutex) < 0) {
    lprintf("kernel_init(): Failed to initialize mutex for malloc_wrappers.c");
    return -1;
  }

  // Initialize the mutex for the functions printing to the console
  if (eff_mutex_init(&kernel.console_mutex) < 0) {
    lprintf("kernel_init(): Failed to initialize mutex for the console");
    return -1;
  }

  lprintf("Kernel malloc mutex is %p", &kernel.malloc_mutex);

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

  if (eff_mutex_init(&kernel.gc.mp) < 0) {
    lprintf("kernel_init(): Failed to initialize the garbage collector");
    return -1;
  }

  stack_queue_init(&kernel.gc.zombie_memory);

  // Create the idle thread
  kernel.idle_thread = create_idle_thread();
  if (kernel.idle_thread == NULL) {
    lprintf("kernel_init(): Failed to create idle thread");
    return -1;
  }

  // Create the keyboard consumer thread
  kernel.keyboard_consumer_thread = create_keyboard_consumer_thread();
  if (kernel.keyboard_consumer_thread == NULL) {
    lprintf("kernel_init(): Failed to create keyboard consumer thread");    
    return -1;
  }

  // Set the current thread as being the idle thread
  kernel.current_thread = kernel.idle_thread;

  // Mark the kernel state as initialized
  kernel.init = KERNEL_INIT_TRUE;

  return 0;
}

/** @brief  Creates a new PCB and add it to the PCBs hash table
 *
 *  The number of frames requested is set to 0 and the task's state is set to
 *  TASK_RUNNING. When a PCB is created it has no child thread. The original 
 *  thread id is set to 0, and should be modified when the root thread is
 *  created for this task. The new PCB is added to the hash table of PCBs.
 *
 *  @return A pointer to the newly created PCB on success, NULL otherwise
 */
pcb_t *create_new_pcb() {

  assert(kernel.init == KERNEL_INIT_TRUE);

  // Allocate space for the new PCB
  pcb_t *new_pcb = malloc(sizeof(pcb_t));
  if (new_pcb == NULL) {
    return NULL;
  }

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
  new_pcb->last_thread_esp0 = 0;

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

/** @brief  Creates a new TCB and add it to the TCBs hash table
 *  
 *  The number of frames requested is set to 0 and the thread's state to 
 *  THR_UNINITALIZED. The esp field is set to 0 and should be modified when the 
 *  thread's stack is manually crafted. The new TCB is added to the hash table 
 *  of TCBs. If the handler argument is not NULL, an exception handler is
 *  registered for the new thread.
 *
 *  @param  pcb         The task to which the new TCB will belong to
 *  @param  esp0        The esp0 value for the new thread
 *  @param  cr3         The cr3 value for the new thread
 *  @param  handler     A pointer to a structure representing a user defined
 *                      exception handler
 *  @param  root_thread A value indicating whether the created TCB is for the
 *                      root thread of the task 
 *
 *  @return A pointer to the newly created TCB on success, NULL otherwise
 */
tcb_t *create_new_tcb(pcb_t *pcb, uint32_t esp0, uint32_t cr3, 
                      swexn_struct_t* handler, int root_thread) {

  assert(kernel.init == KERNEL_INIT_TRUE && pcb != NULL);

  // Allocate space for the new PCB
  tcb_t *new_tcb = malloc(sizeof(tcb_t));
  if (new_tcb == NULL) {
    return NULL;
  }

  // Initialize the mutex on this PCB
  if (eff_mutex_init(&new_tcb->mutex) < 0) {
    lprintf("create_new_tcb(): Failed to initialize mutex");
    free(new_tcb);
    return NULL;
  }

  // Set various fields to their initial value
  new_tcb->task = pcb;
  new_tcb->thread_state = THR_UNINITALIZED; 
  new_tcb->esp = 0; // Should be modified when the new thread's stack is created
  new_tcb->esp0 = esp0;
  new_tcb->cr3 = cr3;
  new_tcb->num_of_frames_requested = 0;

  // Register an exception handler for this thread if the handler argument is 
  // not NULL
  if (handler != NULL) {
    new_tcb->swexn_values = *handler;
  } else {
    new_tcb->swexn_values.esp3 = NULL;
    new_tcb->swexn_values.eip = NULL;
    new_tcb->swexn_values.arg = NULL;
  }

  // Assign a unique id to the TCB
  eff_mutex_lock(&kernel.mutex);
  new_tcb->tid = kernel.thread_id;
  if (++kernel.thread_id < 0) {
    kernel.thread_id = 1;
  }
  eff_mutex_unlock(&kernel.mutex);

  // If this is the root thread, update the task original thread id
  if (root_thread == ROOT_THREAD_TRUE) {
    pcb->original_thread_id = new_tcb->tid;
  }

  // Add the new PCB to the hash table
  if (hash_table_add_element(&kernel.tcbs, new_tcb) < 0) {
    lprintf("create_new_tcb(): Failed to add new TCB to hash table");
    free(new_tcb);
    return NULL;
  }

  return new_tcb;
}

/** @brief  Atomically increases the number of free frames available 
 *
 *  @param  nb  The number of frames to release  
 *
 *  @return void
 */
void release_frames(unsigned int nb) {
  atomic_add_and_update(&kernel.free_frame_count, nb);
}

/** @brief  Atomically tries to reserve a given number of frames
 *
 *  @param  nb  The number of frames to reserve  
 *
 *  @return 0 on success, a negative number on failure
 */
int reserve_frames(unsigned int nb) {
  
  int success = 0;

  do {

    int old_val = kernel.free_frame_count; 
    if (old_val < nb) {
      return -1;
    }
    int new_val = old_val - nb;
    success = atomic_compare_and_exchange_32(&kernel.free_frame_count, 
                                              old_val, new_val);

  } while (!success);

  return 0;
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

// TODO
int find_pcb_ll(void *pcb1, void *pcb2) {
  return (uint32_t)pcb1 == (uint32_t)pcb2;
}
