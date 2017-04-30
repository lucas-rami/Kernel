/** @file swexn.S
 *  @brief Wrapper for swexn() system call
 *  @author akanjani, lramire1
 */

.global swexn

swexn:

  call save_state

  movl 12(%esi), %edx
  pushl %edx
  movl 8(%esi), %edx
  pushl %edx
  movl 4(%esi), %edx
  pushl %edx
  movl (%esi), %edx
  pushl %edx
  call kern_swexn
  addl $16, %esp

  call restore_state_and_iret

