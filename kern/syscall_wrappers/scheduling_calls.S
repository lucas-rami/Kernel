/** @file scheduling_calls.S
 *  @brief Wrappers for sys_yield, sys_deschedule and sys_make_runnable
 *  procedures
 *  @author akanjani, lramire1
 */

#include <seg.h>

.global sys_yield_wrapper
.global sys_deschedule_wrapper
.global sys_make_runnable_wrapper

sys_yield_wrapper:

  call save_state

  pushl %esi
  call sys_yield
  addl $4, %esp

  call restore_state_and_iret

sys_deschedule_wrapper:

  call save_state

  pushl %esi
  call sys_deschedule
  addl $4, %esp

  call restore_state_and_iret

sys_make_runnable_wrapper:

  call save_state

  pushl %esi
  call sys_make_runnable
  addl $4, %esp

  call restore_state_and_iret
