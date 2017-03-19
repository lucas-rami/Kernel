/** @file thread_fork.S
 *  @brief Wrapper for thread fork system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global thread_fork

thread_fork:
  movl 4(%esp), %edx    // Save child's esp in edx
  int $THREAD_FORK_INT  // Make thread_fork system call
  test %eax, %eax       // ZF = 1 if %eax = 0 only
  jne return            // If this is the parent thread, return directly
  movl %edx, %esp       // Otherwise, replace esp with the child's stack address
return:
  ret                   // Return from procedure
