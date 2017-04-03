/** @file exec.S
 *  @brief Wrapper for exec() system call
 *  @author akanjani, lramire1
 */

.global exec
.global switch_esp

exec:

  call save_state

  movl 4(%esi), %edx   // Get second parameter
  pushl %edx           // Pass it as a parameter to exec
  movl (%esi), %edx    // Get first parameter
  pushl %edx
  call kern_exec
  addl $8, %esp

  call restore_state_and_iret

switch_esp:
  movl 4(%esp), %edx
  movl %edx, %esp
  call enable_interrupts
  ret
