/** @file wait.c
 *  @brief  This file contains the definition for kern_wait() system call and
 *          its helper function
 *  @author akanjani, lramire1
 */

#include <virtual_memory.h>
#include <virtual_memory_defines.h>
#include <stdint.h>
#include <simics.h>
#include <stddef.h>
#include <pcb.h>
#include <kernel_state.h>
#include <eff_mutex.h>
#include <syscalls.h>
#include <malloc.h>
#include <stack_queue.h>
#include <scheduler.h>

/** @brief Collects the exit status of a task and stores it in the integer 
 *         referenced by status ptr.
 *
 *   The wait() system call may be invoked simultaneously by any number of 
 *   threads in a task; exited child tasks may be matched to wait()’ing 
 *   threads in any non-pathological way. Threads
 *   which cannot collect an already-exited child task when there exist child
 *   tasks which have not yet exited will generally block until a child task 
 *   exits and collect the status of an exited child
 *   task. However, threads which will definitely not be able to collect the 
 *   status of an exited child task in the future must not block forever; 
 *   in that case, wait() will return an integer error code less than zero.
 *   The invoking thread may specify a status ptr parameter of zero (NULL) 
 *   to indicate that it wishes to collect the ID of an exited task but 
 *   wishes to ignore the exit status of that task. Otherwise, if the status 
 *   ptr parameter does not refer to writable memory, wait() will return
 *   an integer error code less than zero instead of collecting a child task.
 *
 *  @param  status_ptr   The pointer to the integer where the status of the 
 *                       exited task should be stored 
 *
 *  @return  int  Thread ID of the original thread of the exiting task on 
 *                success, -1 otherwise
 *
 */
int kern_wait(int *status_ptr) {
  
  // Argument validation
  if (status_ptr != NULL &&
      is_buffer_valid((unsigned int)status_ptr, sizeof(int), READ_WRITE) < 0) {
    lprintf("The address status_ptr isn't valid");
    return -1;
  }
  
  pcb_t *curr_task = kernel.current_thread->task;
  eff_mutex_lock(&curr_task->list_mutex);

  // Check if this thread will wait infinitely 
  if (curr_task->num_waiting_threads >= curr_task->num_running_children) {
    // This thread will wait infinitely
    eff_mutex_unlock(&curr_task->list_mutex);
    return -1;
  }
  
  if (curr_task->zombie_children.head != NULL) {
    
    // Remove the zombie child
    generic_node_t * zombie_child_node = 
      stack_queue_dequeue(&curr_task->zombie_children);
    if (zombie_child_node == NULL) {
      lprintf("kern_wait(): zombie queue delete failed");
      return -1;
    }

    pcb_t* zombie_child = zombie_child_node->value;

    // Decrease the count of running or zombie children for this task
    curr_task->num_running_children--;
    eff_mutex_unlock(&curr_task->list_mutex);

    if (status_ptr != NULL) {
      // Set the status_ptr if it isn't NULL
      *status_ptr = zombie_child->return_status;
    }

    // Set the return value as original thread id of the zombie child
    int ret = zombie_child->original_thread_id;

    // Cleanup the zombie
    cleanup_process(zombie_child);
    
    return ret;
  }

  // No zombie children at the time
  // Wait for at least one thread to vanish
  curr_task->num_waiting_threads++;

  // Enqueue myself in the the queue of waiting threads
  generic_node_t new_waiting = {kernel.current_thread, NULL};
  stack_queue_enqueue(&curr_task->waiting_threads, &new_waiting);
  
  // Block this thread
  block_and_switch(HOLDING_MUTEX_TRUE, &curr_task->list_mutex);
  
  if (status_ptr != NULL) {
    // Set the status ptr if not NULL
    *status_ptr = kernel.current_thread->reaped_task->return_status;
  }

  // Set the return value as the original thread id of the zombie task
  int ret = kernel.current_thread->reaped_task->original_thread_id;

  // Cleanup the exited process
  cleanup_process(kernel.current_thread->reaped_task);

  return ret;
}

/** @brief  Cleans up the kernel stack and the pcb of the exited thread
 *
 *  @param  task  A pointer to the pcb of the exited thread
 *
 *  @return  void
 *
 */
void cleanup_process(pcb_t *task) {
  // Remove element from the hash table
  hash_table_remove_element(&kernel.pcbs, task);

  // Free everything in the garbage collector queue
  eff_mutex_lock(&kernel.gc.mp);
  generic_node_t *delete_zombie_mem;
  while((delete_zombie_mem = stack_queue_dequeue(&kernel.gc.zombie_memory)) 
        != NULL) {
    free((tcb_t*)delete_zombie_mem->value);
  }
  eff_mutex_unlock(&kernel.gc.mp);

  // Extract the stack pointer start addr from the exited task
  char *delete_me = (char*)task->last_thread_esp0;

  // Free the pcb and the kernel stack
  free(task);
  free(delete_me);
 }
