/** @file console_io.S
 *  @brief Wrappers for readline(), print() and getchar() system calls
 *  @author akanjani, lramire1
 */

.global readline
.global print
.global getchar

readline:
  
  call save_state
  
  movl 4(%esi), %edx   // Get second parameter
  pushl %edx           // Pass it as a parameter to readline
  movl (%esi), %edx    // Get first parameter
  pushl %edx           // Pass it as a parameter to readline
  call kern_readline
  addl $8, %esp

  call restore_state_and_iret

print:

  call save_state
  
  movl 4(%esi), %edx   // Get second parameter
  pushl %edx           // Pass it as a parameter to print
  movl (%esi), %edx    // Get first parameter
  pushl %edx           // Pass it as a parameter to print
  call kern_print
  addl $8, %esp

  call restore_state_and_iret

getchar:

  call save_state
  call kern_getchar
  call restore_state_and_iret