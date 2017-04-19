/** @file pcb.h
 *  @brief This file contains the declaration for the pcb_t data structure.
 *  @author akanjani, lramire1
 */

#ifndef _PCB_H_
#define _PCB_H_

#include <eff_mutex.h>
#include <stdint.h>
#include <linked_list.h>
#include <dynamic_queue.h>

#define TASK_RUNNING 0
#define TASK_ZOMBIE 1

typedef struct pcb {

  /* @brief The task's kernel issued id */
  int tid;

  /* @brief The task's parent's kernel issued id */
  struct pcb *parent;

  /* @brief The task's return status */
  int return_status;

  /* @brief Task's current state: may be one of [RUNNING, ZOMBIE] */
  int task_state;

  /* @brief Task's page table base register value */
  uint32_t cr3;

  /* @brief Task's original thread's tid */
  int original_thread_id;

  /* @brief Holds the number of frames requested by this process(includes 
   *  the ones that have not been allocated yet as well */
  uint32_t num_of_frames_requested;

  /* @brief Number of threads associated with this task */
  uint32_t num_of_threads;

  /* @brief Number of threads associated with this task */
  uint32_t num_waiting_threads;

  /* @brief Number of threads associated with this task */
  uint32_t num_running_children;

  /* @brief List of allocations made using new_pages() */
  generic_linked_list_t allocations;

  /* @brief Queue of running children */
  generic_linked_list_t running_children;

  /* @brief Queue of zombie children */
  generic_queue_t zombie_children;

  /* @brief Queue of waiting threads */
  generic_queue_t waiting_threads;

  /** @brief Mutex used to ensure atomicity when changing the above three queues */
  eff_mutex_t list_mutex;

  /** @brief Mutex used to ensure atomicity when changing the task state */
  eff_mutex_t mutex;

} pcb_t;

#endif /* _PCB_H_ */
