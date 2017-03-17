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

  /* Save all registers on stack */

  pusha
  pushl %ds
  pushl %es
  pushl %fs
  pushl %gs

  /* Set data segment selectors to their "kernel version" */

  movw $SEGSEL_KERNEL_DS, %cx
  movw %cx, %ds
  movw %cx, %es
  movw %cx, %fs
  movw %cx, %gs

  /* Call procedure */

  pushl %esi
  call sys_yield

  /* Pop all registers from stack and return */

  popl %gs
  popl %fs
  popl %es
  popl %ds
  popa

  iret

sys_deschedule_wrapper:

  /* Save all registers on stack */

  pusha
  pushl %ds
  pushl %es
  pushl %fs
  pushl %gs

  /* Set data segment selectors to their "kernel version" */

  movw $SEGSEL_KERNEL_DS, %cx
  movw %cx, %ds
  movw %cx, %es
  movw %cx, %fs
  movw %cx, %gs

  /* Call procedure */

  pushl %esi
  call sys_deschedule

  /* Pop all registers from stack and return */

  popl %gs
  popl %fs
  popl %es
  popl %ds
  popa

  iret

sys_make_runnable_wrapper:

  /* Save all registers on stack */

  pusha
  pushl %ds
  pushl %es
  pushl %fs
  pushl %gs

  /* Set data segment selectors to their "kernel version" */

  movw $SEGSEL_KERNEL_DS, %cx
  movw %cx, %ds
  movw %cx, %es
  movw %cx, %fs
  movw %cx, %gs

  /* Call procedure */

  pushl %esi
  call sys_make_runnable

  /* Pop all registers from stack and return */

  popl %gs
  popl %fs
  popl %es
  popl %ds
  popa

  iret
