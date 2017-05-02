/** @file vanish.c
 *  @brief  This file contains the definition for kern_vanish() system call
 *          and its helper function
 *  @author akanjani, lramire1
 */

#include <hash_table.h>
#include <kernel_state.h>
#include <eff_mutex.h>
#include <cr.h>
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
#include <page.h>

#define EXITED 5
#define LAST_THREAD_FALSE 0
#define LAST_THREAD_TRUE 1

/** @brief  Terminates execution of the calling thread “immediately.” 
 *
 *   If the invoking thread is the last thread in its task, the kernel 
 *   deallocates all resources in use by the task and makes the exit status 
 *   of the task available to the parent task (the task which created this 
 *   task using fork()) via wait(). If the parent task is no longer running, 
 *   the exit status of the task is made available to the kernel-launched 
 *   “init” task instead. The statuses of any child tasks that have not 
 *   been collected via wait() are made available to the 
 *   kernel-launched “init” task.
 *
 *  @return  void
 *
 */
void kern_vanish() {

  int is_last_thread = LAST_THREAD_FALSE;

  pcb_t *curr_task = kernel.current_thread->task;

  eff_mutex_lock(&curr_task->mutex);
  if (curr_task->num_of_threads <= 1) {
    is_last_thread = LAST_THREAD_TRUE;
    curr_task->task_state = EXITED;
  }
  curr_task->num_of_threads--;
  eff_mutex_unlock(&curr_task->mutex);

  if (is_last_thread == LAST_THREAD_TRUE) {
    // Last thread vanishing
    
    eff_mutex_lock(&curr_task->list_mutex);

    // Update number of running children for init
    eff_mutex_lock(&kernel.init_task->list_mutex);
    kernel.init_task->num_running_children += curr_task->num_running_children;
    eff_mutex_unlock(&kernel.init_task->list_mutex);
    
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

    // Free any user space memory being used by this task
    unsigned int *cr3 = (unsigned int *)kernel.current_thread->cr3;
    kernel.current_thread->cr3 = kernel.init_cr3;
    set_cr3(kernel.init_cr3);
    free_address_space(cr3, KERNEL_AND_USER_SPACE);

    // Update the kernel count of frames
    release_frames(curr_task->num_of_frames_requested);

    // Delete the linked list allocations which we store for new pages
    linked_list_delete_list(&kernel.current_thread->task->allocations);

    eff_mutex_lock(&curr_task->list_mutex);

    generic_node_t *curr_zombie_list = curr_task->zombie_children.head;

    if (curr_zombie_list != NULL) {
      // At least 1 zombie child of this task
      eff_mutex_lock(&kernel.init_task->list_mutex);
      generic_node_t *init_zombie_tail = kernel.init_task->zombie_children.tail;
      if (init_zombie_tail == NULL) {
        // Init has no zombie child. Make its list's head as that of mine
        init_zombie_tail = 
              (kernel.init_task->zombie_children.head = curr_zombie_list);
      } else {
        // Init at least has one zombie child. Append my list
        init_zombie_tail->next = curr_zombie_list;
      }
      eff_mutex_unlock(&kernel.init_task->list_mutex);
    }

    eff_mutex_unlock(&curr_task->list_mutex);

    eff_mutex_lock(&curr_task->parent->list_mutex);
    
    // Remove yourself from the running queue of the parent
    // Even if the task is not in the linked list, this function will have no 
    // side effect
    linked_list_delete_node(&curr_task->parent->running_children, curr_task);

    if (is_stack_queue_empty(&curr_task->parent->waiting_threads)) {
      // None of the threads of my parent process are waiting for me.
      // Add myself to the zombie queue of the parent
      generic_node_t new_zombie = {curr_task, NULL};
      stack_queue_enqueue(&curr_task->parent->zombie_children, &new_zombie);
      curr_task->last_thread_esp0 = kernel.current_thread->esp0 - PAGE_SIZE;
    } else {
      // At least one thread is waiting in my parent process
      generic_node_t *wait_thread_node = 
                    stack_queue_dequeue(&curr_task->parent->waiting_threads);
      assert(wait_thread_node != NULL);

      tcb_t* wait_thread = wait_thread_node->value;
      curr_task->last_thread_esp0 = kernel.current_thread->esp0 - PAGE_SIZE;
      wait_thread->reaped_task = curr_task;
      curr_task->parent->num_running_children--;
      curr_task->parent->num_waiting_threads--;

      add_runnable_thread(wait_thread);
    }
  } 

  // Remove the tcb from the hashmap
  hash_table_remove_element(&kernel.tcbs, kernel.current_thread);

  eff_mutex_lock(&kernel.gc.mp);

  // Free everything in the garbage collector queue at this time
  generic_node_t *delete_zombie_mem;
  while((delete_zombie_mem = 
                stack_queue_dequeue(&kernel.gc.zombie_memory)) != NULL) {
    free((tcb_t*)delete_zombie_mem->value);
  }

  generic_node_t tmp_delete;
  tmp_delete.value = kernel.current_thread;
  tmp_delete.next = NULL;
  // Enqueue the tcb for the current thread in the garbage collector queue
  stack_queue_enqueue(&kernel.gc.zombie_memory, &tmp_delete);

  if (is_last_thread != LAST_THREAD_TRUE) {
    // The stack of the last thread will be freed by the wait call
    // Otherwise, add the kernel stack to the garbage collector queue
    generic_node_t tmp_delete2;
    tmp_delete2.value = (char*)(kernel.current_thread->esp0 - PAGE_SIZE);
    tmp_delete2.next = NULL;
    stack_queue_enqueue(&kernel.gc.zombie_memory, &tmp_delete2);
  }

  disable_interrupts();

  // Interrupts are disabled so no one can delete this kernel stack/tcb 
  // before context switch
  eff_mutex_unlock(&kernel.gc.mp);

  if (is_last_thread == LAST_THREAD_TRUE) {
    // Unlock the parent's list mutex
    eff_mutex_unlock(&curr_task->parent->list_mutex);
  }

  block_and_switch(HOLDING_MUTEX_FALSE, NULL);

  lprintf("kern_vanish(): Should never have reached here");
  assert(0);
  return;
}
