/** @file set_status.S
 *  @brief Wrapper for set_status() system call
 *  @author akanjani, lramire1
 */

.global set_status

set_status:

  call save_state

  pushl %esi
  call kern_set_status
  addl $4, %esp
  
  call restore_state_and_iret
