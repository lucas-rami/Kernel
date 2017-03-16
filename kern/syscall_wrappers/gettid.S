/** @file gettid.S
 *  @brief Wrapper for sys_gettid procedure
 *  @author akanjani, lramire1
 */

#include <seg.h>

.global sys_gettid_wrapper

sys_gettid_wrapper:

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

  call sys_gettid

  /* Pop all registers from stack and return */

  popl %gs
  popl %fs
  popl %es
  popl %ds
  popa

  iret
