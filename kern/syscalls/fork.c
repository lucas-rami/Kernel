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

/* VM system */
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <virtual_memory_defines.h>

#include <simics.h>

#define NB_REGISTERS_POPA 8

static unsigned int * copy_memory_regions(void);

// TODO: doc
int kern_fork(unsigned int *esp) {

  // NOTE: Reject call if more than one thread in the task ?
  // TODO: Need to register software exception handler
  // TODO: Args validation?? Should be OK, the argument is not given by the user 

  mutex_lock(&kernel.mutex);
  if (kernel.current_thread->num_of_frames_requested <= kernel.free_frame_count) {
    // Shouldn't it be kernel.current_thread->task->num_of_frames_requested ?
    kernel.free_frame_count -= kernel.current_thread->num_of_frames_requested;
  } else {
    lprintf("Can't fork as no frames left");
    mutex_unlock(&kernel.mutex);
    return -1;
  }
  mutex_unlock(&kernel.mutex);

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

  uint32_t esp0 = (uint32_t)(stack_kernel) + PAGE_SIZE;

  // Create new PCB/TCB for the task and its root thread
  pcb_t *new_pcb = create_new_pcb();
  if (new_pcb == NULL) {
    lprintf("fork(): PCB initialization failed");
    free(stack_kernel);
    free_address_space(new_cr3, KERNEL_AND_USER_SPACE);
    // TODO: Increase the kernel free frame count
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
  

  // Add the child to the running queue
  mutex_lock(&kernel.current_thread->task->list_mutex);
  queue_insert_node(&kernel.current_thread->task->running_children, new_pcb);
  mutex_unlock(&kernel.current_thread->task->list_mutex);

  /* ------ Craft the kernel stack for the new task ------ */

  // Copy part of the kernel stack of the original task to the new one's
  char *orig_stack, *new_stack;
  for (orig_stack = (char *)kernel.current_thread->esp0,
      new_stack = (char *)esp0;
       (unsigned int)orig_stack >= (unsigned int)esp;
       --orig_stack, --new_stack) {

    *new_stack = *orig_stack;

  }

  // Manually craft the new stack for first context switch to this thread
  unsigned int * stack_addr = (unsigned int*) (new_stack + 1);
  --stack_addr;
  *stack_addr = (unsigned int) new_tcb;
  --stack_addr;
  *stack_addr = (unsigned int) fork_return_new_task;
  --stack_addr;
  *stack_addr = (unsigned int) init_thread;
  stack_addr -= NB_REGISTERS_POPA;

  new_tcb->esp = (uint32_t) stack_addr;

  // Make the thread runnable
  add_runnable_thread(new_tcb);

  return new_tcb->tid;
}

int kern_thread_fork() {
  // TODO
  return -1;
}

// TODO: doc
static unsigned int * copy_memory_regions() {

  char *buffer = malloc(PAGE_SIZE);
  if (buffer == NULL) {
    lprintf("copy_memory_regions(): Unable to allocate buffer");
    return NULL;
  }

  unsigned int *orig_cr3 = (unsigned int *)get_cr3();
  unsigned int *new_cr3 = (unsigned int *)smemalign(PAGE_SIZE, PAGE_SIZE);
  if (new_cr3 == NULL) {
    free(buffer);
    return NULL;
  }

  memset(new_cr3, 0, PAGE_SIZE);

  unsigned int nb_entries = PAGE_SIZE / SIZE_ENTRY_BYTES;
  unsigned int *orig_dir_entry, *new_dir_entry;
  for (orig_dir_entry = orig_cr3, new_dir_entry = new_cr3;
       orig_dir_entry < orig_cr3 + nb_entries;
       ++orig_dir_entry, ++new_dir_entry) {

    // If the page directory entry is present
    if (is_entry_present(orig_dir_entry)) {

      unsigned int *orig_page_table_addr = get_page_table_addr(orig_dir_entry);
      unsigned int *new_page_table_addr = 
                      create_page_table(new_dir_entry, PAGE_TABLE_FLAGS);

      unsigned int *orig_tab_entry, *new_tab_entry;
      for (orig_tab_entry = orig_page_table_addr,
          new_tab_entry = new_page_table_addr;
           orig_tab_entry < orig_page_table_addr + nb_entries;
           ++orig_tab_entry, ++new_tab_entry) {

        // If the page table entry is present
        if (is_entry_present(orig_tab_entry)) {

          if ((unsigned int)get_frame_addr(orig_tab_entry) < USER_MEM_START) {
            // This is direct mapped kernel memory
            *(new_tab_entry) = *orig_tab_entry;
          } else {
            // This is user space memory, we have to allocated a new frame

            // Create a new page table entry
            create_page_table_entry(new_tab_entry,
                                    get_entry_flags(orig_tab_entry));

            // Get the virtual address associated with the frame in the original
            // task
            unsigned int orig_virtual_address =
                get_virtual_address(orig_dir_entry, orig_tab_entry);

            // Get the virtual address associated with the frame in the new task
            unsigned int new_virtual_address =
                get_virtual_address(new_dir_entry, new_tab_entry);

            // Copy the frame content to the buffer
            memcpy(buffer, (unsigned int *)orig_virtual_address, PAGE_SIZE);

            // Copy the frame to the new task user spce
            set_cr3((uint32_t)new_cr3);
            memcpy((unsigned int *)new_virtual_address, buffer, PAGE_SIZE);
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
