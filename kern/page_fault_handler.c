/** @file page_fault_handler.c
 *  @brief  This file contains the definition for the page fault handler and
 *          the function to register the page fault handler in the IDT
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
#include <string.h>
#include <stdio.h>

/* Index of page fault handler in the IDT */
#define PAGE_FAULT_IDT 0xE
#define NB_REGISTERS_POPA 8

/** @brief  Registers the page fault handler in the IDT
 *
 *  @return 0 on success, a negative number on error
 */
int page_fault_init(void) {
  // Interrupt gate is being used especially for the page fault handler
  // as it gets information like the cr2 value and if a task gate is used
  // that value might change if we get another page fault while handling a
  // page fault. Interrupt gate ensures we don't end up with that race
  // condition in our code
  return register_handler((uintptr_t)page_fault_handler, INTERRUPT_GATE, 
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

  if (allocate_frame_if_address_requested(get_cr2()) < 0) {
    // Calls the user-registered handler, if any
    create_stack_sw_exception(SWEXN_CAUSE_PAGEFAULT, stack_ptr);

    // No exception handler installed
    // Set the task's exit status and kill the thread 
    char err_msg[] = "Vanishing thread due to a PAGE FAULT!";
    kern_print(strlen(err_msg), err_msg);
    kern_set_status(EXCEPTION_EXIT_STATUS);
    kern_vanish();
  }
  
  // The page fault was because of the ZFOD system, we can return to user-space
  return;
}
