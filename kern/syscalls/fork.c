/** @file fork.c
 *  @brief This file contains the definition for the kern_fork() and
 *  kern_thread_fork() system calls.
 *  @author akanjani, lramire1
 */

#include <common_kern.h>
#include <cr.h>
#include <kernel_state.h>
#include <malloc.h>
#include <page.h>
#include <string.h>
#include <syscalls/fork_helper.h>
#include <context_switch.h>
#include <scheduler.h>
#include <asm.h>
#include <syscalls.h>
#include <assert.h>

/* VM system */
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <virtual_memory_defines.h>

/* Debugging */
#include <simics.h>

#define NB_REGISTERS_POPA 8

static unsigned int * copy_memory_regions(void);
static unsigned int * initialize_stack_fork(uint32_t orig_stack, 
                      uint32_t new_stack, unsigned int * esp, tcb_t * new_tcb);

/** @brief  Creates a new task by copying the invoking task's memory regions 
 *
 *  The new task receives an exact, coherent copy of all memory regions of the 
 *  invoking task. The new task contains a single thread which is a copy of the
 *  thread invoking fork() except for the return value of the system call.
 *  The exit status of a newly-created task is 0. If a thread in the task 
 *  invoking fork() has a software exception handler registered, the corresponding
 *  thread in the newly-created task will have exactly the same handler 
 *  registered.
 *
 *  @param  esp   The limit address on the invoking thread's stack used for
 *                creating the child thread's stack
 *
 *  @return If fork() succeeds, the invoking thread will receive the ID of the 
 *          new task’s thread and the newly created thread will receive the 
 *          value zero. Errors are reported via a negative return value, in 
 *          which case no new task has been created.
 */
int kern_fork(unsigned int *esp) {

  // TODO: Reject call if more than one thread in the task ?
  // TODO: Need to register software exception handler

  eff_mutex_lock(&kernel.mutex);
  if (kernel.current_thread->num_of_frames_requested <= kernel.free_frame_count) {
    // TODO: Shouldn't it be kernel.current_thread->task->num_of_frames_requested ?
    kernel.free_frame_count -= kernel.current_thread->num_of_frames_requested;
  } else {
    lprintf("Can't fork as no frames left");
    eff_mutex_unlock(&kernel.mutex);
    return -1;
  }
  eff_mutex_unlock(&kernel.mutex);

  // Allocate a kernel stack for the new task
  void *stack_kernel = malloc(PAGE_SIZE);
  if (stack_kernel == NULL) {
    lprintf("fork(): Could not allocate kernel stack for task's root thread");
    // TODO: Increase the kernel free frame count
    return -1;
  }

  unsigned int * new_cr3 = copy_memory_regions();
  if (new_cr3 == NULL) {
    lprintf("fork(): Could not allocate memory regions");
    // TODO: Increase the kernel free frame count
    free(stack_kernel);    
    return -1;
  }

  // SIMICS Debugging 
  // sim_reg_child(new_cr3, (void *) kernel.current_thread->cr3);

  // Highest address of child's kernel stack
  uint32_t esp0 = (uint32_t)(stack_kernel) + PAGE_SIZE;

  // Create new PCB/TCB for the task and its root thread
  pcb_t *new_pcb = create_new_pcb();
  if (new_pcb == NULL) {
    lprintf("fork(): PCB initialization failed");
    free(stack_kernel);
    free_address_space(new_cr3, KERNEL_AND_USER_SPACE);
    return -1;
  }

  tcb_t *new_tcb = create_new_tcb(new_pcb, esp0, (uint32_t)new_cr3);
  if (new_tcb == NULL) {
    lprintf("fork(): TCB initialization failed");
    free(stack_kernel);
    hash_table_remove_element(&kernel.pcbs, new_pcb);
    free_address_space(new_cr3, KERNEL_AND_USER_SPACE);
    // TODO: Increase the kernel free frame count
    return -1;
  }
  new_pcb->original_thread_id = new_tcb->tid;
  new_pcb->parent = kernel.current_thread->task;
  lprintf("Setting %p as the task for thread %d", new_pcb, new_tcb->tid);
  new_tcb->task = new_pcb;
  

  // Add the child to the running queue
  eff_mutex_lock(&kernel.current_thread->task->list_mutex);
  kernel.current_thread->task->num_running_children++;
  linked_list_insert_node(&kernel.current_thread->task->running_children, new_pcb);
  eff_mutex_unlock(&kernel.current_thread->task->list_mutex);

  // Craft the kernel stack for the new thread
  new_tcb->esp = (uint32_t) initialize_stack_fork(kernel.current_thread->esp0,
                                                  esp0, esp, new_tcb);

  lprintf("\tkern_fork(): Thread %d forking %d", kernel.current_thread->tid, new_tcb->tid);

  // Make the thread runnable
  add_runnable_thread(new_tcb);

  return new_tcb->tid;
}

