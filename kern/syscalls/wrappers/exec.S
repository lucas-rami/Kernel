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
  pushl %edx           // Pass it as a parameter to exec
  call kern_exec       // Call kern_exec, the c function for exec
  addl $8, %esp        // Skip the two parameters

  call restore_state_and_iret

switch_esp:
  movl 4(%esp), %edx   // Get the first parameter passed to this function 
  movl %edx, %esp      // Set the stack pointer to the value passed
  ret
