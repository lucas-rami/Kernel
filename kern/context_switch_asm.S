/** @file context_switch_asm.S
 *  @brief This file contains the definitions for assembly functions related
 *  to context switching
 *  @author akanjani, lramire1
 */

#include <cr.h>
#include <seg.h>

.global context_switch_asm
.global run_first_thread

context_switch_asm:

  /* Get function arguments */

  movl 4(%esp), %eax
  movl 8(%esp), %ecx
  movl 12(%esp), %edx

  /* Push thread's current state on stack */

  pusha         // General purpose registers
  pushl %eax    // cr3

  /* Update the 'esp' field in invoking thread's TCB */

  movl %esp, (%ecx)

  /* Restore stack pointer of scheduled thread */

  movl %edx, %esp

  /* Pop next thread's state from stack */

  // Pop out cr3
  popl %ecx
  mov %ecx, %cr3

  // Pop out general purpose registers
  popa

  /* Return from procedure as another thread */
  ret


run_first_thread:

  /* Change the value of data segment selectors */

  movw $SEGSEL_USER_DS, %cx
  movw %cx, %ds   // Change the value of %ds
  movw %cx, %es   // Change the value of %es
  movw %cx, %fs   // Change the value of %fs
  movw %cx, %gs   // Change the value of %gs

  /* Store function arguments into registers to initialize the stack */

  movl 4(%esp),   %eax  // %eax contains the thread's entry point
  movl 8(%esp),   %ecx  // %ecx contains the thread's stack pointer
  movl 12(%esp),  %edx  // %edx contains EFLAGS

  /* Craft the stack for the new thread */

  pushl $SEGSEL_USER_DS             // SS
  pushl %ecx                        // ESP
  pushl %edx                        // EFLAGS
  pushl $SEGSEL_USER_CS             // CS
  pushl %eax                        // EIP
  iret                              // Return from "interrupt"
