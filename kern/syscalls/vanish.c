#include <hash_table.h>
#include <kernel_state.h>
#include <mutex.h>
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
  lprintf("Kern vanish");
  hash_table_remove_element(&kernel.tcbs, kernel.current_thread);
  // Add the tcb to the garbage collector list
  // Add the kernel stack for this thread to the garbage collector
  mutex_lock(&kernel.current_thread->task->mutex);
  if (kernel.current_thread->task->num_of_threads <= 1) {
    kernel.current_thread->task->task_state = EXITED;
    mutex_unlock(&kernel.current_thread->task->mutex);
    hash_table_remove_element(&kernel.pcbs, kernel.current_thread->task);
    
    // Free up the virtual memory. Switch to the init's cr3 to make kernel code
    // run
    set_cr3(kernel.init_cr3);
    free_address_space((unsigned int *)kernel.current_thread->task->cr3, KERNEL_AND_USER_SPACE);
    // TODO: Increase the free frame count. Can be done in deallocating code 
    // itself
    // TODO: Delete the linked list allocations which we store for new pages
    linked_list_delete_list(&kernel.current_thread->task->allocations);

    pcb_t *curr_task = kernel.current_thread->task;
    mutex_lock(&curr_task->list_mutex);

    // Mark all running children's parent as init
    generic_node_t *tmp = curr_task->running_children.head;
    while (tmp != NULL) {
      ((pcb_t*)tmp->value)->parent = kernel.init_task;
      tmp = tmp->next;
    }

    tmp = curr_task->zombie_children.head;
    while (tmp != NULL) {
      queue_delete_node(&curr_task->zombie_children);
      if (queue_insert_node(&kernel.init_task->zombie_children, tmp->value) < 0) {
        // TODO: Handle this
        lprintf("FATAL ERROR. Can't add the thread to the zombie list of init");
      }
      tmp = tmp->next;
    }

    mutex_unlock(&curr_task->list_mutex);

    mutex_lock(&curr_task->parent->list_mutex);
    if (is_queue_empty(&curr_task->parent->waiting_threads)) {
      // None of the threads of my parent process are waiting for me.
      // Add myself to the zombie queue of the parent
      queue_insert_node(&curr_task->parent->zombie_children, curr_task);
    } else {
      // At least one thread is waiting in my parent process
      tcb_t *wait_thread = queue_delete_node(&curr_task->parent->waiting_threads);
      wait_thread->reaped_task = curr_task;
      kern_make_runnable(wait_thread->tid);
      // Populate the address of the zombie block in the tcb of wait thread
      // Make runnable the wait thread
    }
    mutex_unlock(&curr_task->parent->list_mutex);
  } else {
    mutex_unlock(&kernel.current_thread->task->mutex);
  }

  // TODO: What about the PCB and the kernel stack. How do you delete those?
  disable_interrupts();
  context_switch(next_thread());
  lprintf("kern_vanish(): Should never have reached here");
  assert(0);
  return;
}
