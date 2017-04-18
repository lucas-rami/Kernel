/** @file swexn.S
 *  @brief This file contains the definition for the swexn() system call 
 *    wrapper.
 *  @author akanjani, lramire1
 */

.global swexn

swexn:

  call save_state

  movl %esp, %edx
  addl $48, %edx
  pushl %edx
  movl 12(%esi), %edx
  pushl %edx
  movl 8(%esi), %edx
  pushl %edx
  movl 4(%esi), %edx
  pushl %edx
  movl (%esi), %edx
  pushl %edx
  call kern_swexn
  addl $20, %esp

  call restore_state_and_iret
