/** @file kernel_state.h
 *  @brief This file contains the declaration for the kernel_state_t data
 *  structure and the functions to act on it.
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

#define KERNEL_INIT_FALSE 0
#define KERNEL_INIT_TRUE 1
#define CPU_IDLE_FALSE 0
#define CPU_IDLE_TRUE 1
#define KERNEL_READY_FALSE 0
#define KERNEL_READY_TRUE 1

#define FIRST_TASK "init"

typedef struct kernel {

  /** @brief Indicate whether the kernel state is initialized or not */
  char init;

  /** @brief Holds the TCB of the thread currently running,
   * only works for kernel running on uniprocessor */
  tcb_t *current_thread;

  /** @brief Hold the task id that should be assigned to the next
   * task created, the value is incremented each time a task is created */
  int task_id;

  /** @brief Hold the thread id that should be assigned to the next
   * thread created, the value is incremented each time a thread is created */
  int thread_id;

  /** @brief Queue of runnable threads */
  stack_queue_t runnable_queue;

  /** @brief Idle thread (ran when there is nothing to run) */
  tcb_t *idle_thread;

  /** @brief Indicates wether the CPU is currently running the idle thread */
  int cpu_idle;

  /** @brief Mutex used to ensure atomicity when changing the kernel state */
  eff_mutex_t mutex;

  /** @brief Mutex used to ensure atomicity when calling malloc */
  eff_mutex_t malloc_mutex;

  /** @brief Count of the number of frames available(not used or requested for) */
  unsigned int free_frame_count;

  /** @brief The address of the zeroed out frame for the whole kernel for 
   * ZFOD */
  unsigned int zeroed_out_frame;

  /** @brief INIT's page table base register value */
  // TODO: Initialize this whe init is loaded
  uint32_t init_cr3;

  /** @brief INIT's pcb */
  pcb_t *init_task;

  uint32_t kernel_ready;

  /* ------------------------- */

  /** @brief Hash table holding all the PCBs */
  generic_hash_table_t pcbs;

  /** @brief Hash table holding all the TCBs */
  generic_hash_table_t tcbs;

} kernel_t;

/* @brief Hold information about an allocation made using new_pages() */
typedef struct alloc {

  /** @brief The base address for the allocation*/
  void* base;

  /** @brief The length for the allocation (in number of pages) */
  int len;

} alloc_t;


/* Hold the kernel state*/
kernel_t kernel;

int kernel_init();
pcb_t *create_new_pcb();
tcb_t *create_new_tcb(pcb_t *pcb, uint32_t esp0, uint32_t cr3);

tcb_t* create_idle_thread();

unsigned int hash_function_pcb(void *pcb, unsigned int nb_buckets);
int find_pcb(void *pcb1, void *pcb2);
unsigned int hash_function_tcb(void *tcb, unsigned int nb_buckets);
int find_tcb(void *tcb1, void *tcb2);
int find_alloc(void* alloc, void* base);
int find_pcb_ll(void* pcb1, void* pcb2);

#endif /* _KERNEL_STATE_H_ */
