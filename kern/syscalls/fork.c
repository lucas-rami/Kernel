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
#include <virtual_memory.h>
#include <syscalls/fork_helper.h>
#include <context_switch.h>
#include <scheduler.h>
#include <asm.h>

#include <simics.h>

#define SIZE_ENTRY_BYTES 4
#define NB_REGISTERS_POPA 8

static unsigned int * copy_memory_regions();

// TODO: doc
int kern_fork(unsigned int *esp) {

  //lprintf("kern_fork: Called\n");
  // NOTE: Reject call if more than one thread in the task ?
  // TODO: Need to register software exception handler

  // Allocate a kernel stack for the new task
  void *stack_kernel = malloc(PAGE_SIZE);
  if (stack_kernel == NULL) {
    lprintf("fork(): Could not allocate kernel stack for task's root thread");
    return -1;
  }

  //lprintf("kern_fork: Kernel stack allocated. Calling copy memory regions\n");
  unsigned int * new_cr3 = copy_memory_regions();

  //lprintf("Copied memory regions\n");
  uint32_t esp0 = (uint32_t)(stack_kernel) + PAGE_SIZE;

  //lprintf("kern_fork: Creating new PCB\n");
  // Create new PCB/TCB for the task and its root thread
  pcb_t *new_pcb = create_new_pcb();
  if (new_pcb == NULL) {
    lprintf("fork(): PCB initialization failed");
    free(stack_kernel);
    // TODO: free copy of memory regions
    return -1;
  }

  //lprintf("kern_fork: Creating new TCB\n");
  tcb_t *new_tcb = create_new_tcb(new_pcb, esp0, (uint32_t)new_cr3);
  if (new_tcb == NULL) {
    lprintf("fork(): TCB initialization failed");
    free(stack_kernel);
    hash_table_remove_element(&kernel.pcbs, new_pcb);
    // TODO: free copy of memory regions
    return -1;
  }

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
  mutex_lock(&kernel.mutex);
  add_runnable_thread(new_tcb);
  mutex_unlock(&kernel.mutex);

  enable_interrupts();
  return new_pcb->tid;
}

int kern_thread_fork() {
  // TODO
  return -1;
}

// TODO: doc
static unsigned int * copy_memory_regions() {
  //lprintf("copy_memory_regions: Called\n");
  char *buffer = malloc(PAGE_SIZE);
  if (buffer == NULL) {
    lprintf("copy_memory_regions(): Unable to allocate buffer");
    return NULL;
  }

  //lprintf("copy_memory_regions: Buffer malloced\n");
  unsigned int *orig_cr3 = (unsigned int *)get_cr3();
  unsigned int *new_cr3 = (unsigned int *)smemalign(PAGE_SIZE, PAGE_SIZE);

  memset(new_cr3, 0, PAGE_SIZE);

  //lprintf("copy_memory_regions: New page directory created\n");
  unsigned int nb_entries = PAGE_SIZE / SIZE_ENTRY_BYTES;
  unsigned int *orig_dir_entry, *new_dir_entry;
  for (orig_dir_entry = orig_cr3, new_dir_entry = new_cr3;
       orig_dir_entry < orig_cr3 + nb_entries;
       ++orig_dir_entry, ++new_dir_entry) {

    // If the page directory entry is present
    if (is_entry_present(orig_dir_entry)) {
      //lprintf("copy_memory_regions: Page directory entry present\n");

      unsigned int *orig_page_table_addr = get_page_table_addr(orig_dir_entry);
      unsigned int *new_page_table_addr = create_page_table(new_dir_entry);

      unsigned int *orig_tab_entry, *new_tab_entry;
      for (orig_tab_entry = orig_page_table_addr,
          new_tab_entry = new_page_table_addr;
           orig_tab_entry < orig_page_table_addr + nb_entries;
           ++orig_tab_entry, ++new_tab_entry) {

        // If the page table entry is present
        if (is_entry_present(orig_tab_entry)) {

          //lprintf("copy_memory_regions: page table entry present\n");
          if ((unsigned int)get_frame_addr(orig_tab_entry) < USER_MEM_START) {
            // This is direct mapped kernel memory
            //lprintf("copy_memory_regions: Kernel space\n");
            *(new_tab_entry) = *orig_tab_entry;
          } else {
            // This is user space memory, we have to allocate a new frame

            //lprintf("copy_memory_regions: User space. Calling create_page_table_entry\n");
            // Create a new page table entry
            create_page_table_entry(new_tab_entry,
                                    get_entry_flags(orig_tab_entry));

            // Get the virtual address associated with the frame in the original
            // task
            unsigned int *orig_virtual_address =
                get_virtual_address(orig_dir_entry, orig_tab_entry);

            // Get the virtual address associated with the frame in the new task
            unsigned int *new_virtual_address =
                get_virtual_address(new_dir_entry, new_tab_entry);

            //lprintf("copy_memory_regions: Copying from old space to buffer\n");
            // Copy the frame content to the buffer
            memcpy(buffer, orig_virtual_address, PAGE_SIZE);

            //lprintf("copy_memory_regions: Copying from buffer to new virtual space\n");
            // Copy the frame to the new task user spce
            set_cr3((uint32_t)new_cr3);
            memcpy(new_virtual_address, buffer, PAGE_SIZE);
            set_cr3((uint32_t)orig_cr3);
            //lprintf("copy_memory_regions: Done copying frame\n");
          }
        }
      }
    }
  }

  // Free the buffer
  free(buffer);

  return new_cr3;
}
