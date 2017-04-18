#include <hash_table.h>
#include <kernel_state.h>
#include <eff_mutex.h>
#include <cr.h>
#include <dynamic_queue.h>
#include <linked_list.h>
#include <stddef.h>
#include <pcb.h>
#include <simics.h>
#include <tcb.h>
#include <context_switch.h>
#include <assert.h>
#include <virtual_memory.h>
#include <virtual_memory_defines.h>
#include <syscalls.h>
#include <asm.h>
#include <scheduler.h>

#define EXITED 5

void kern_vanish() {
  // Do the opposite of kern_thread_fork
  // Destroy the tcb
  // Call the grabage collector
  // Is mutex_lock(&kernel.mutex) reqd?
  // hash_table_remove_element(&kernel.tcbs, kernel.current_thread);
  // Add the tcb to the garbage collector list
  // Add the kernel stack for this thread to the garbage collector

  lprintf("\tkern_vanish(): Thread %d vanishing", kernel.current_thread->tid);

  pcb_t *curr_task = kernel.current_thread->task;

  // Get mutex on current task's state
  eff_mutex_lock(&curr_task->mutex);

  if (curr_task->num_of_threads <= 1) {
    // We are the last thread in the task...

    // Mark the task as EXITED
    curr_task->task_state = EXITED;

    // Release mutex on current task's state
    eff_mutex_unlock(&curr_task->mutex);
    
    // hash_table_remove_element(&kernel.pcbs, curr_task);
    
    // Free up the virtual memory. Switch to the init's cr3 to make kernel code
    // run
   
    // TODO: Change this to task again when above is fixed

    // Free whole address space of invoking task's
    unsigned int *cr3 = (unsigned int *)kernel.current_thread->cr3;
    kernel.current_thread->cr3 = kernel.init_cr3;
 
    set_cr3(kernel.init_cr3);
    free_address_space(cr3, KERNEL_AND_USER_SPACE);
   
    // TODO: Increase the free frame count. Can be done in deallocating code 
    // itself

    // Delete linked list of task's allocations
    linked_list_delete_list(&curr_task->allocations);

    // To modify task's ZOMBIE/RUNNING list
    eff_mutex_lock(&curr_task->list_mutex);

    // Mark all running children's parent as init
    generic_node_t *tmp = curr_task->running_children.head;
    while (tmp != NULL) {
      ((pcb_t*)tmp->value)->parent = kernel.init_task;
      tmp = tmp->next;
    }

    // Transfer all ZOMBIE tasks from this task's list to the idle task's list
    tmp = curr_task->zombie_children.head;
    while (tmp != NULL) {
      queue_delete_node(&curr_task->zombie_children);
      if (queue_insert_node(&kernel.init_task->zombie_children, tmp->value) < 0) {
        // TODO: Handle this
        lprintf("\tkern_vanish(): Failed to add PCB to ZOMBIE list");
        assert(0);
      }
      tmp = tmp->next;
    }

    // Release the mutex task's ZOMBIE/RUNNING list
    eff_mutex_unlock(&curr_task->list_mutex);

    // To modify parent task's ZOMBIE/RUNNING list
    eff_mutex_lock(&curr_task->parent->list_mutex);
    
    if (is_queue_empty(&curr_task->parent->waiting_threads)) {
      // No thread is waiting for me in, add myself to the zombie list
      queue_insert_node(&curr_task->parent->zombie_children, curr_task);

    } else {
      // At least one thread is waiting in my parent process...
      tcb_t *wait_thread = queue_delete_node(&curr_task->parent->waiting_threads);

      // Mark myself as its reaped task and make it runnable
      wait_thread->reaped_task = curr_task;
      add_runnable_thread(wait_thread);

    }

    // Release the mutex on parent task's ZOMBIE/RUNNING list    
    eff_mutex_unlock(&curr_task->parent->list_mutex);

  } else {

    // Release mutex on current task's state  
    eff_mutex_unlock(&kernel.current_thread->task->mutex);
  }

  // TODO: What about the PCB and the kernel stack. How do you delete those?

  // Remove TCB of current thread from hash table
  hash_table_remove_element(&kernel.tcbs, kernel.current_thread);

  // Context switch to someone else
  disable_interrupts();
  context_switch(next_thread());

  lprintf("\tkern_vanish(): Reached end of vanish -> PANIC");

  assert(0);

  return;
}
