#include <virtual_memory.h>
#include <stdint.h>
#include <simics.h>
#include <stddef.h>
#include <pcb.h>
#include <kernel_state.h>
#include <eff_mutex.h>
#include <dynamic_queue.h>
#include <syscalls.h>
#include <malloc.h>
#include <stack_queue.h>

int kern_wait(int *status_ptr) {
  
  // Validation of args
  if (is_buffer_valid((unsigned int)&status_ptr, sizeof(uintptr_t)) < 0) {
    // status ptr isn't a valid memory address
    lprintf("kern_wait(): Invalid args");
  }

  // If number of running children is less than or equal
  // to number of waiitng threads, return err
  if (status_ptr != NULL && is_buffer_valid((unsigned int)status_ptr, sizeof(int)) < 0) {
    lprintf("The address status_ptr isn't valid");
    return -1;
  }
  
  
  lprintf("Wait %d", kernel.current_thread->tid);

  // Check if this thread will wait infinitely 
  pcb_t *curr_task = kernel.current_thread->task;
  lprintf("Taking list mutex %p for task", &curr_task->list_mutex);
  eff_mutex_lock(&curr_task->list_mutex);
  // lprintf("Taken list mutex %p for task", &curr_task->list_mutex);
  lprintf("Waiting threads %d running children %d", (int)curr_task->num_waiting_threads, (int)curr_task->num_running_children);
  if (curr_task->num_waiting_threads >= curr_task->num_running_children) {
    // This thread will wait infinitely
    lprintf("Waitin children equals running children. Returning -1");
    eff_mutex_unlock(&curr_task->list_mutex);
    lprintf("Waitin children equals running children. Returning -1. Mutex unlocked");
    return -1;
  }
  
  if (curr_task->zombie_children.head != NULL) {
    lprintf("At least 1 zombie child present");
    
    // Remove the zombie child
    generic_node_t * zombie_child_node = stack_queue_dequeue(&curr_task->zombie_children);
    if (zombie_child_node == NULL) {
      lprintf("kern_wait(): zombie queue delete failed");
      return -1;
    }

    pcb_t* zombie_child = zombie_child_node->value;

    curr_task->num_running_children--;
    eff_mutex_unlock(&curr_task->list_mutex);

    if (status_ptr != NULL) {
      *status_ptr = zombie_child->return_status;
    }
    int ret = zombie_child->original_thread_id;
    
    // TODO: Free kernel stack of the last thread
    // TODO: Destroy the mutexes and everything?
    lprintf("Freeing the pcb %p of thread %d. My tid %d.", zombie_child, zombie_child->original_thread_id, kernel.current_thread->tid);
    free(zombie_child);
    lprintf("Free returned");
 
    return ret;
  }

  curr_task->num_waiting_threads++;
  lprintf("No zombie child");
  // No zombie children at the time
  // Wait for at least one thread to vanish

  // Enqueue myself in the the queue of waiting threads
  generic_node_t new_waiting = {kernel.current_thread, NULL};
  stack_queue_enqueue(&curr_task->waiting_threads, &new_waiting);
  
  eff_mutex_unlock(&curr_task->list_mutex);
  lprintf("Reaching there");
  int x = 0;
  lprintf("Calling kern_deschedule waiting for thread %d", kernel.current_thread->tid);
  kern_deschedule(&x);
  lprintf("SHOULDNT PRINT UNTIL THE FIRST ONE EXITS");
  
  if (status_ptr != NULL) {
    *status_ptr = kernel.current_thread->reaped_task->return_status;
  }
  int ret = kernel.current_thread->reaped_task->original_thread_id;

  // TODO: Free kernel stack of the last thread
  lprintf("Freeing pcb of the reaped task. My tid %d", kernel.current_thread->tid);
  free(kernel.current_thread->reaped_task);
  lprintf("Free returned");

  return ret;
}
