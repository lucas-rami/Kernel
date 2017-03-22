/** @file scheduling_calls.S
 *  @brief Wrappers for yield(), deschedule() and make_runnable()
 *  procedures
 *  @author akanjani, lramire1
 */

#include <seg.h>

.global yield
.global deschedule
.global make_runnable

yield:

  call save_state

  pushl %esi
  call kern_yield
  addl $4, %esp

  call restore_state_and_iret

deschedule:

  call save_state

  pushl %esi
  call kern_deschedule
  addl $4, %esp

  call restore_state_and_iret

make_runnable:

  call save_state

  pushl %esi
  call kern_make_runnable
  addl $4, %esp

  call restore_state_and_iret
