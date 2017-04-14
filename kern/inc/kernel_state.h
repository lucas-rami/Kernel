/** @file kernel_state.h
 *  @brief This file contains the declaration for the kernel_state_t data
 *  structure and the functions to act on it.
 *  @author akanjani, lramire1
 */

#ifndef _KERNEL_STATE_H_
#define _KERNEL_STATE_H_

#include <hash_table.h>
#include <mutex.h>
#include <generic_node.h>
#include <tcb.h>
#include <pcb.h>

#define KERNEL_INIT_FALSE 0
#define KERNEL_INIT_TRUE 1
#define CPU_IDLE_FALSE 0
#define CPU_IDLE_TRUE 1

#define FIRST_TASK "print_basic"

typedef struct kernel {

  /** @brief Indicate whether the kernel state is initialized or not */
  char init;

  /* @brief Holds the TCB of the thread currently running,
   * only works for kernel running on uniprocessor */
  tcb_t *current_thread;

  /* @brief Hold the task id that should be assigned to the next
   * task created, the value is incremented each time a task is created */
  int task_id;

  /* @brief Hold the thread id that should be assigned to the next
   * thread created, the value is incremented each time a thread is created */
  int thread_id;

  /* @brief HEAD and TAIL pointers for the queue of runnable threads
   * TODO: should we move these fields in scheduler.c (and make them static) ? 
   * We don't use them elsewhere... */
  generic_node_t *runnable_head, *runnable_tail;

  /* @brief Idle thread (ran when there is nothing to run) */
  tcb_t *idle_thread;

  /* @brief Indicates wether the CPU is currently running the idle thread */
  int cpu_idle;

  /** @brief Mutex used to ensure atomicity when changing the kernel state */
  mutex_t mutex;

  /** @brief Count of the number of frames available(not used or requested for) */
  unsigned int free_frame_count;

  /* @brief INIT's page table base register value */
  // TODO: Initialize this whe init is loaded
  uint32_t init_cr3;

  /* @brief INIT's pcb */
  pcb_t *init_task;

  /* ------------------------- */

  /** @brief Hash table holding all the PCBs */
  generic_hash_table_t pcbs;

  /** @brief Hash table holding all the TCBs */
  generic_hash_table_t tcbs;

} kernel_t;

/* @brief Hold information about an allocation made using new_pages() */
typedef struct alloc {

  /* @brief The base address for the allocation*/
  void* base;

  /* @brief The length for the allocation (in number of pages) */
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

#endif /* _KERNEL_STATE_H_ */
