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
  //lprintf("Kern vanish");
  // lprintf("TID %d", kernel.current_thread->tid);
  // hash_table_remove_element(&kernel.tcbs, kernel.current_thread);
  // Add the tcb to the garbage collector list
  // Add the kernel stack for this thread to the garbage collector
  eff_mutex_lock(&kernel.current_thread->task->mutex);
  if (kernel.current_thread->task->num_of_threads <= 1) {
    kernel.current_thread->task->task_state = EXITED;
    eff_mutex_unlock(&kernel.current_thread->task->mutex);
    // hash_table_remove_element(&kernel.pcbs, kernel.current_thread->task);
    
    // Free up the virtual memory. Switch to the init's cr3 to make kernel code
    // run
    // lprintf("Setting cr3 as %d", (int)kernel.init_cr3);
    // TODO: Change this so that only a task has cr3
    // disable_interrupts();
    unsigned int *cr3 = (unsigned int *)kernel.current_thread->cr3;
    // TODO: Change this to task again when above is fixed
    kernel.current_thread->cr3 = kernel.init_cr3;
    // lprintf("Setting cr3 as %p", (uint32_t*)kernel.init_cr3);
    // disable_interrupts();
    // enable_interrupts();
    set_cr3(kernel.init_cr3);
    // lprintf("Free address space called with cr3 %p", cr3);
    // enable_interrupts();
    free_address_space(cr3, KERNEL_AND_USER_SPACE);
    // lprintf("Freed address space");
    // TODO: Increase the free frame count. Can be done in deallocating code 
    // itself
    // TODO: Delete the linked list allocations which we store for new pages
    linked_list_delete_list(&kernel.current_thread->task->allocations);
    // lprintf("Linked list deleted for new_pages");
    // MAGIC_BREAK;

    pcb_t *curr_task = kernel.current_thread->task;
    eff_mutex_lock(&curr_task->list_mutex);

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

    eff_mutex_unlock(&curr_task->list_mutex);

    // lprintf("Modifying the parent now. Mutex %p", &curr_task->parent->list_mutex);
    eff_mutex_lock(&curr_task->parent->list_mutex);
    // lprintf("Took a lock on parent");
    
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
    eff_mutex_unlock(&curr_task->parent->list_mutex);
  } else {
    eff_mutex_unlock(&kernel.current_thread->task->mutex);
  }

  // lprintf("Vanish deleted everything");

  hash_table_remove_element(&kernel.tcbs, kernel.current_thread);
  // TODO: What about the PCB and the kernel stack. How do you delete those?
  disable_interrupts();
  context_switch(next_thread());
  lprintf("kern_vanish(): Should never have reached here");
  assert(0);
  return;
}
