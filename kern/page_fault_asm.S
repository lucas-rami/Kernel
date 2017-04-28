/** @file page_fault_asm.S
 *  @brief This file contains the definition for the page fault handler wrapper
 *  @author akanjani, lramire1
 */

.global page_fault_handler

page_fault_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call page_fault_c_handler
  addl $4, %esp
  call restore_state_and_iret_with_errcode
