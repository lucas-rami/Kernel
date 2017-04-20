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
#include <malloc.h>
#include <stack_queue.h>

#define EXITED 5
#define LAST_THREAD_FALSE 0
#define LAST_THREAD_TRUE 1

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
  lprintf("Vanish %d", kernel.current_thread->tid);

  int is_last_thread = LAST_THREAD_FALSE;

  pcb_t *curr_task = kernel.current_thread->task;

  eff_mutex_lock(&curr_task->mutex);
  if (curr_task->num_of_threads <= 1) {
    is_last_thread = LAST_THREAD_TRUE;
    curr_task->task_state = EXITED;
  }
  eff_mutex_unlock(&curr_task->mutex);

  if (is_last_thread == LAST_THREAD_TRUE) {
    eff_mutex_lock(&curr_task->list_mutex);
    // Mark all running children's parent as init
    generic_node_t *temp = curr_task->running_children.head;
    while(temp != NULL) {
      pcb_t *pcb_tmp = (pcb_t*)temp->value;
      generic_node_t *next = temp->next;
      pcb_tmp->parent = kernel.init_task;
      free(temp);
      temp = next;
    }
    eff_mutex_unlock(&curr_task->list_mutex);

    // Update number of running children for init
    eff_mutex_lock(&kernel.init_task->list_mutex);
    kernel.init_task->num_running_children += curr_task->num_running_children;
    eff_mutex_unlock(&kernel.init_task->list_mutex);
    
    // Free up the virtual memory. Switch to the init's cr3 to make kernel code
    // run
    // lprintf("Setting cr3 as %d", (int)kernel.init_cr3);
    // TODO: Change this so that only a task has cr3
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
    eff_mutex_lock(&kernel.mutex);
    kernel.free_frame_count += curr_task->num_of_frames_requested;
    eff_mutex_unlock(&kernel.mutex);

    // TODO: Delete the linked list allocations which we store for new pages
    linked_list_delete_list(&kernel.current_thread->task->allocations);
    // lprintf("Linked list deleted for new_pages");
    // MAGIC_BREAK;

    eff_mutex_lock(&curr_task->list_mutex);

    generic_node_t *curr_zombie_list = curr_task->zombie_children.head;

    if (curr_zombie_list != NULL) {
      eff_mutex_lock(&kernel.init_task->list_mutex);
      generic_node_t *init_zombie_tail = kernel.init_task->zombie_children.tail;
      if (init_zombie_tail == NULL) {
        init_zombie_tail = (kernel.init_task->zombie_children.head = curr_zombie_list);
      } else {
        init_zombie_tail->next = curr_zombie_list;
      }
      eff_mutex_unlock(&kernel.init_task->list_mutex);
    }

    eff_mutex_unlock(&curr_task->list_mutex);


    // lprintf("Modifying the parent now. Mutex %p", &curr_task->parent->list_mutex);
    eff_mutex_lock(&curr_task->parent->list_mutex);
    // lprintf("Took a lock on parent");
    
    // Remove yourself from the running queue of the parent
    // Even if I am not in the linked list, this function will have no 
    // side effect
    linked_list_delete_node(&curr_task->parent->running_children, curr_task);

    if (is_stack_queue_empty(&curr_task->parent->waiting_threads)) {
      // None of the threads of my parent process are waiting for me.
      // Add myself to the zombie queue of the parent
      generic_node_t new_zombie = {curr_task, NULL};
      stack_queue_enqueue(&curr_task->parent->zombie_children, &new_zombie);
      // lprintf("Adding task %p %d to %p %d zombie children", curr_task, curr_task->tid, curr_task->parent, curr_task->parent->tid);
    } else {
      // At least one thread is waiting in my parent process
      generic_node_t *wait_thread_node = stack_queue_dequeue(&curr_task->parent->waiting_threads);
      assert(wait_thread_node != NULL);

      tcb_t* wait_thread = wait_thread_node->value;
      wait_thread->reaped_task = curr_task;
      curr_task->parent->num_running_children--;
      curr_task->parent->num_waiting_threads--;

      // kern_make_runnable(wait_thread->tid);
      // lprintf("Current thread %d. Made runnable %d", kernel.current_thread->tid, wait_thread->tid);
     /* < 0) {
        lprintf("Make runnable failed. Current thread %d, mkr thread %d", kernel.current_thread->tid, wait_thread->tid);
      }*/
      while (kern_make_runnable(wait_thread->tid) < 0) {
        lprintf("Make runnable failing");
      }
      // Populate the address of the zombie block in the tcb of wait thread
      // Make runnable the wait thread
    }
  } 

  // lprintf("Removing tcb for thread %d", kernel.current_thread->tid);
  hash_table_remove_element(&kernel.tcbs, kernel.current_thread);
  // TODO: What about the PCB and the kernel stack. How do you delete those?
  disable_interrupts();
  if (is_last_thread == LAST_THREAD_TRUE) {
    eff_mutex_unlock(&curr_task->parent->list_mutex);
  }

  block_and_switch(HOLDING_MUTEX_FALSE);

  lprintf("kern_vanish(): Should never have reached here");
  assert(0);
  return;
}
