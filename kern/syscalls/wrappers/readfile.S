/** @file readfile.S
 *  @brief Wrapper for readfile() system call
 *  @author akanjani, lramire1
 */

.global readfile

readfile:

  call save_state

  movl 12(%esi), %edx  // Get forth parameter
  pushl %edx           // Pass it as a parameter to readfile
  movl 8(%esi), %edx   // Get third parameter
  pushl %edx           // Pass it as a parameter to readfile
  movl 4(%esi), %edx   // Get second parameter
  pushl %edx           // Pass it as a parameter to readfile
  movl (%esi), %edx    // Get first parameter
  pushl %edx           // Pass it as a parameter to readfile
  call kern_readfile
  addl $16, %esp

  call restore_state_and_iret