/** @brief  Creates a new thread in the current task
 *
 *  All registers (except %eax) in the new thread will be initialized to the
 *  same values as the corresponding registers in the old thread. A thread newly 
 *  created by thread fork has no software exception handler registered. threads
 *  are runnable as soon as they are created.
 *
 *  @param  esp   The limit address on the invoking thread's stack used for
 *                creating the child thread's stack
 *
 *  @return The invoking thread’s return value in %eax is the thread ID of the 
 *          newly-created thread; the new thread’s return value is zero. 
 *          Errors are reported via a negative return value, in which case no 
 *          new thread has been created.
 */
int kern_thread_fork(unsigned int * esp) {
  
  pcb_t * current_task = kernel.current_thread->task;

  // Allocate new kernel stack
  void* kernel_stack = malloc(PAGE_SIZE);
  if (kernel_stack == NULL) {
    lprintf("kern_thread_fork(): Unable to allocate kernel stack");
    return -1;
  }

  uint32_t esp0 = (uint32_t)(kernel_stack) + PAGE_SIZE;

  // Create new TCB
  tcb_t * new_tcb = create_new_tcb(current_task, esp0, 
                                      kernel.current_thread->cr3);

  // Craft the kernel stack for the new thread  
  new_tcb->esp = (uint32_t) initialize_stack_fork(kernel.current_thread->esp0,
                                                  esp0, esp, new_tcb);

  // Increment the number of threads in the current task
  eff_mutex_lock(&current_task->mutex);
  ++current_task->num_of_threads;
  eff_mutex_unlock(&current_task->mutex);

  // Make the thread runnable
  add_runnable_thread(new_tcb);

  return new_tcb->tid;
}

/** @brief  Initializes the child thread's stack for the fork() and 
 *          thread_fork() system calls
 *
 *  This function copy part of the parent thread's stack onto the child thread's
 *  stack. Then the function manually creates stack frames for the child thread
 *  in order for it to be ready for the first context switch.  
 *
 *  @param  orig_stack  The highest address of the parent's thread task
 *  @param  new_stack   The highest address of the child's thread stack 
 *  @param  esp         The limit on the parent's stack at which the copy
 *                      should stop (inclusive)
 *  @param  new_tcb     A pointer to the child thread's TCB
 *
 *  @return A pointer to an unsigned int indicating the virtual address on the
 *          child thread's new stack above which we can start pushing data 
 */
static unsigned int * initialize_stack_fork(uint32_t orig_stack, 
                      uint32_t new_stack, unsigned int * esp, tcb_t * new_tcb) {

  // Copy part of the kernel stack of the original task to the new one's
  char *orig = (char*) (((char *)orig_stack) - 1); 
  char *new = (char*) (((char *)new_stack) - 1);
  for ( ; (unsigned int)orig >= (unsigned int)esp ; --orig, --new) {
    *new = *orig;
  }

  // Manually craft the new stack for first context switch to this thread
  unsigned int * stack_addr = (unsigned int*) (new + 1);
  --stack_addr;
  *stack_addr = (unsigned int) new_tcb;
  --stack_addr;
  *stack_addr = (unsigned int) fork_return_new_thread;
  --stack_addr;
  *stack_addr = (unsigned int) init_thread;
  stack_addr -= NB_REGISTERS_POPA;

  return stack_addr;                    
}

