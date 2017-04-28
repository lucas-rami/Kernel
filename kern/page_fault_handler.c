/** @file page_fault_handler.c
 *  @brief  This file contains the definition for the page fault handler and the
 *          function to register the page fault handler in the IDT
 *  @author akanjani, lramire1
 */

#include <interrupts.h>
#include "page_fault_asm.h"
#include <seg.h>
#include <simics.h>
#include <virtual_memory_helper.h>
#include <cr.h>
#include <syscalls.h>
#include <kernel_state.h>

/* Index of page fault handler in the IDT */
#define PAGE_FAULT_IDT 0xE

/** @brief  Registers the page fault handler in the IDT
 *
 *  @return 0 on success, a negative number on error
 */
int page_fault_init(void) {
  return register_handler((uintptr_t)page_fault_handler, TRAP_GATE, 
      PAGE_FAULT_IDT, USER_PRIVILEGE_LEVEL, SEGSEL_KERNEL_CS);
}

/** @brief  Page fault handler
 *
 *  The function first checks whether the page fault is cause by a first write
 *  to a page allocated with new_pages(), in which case it allocates the page
 *  and returns void. If that is not the case, the user-registered handler (if
 *  any) is called. If the handler is not able to resolve the issue, the kernel
 *  sets the current task's exit status to -2 and kill the faulting thread.   
 *
 *  @param  stack_ptr   The address on the invoking thread's kernel stack where
 *                      we should start constructing the stack for executing 
 *                      the potential user-registered handler 
 *
 *  @return void if the fault is because of the ZFOD system, does not return
 *          otherwise
 */
void page_fault_c_handler(char *stack_ptr) {

  lprintf("\tpage_fault_c_handler(): Page fault @ %p", (void*) get_cr2());

  if (allocate_frame_if_address_requested(get_cr2()) < 0) {

    // Calls the user-registered handler, if any
    create_stack_sw_exception(SWEXN_CAUSE_PAGEFAULT, stack_ptr);

    lprintf("Page fault not handled! Setting status -2 and killing the thread");
    MAGIC_BREAK;

    // Set the task's exit status and kill the thread 
    kern_set_status(EXCEPTION_EXIT_STATUS);
    kern_vanish();
  }
  
  // The page fault was because of the ZFOD system, we can return to user-space

 lprintf("\tpage_fault_c_handler(): The frame allocation was successfull");
  // Re run the instruction

  // Get the page table base register from the register cr3
  //
  // Figure out the correct offset and check if the entry is valid
  // if not, call malloc to create a new page table entry and set the
  // address of the new entry in the page directory and set the valid
  // bit and continue
  //
  // Go at the correct offset in the page table and check whether the
  // entry is valid or not
  // If it is valid, why was there a page fault?
  //
  // Allocate a new frame from the bit map and set it to be allocated
  // copy its address to the page table entry and set the page table entry
  // as valid now
  //
  return;
}
