/** @file context_switch_asm.S
 *  @brief This file contains the definitions for assembly functions related
 *  to context switching
 *  @author akanjani, lramire1
 */

#include <cr.h>
#include <seg.h>

.global context_switch_asm
.global run_first_thread
.global run_idle

context_switch_asm:

  /* Get function arguments */
  movl 4(%esp), %ecx
  movl 8(%esp), %edx

  pusha               // Push general purpose registers on stack
  movl %esp, (%ecx)   // Update the 'esp' field in invoking thread's TCB
  movl %edx, %esp     // Restore stack pointer of scheduled thread
  popa                // Pop general purpose registers from stack
  ret                 // Return from procedure as another thread


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

run_idle:
  movl 4(%esp), %ecx
  movl %ecx, %esp
  ret
