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
#include <assert.h>

int kern_wait(int *status_ptr) {
  
  if (is_buffer_valid((unsigned int)&status_ptr, sizeof(uintptr_t)) < 0) {
    // status ptr isn't a valid memory address
    lprintf("kern_wait(): Invalid args");
  }

  // If number of running children is less than or equal
  // to number of waiitng threads, return err
  if (status_ptr != NULL && is_buffer_valid((unsigned int)*status_ptr, sizeof(int)) < 0) {
    lprintf("The address status_ptr isn't valid");
    return -1;
  }

  lprintf("\tkern_wait(status_ptr = %p): Thread %d waiting", status_ptr, kernel.current_thread->tid);

  int ret;

  // Get current task
  pcb_t *curr_task = kernel.current_thread->task;
  
  // To modify task's ZOMBIE/WAITING list
  eff_mutex_lock(&curr_task->list_mutex);

  if (curr_task->zombie_children.head != NULL) {
    // If one child is in ZOMBIE state... 

    // Remove it from the ZOMBIE list
    pcb_t *zombie_child = queue_delete_node(&curr_task->zombie_children);
    if (zombie_child == NULL) {
      lprintf("\tkern_wait(): zombie queue delete failed");
      assert(0);
      return -1;
    }

    // Release the mutex for ZOMBIE list
    eff_mutex_unlock(&curr_task->list_mutex);

    // Collect return status of the child
    if (status_ptr != NULL) {
      *status_ptr = zombie_child->return_status;
    }
    ret = zombie_child->original_thread_id;
    
    // TODO: Free kernel stack of the last thread
    free(zombie_child);

  } else {
    // No one is ZOMBIE state...

    // Insert myself in list of waiting threads
    queue_insert_node(&curr_task->waiting_threads, kernel.current_thread);

    // Release the mutex for WAITING list    
    eff_mutex_unlock(&curr_task->list_mutex);

    lprintf("Reaching there");

    // Deschedule thread while waiting for someone to vanish()
    int x = 0;
    kern_deschedule(&x);

    lprintf("SHOULDNT PRINT UNTIL THE FIRST ONE EXITS");

    // Collect return status of the child
    if (status_ptr != NULL) {
      *status_ptr = kernel.current_thread->reaped_task->return_status;
    }

    ret = kernel.current_thread->reaped_task->original_thread_id;
   
    // TODO: Free kernel stack of the last thread
    free(kernel.current_thread->reaped_task);

    return ret;
  }

  // Return original thread id of reaped task
  return ret;
}