/** @brief  Creates a copy of the invoking task entire address space, in order
 *          to be used by a newly created child task
 *
 *  This function will fail if the copy buffer can't be allocated or if there
 *  isn't enough memory in user space to copy the entire address space. In that
 *  case the function returns NULL and the previously allocated frames (if any)
 *  are deallocated before returning.
 *
 *  @return A pointer to the page directory address for the child task on 
 *          success, NULL on error
 */
static unsigned int * copy_memory_regions() {

  // Create a copy buffer
  char *buffer = malloc(PAGE_SIZE);
  if (buffer == NULL) {
    lprintf("copy_memory_regions(): Unable to allocate buffer");
    return NULL;
  }

  unsigned int *orig_cr3 = (unsigned int *)get_cr3();

  // Create a new page directory
  unsigned int *new_cr3 = (unsigned int *)smemalign(PAGE_SIZE, PAGE_SIZE);
  if (new_cr3 == NULL) {
    free(buffer);
    return NULL;
  }

  // Memset the page directory to 0
  memset(new_cr3, 0, PAGE_SIZE);

  unsigned int nb_entries = PAGE_SIZE / SIZE_ENTRY_BYTES;
  unsigned int *orig_dir_entry, *new_dir_entry;
  for (orig_dir_entry = orig_cr3, new_dir_entry = new_cr3;
       orig_dir_entry < orig_cr3 + nb_entries;
       ++orig_dir_entry, ++new_dir_entry) {

    // If the page directory entry is present
    if (is_entry_present(orig_dir_entry)) {
      lprintf("Copying directory entry %p to %p", orig_dir_entry, new_dir_entry);

      unsigned int *orig_page_table_addr = get_page_table_addr(orig_dir_entry);
      unsigned int *new_page_table_addr = 
                      create_page_table(new_dir_entry, DIRECTORY_FLAGS, FIRST_TASK_FALSE);
      
      if (is_entry_present(orig_page_table_addr) && 
          (unsigned int)get_frame_addr(orig_page_table_addr) < USER_MEM_START) {
        continue;
      }
      if (new_page_table_addr == NULL) {
        lprintf("copy_memory_regions(): Unable to allocate new page table");
        free(buffer);    

        // Free everything previously allocated
        free_address_space(new_cr3, KERNEL_AND_USER_SPACE);
        return NULL;
      }

      unsigned int *orig_tab_entry, *new_tab_entry;
      for (orig_tab_entry = orig_page_table_addr,
          new_tab_entry = new_page_table_addr;
           orig_tab_entry < orig_page_table_addr + nb_entries;
           ++orig_tab_entry, ++new_tab_entry) {

        // If the page table entry is present
        if (is_entry_present(orig_tab_entry)) {

          if ((unsigned int)get_frame_addr(orig_tab_entry) < USER_MEM_START) {
            // TODO: Redundant?
            // This is direct mapped kernel memory
            *new_tab_entry = *orig_tab_entry;
          } else {
            // This is user space memory, we have to allocate a new frame

            // Create a new page table entry
            if (create_page_table_entry(new_tab_entry,
                                    get_entry_flags(orig_tab_entry)) == NULL) {
              lprintf("copy_memory_regions(): Unable to allocate frame");
              free(buffer);    

              // Free everything previously allocated
              free_address_space(new_cr3, KERNEL_AND_USER_SPACE);
              return NULL;        
            }

            // Get the virtual address associated with the frame in the original
            // task
            unsigned int orig_virtual_address =
                get_virtual_address(orig_dir_entry, orig_tab_entry);

            // Get the virtual address associated with the frame in the new task
            unsigned int new_virtual_address =
                get_virtual_address(new_dir_entry, new_tab_entry);

            // Copy the frame content to the buffer
            memcpy(buffer, (unsigned int *)orig_virtual_address, PAGE_SIZE);

            // Copy the frame to the new task user space
            kernel.current_thread->cr3 = (uint32_t)new_cr3;
            set_cr3((uint32_t)new_cr3);
            memcpy((unsigned int *)new_virtual_address, buffer, PAGE_SIZE);
            kernel.current_thread->cr3 = (uint32_t)orig_cr3;
            set_cr3((uint32_t)orig_cr3);
          }
        }
      }
    }
  }

  // Free the buffer
  free(buffer);

  return new_cr3;
}
