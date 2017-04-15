/** @file sleep.S
 *  @brief Wrapper for sleep() system call
 *  @author akanjani, lramire1
 */

.global sleep

sleep:

  call save_state

  pushl %esi
  call kern_sleep
  addl $4, %esp
  
  call restore_state_and_iret
