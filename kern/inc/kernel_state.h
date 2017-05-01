/** @file kernel_state.h
 *  @brief  This file contains the declarations for the kerne internal data
 *          structures, as welle as functions to act on these
 *  @author akanjani, lramire1
 */

#ifndef _KERNEL_STATE_H_
#define _KERNEL_STATE_H_

#include <hash_table.h>
#include <eff_mutex.h>
#include <generic_node.h>
#include <tcb.h>
#include <pcb.h>
#include <stack_queue.h>
#include <syscalls.h>

/* Boolean values for fields related to the kernel state*/
#define KERNEL_INIT_FALSE 0
#define KERNEL_INIT_TRUE 1
#define CPU_IDLE_FALSE 0
#define CPU_IDLE_TRUE 1
#define KERNEL_READY_FALSE 0
#define KERNEL_READY_TRUE 1
#define ROOT_THREAD_FALSE 0
#define ROOT_THREAD_TRUE 1

/* Exit status in case of exception */
#define EXCEPTION_EXIT_STATUS -2

/* Name of the first task to run */
#define FIRST_TASK "init"

/** @brief  Holds information about an outstanding call to readline() */
typedef struct {
  
  /** @brief  The user character buffer */
  char *buf;
  
  /** @brief  The buffer size, in bytes */  
  int len;

  /** @brief  Readline caller's TCB */
  tcb_t *caller;

  /** @brief  Buffer storing the list of characters printed on the console */
  char key_buf[CONSOLE_IO_MAX_LEN];

  /** @brief  Index in the merged buffer indicating where to write next */
  int key_index;

} readline_t;

typedef struct {

  /** @brief  The mutex to protect the queue */
  eff_mutex_t mp;

  /** @brief  The queue of memory to be freed */
  stack_queue_t zombie_memory;

} garbage_collector_t;

/** @brief  Data structure holding all the kernel's internal data structures
  *         and current state */
typedef struct kernel {

  /** @brief  Indicate whether the kernel state is initialized or not */
  char init;

  /** @brief  Holds the TCB of the thread currently running,
   *          only works for kernel running on uniprocessor */
  tcb_t *current_thread;

  /** @brief  Hold the task id that should be assigned to the next
   *          task created, the value is incremented each time a task is created */
  int task_id;

  /** @brief  Hold the thread id that should be assigned to the next
   *          thread created, the value is incremented each time a thread is created */
  int thread_id;

  /** @brief  Queue of runnable threads */
  stack_queue_t runnable_queue;

  /** @brief  Idle thread (ran when there is nothing to run) */
  tcb_t *idle_thread;

  /** @brief  Keyboard consumer thread (to handler keyboard input) */
  tcb_t *keyboard_consumer_thread;

  /** @brief  Indicates wether the CPU is currently running the idle thread */
  int cpu_idle;

  /** @brief  Mutex used to ensure atomicity when changing the kernel state */
  eff_mutex_t mutex;

  /** @brief  Mutex used to ensure atomicity when calling the malloc library */
  eff_mutex_t malloc_mutex;

  /** @brief  Mutex used to ensure atomicity when printing to the console */
  eff_mutex_t console_mutex;

  /** @brief  Mutex used by readline() to ensure that threads do not 
   *          interleave */
  eff_mutex_t print_mutex;

  /** @brief  Mutex used by readline() to ensure that threads do not 
   *          interleave */
  eff_mutex_t readline_mutex;

  /** @brief  Count of the number of frames available
   *          (not used or requested for) */
  unsigned int free_frame_count;

  /** @brief  The address of the zeroed out frame for the whole kernel for 
   *          ZFOD */
  unsigned int zeroed_out_frame;

  /** @brief  INIT's page table base register value */
  uint32_t init_cr3;

  /** @brief  INIT's pcb */
  pcb_t *init_task;

  /** @brief  A flag that indicates if the kernel is ready for context 
   *          switching */
  uint32_t kernel_ready;

  /** @brief  A garbage collector that frees any kernel memory that was being 
   *          used by zombie/dead threads */
  garbage_collector_t gc;

  /** @brief  A structure holding information about the current outstanding
   *          call to readline() */
  readline_t rl;

  /* ------------------------- */

  /** @brief Hash table holding all the PCBs */
  generic_hash_table_t pcbs;

  /** @brief Hash table holding all the TCBs */
  generic_hash_table_t tcbs;

} kernel_t;

/** @brief  Holds information about an allocation made using new_pages() */
typedef struct {

  /** @brief The base address for the allocation*/
  void* base;

  /** @brief The length for the allocation (in number of pages) */
  int len;

} alloc_t;

/* Holds the kernel state*/
kernel_t kernel;

int kernel_init();
pcb_t *create_new_pcb();
tcb_t *create_new_tcb(pcb_t *pcb, uint32_t esp0, uint32_t cr3,
                      swexn_struct_t* handler, int root_thread);

/* Frames management */
int reserve_frames(unsigned int nb);
void release_frames(unsigned int nb);


/* Functions used by the kernel's internal data strutures */
unsigned int hash_function_pcb(void *pcb, unsigned int nb_buckets);
int find_pcb(void *pcb1, void *pcb2);
unsigned int hash_function_tcb(void *tcb, unsigned int nb_buckets);
int find_tcb(void *tcb1, void *tcb2);
int find_alloc(void* alloc, void* base);
int find_pcb_ll(void* pcb1, void* pcb2);

void keyboard_consumer();

#endif /* _KERNEL_STATE_H_ */
