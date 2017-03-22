/** @file fork.S
 *  @brief Wrappers for fork() and thread_fork() system calls
 *  @author akanjani, lramire1
 */

#include <seg.h>

.global fork
.global thread_fork
.global fork_return_new_task

fork:

  call save_state

  movl %esp, %ecx   // Get %esp value
  pushl %ecx        // Pass it as a parameter to fork
  call kern_fork
  addl $4, %esp

  call restore_state_and_iret

thread_fork:

  call save_state
  call thread_fork
  call restore_state_and_iret

fork_return_new_task:

  movl $0, %eax     // Set return value to 0 for new task
  addl $4, %esp     // Restore %esp to where it was when save_state() returned
  call restore_state_and_iret
