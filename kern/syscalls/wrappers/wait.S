/** @file wait.S
 *  @brief Wrapper for wait() system call
 *  @author akanjani, lramire1
 */

.global wait

wait:
  call save_state

  pushl %esi      // Push the first parameter on the stack
  call kern_wait  // Call the c function
  addl $4, %esp   // Update the stack pointer so that iret works fine

  call restore_state_and_iret
