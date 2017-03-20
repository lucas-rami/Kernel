/** @file context_switch_asm.S
 *  @brief This file contains the definitions for assembly functions related
 *  to context switching
 *  @author akanjani, lramire1
 */

#include <cr.h>
#include <seg.h>

.global context_switch_asm
.global run_first_thread
.global init_new_task

context_switch_asm:

  /* Get function arguments */
  movl 4(%esp), %ecx
  movl 8(%esp), %edx

  pusha               // Push general purpose registers on stack
  movl %esp, (%ecx)   // Update the 'esp' field in invoking thread's TCB
  movl %edx, %esp     // Restore stack pointer of scheduled thread
  popa                // Pop general purpose registers from stack
  ret                 // Return from procedure as another thread

init_new_task:

  movl %esp, %edx     // Save %esp in %edx

  movl 4(%edx), %esp  // Set the stack pointer to the new task's stack

  movl 8(%edx), %ecx  // Push EFLAGS
  pushl %ecx
  movl 12(%edx), %ecx // Push the stack pointer for user stack
  pushl %ecx
  movl 16(%edx), %ecx // Push the entry point address
  pushl %ecx
  movl 20(%edx), %ecx // Push the task's root thread TCB
  pushl %ecx
  movl 24(%edx), %ecx // Push &init_thread
  pushl %ecx
  movl 28(%edx), %ecx // Push &run_first_thread
  pushl %ecx

  pusha               // Push general purpose registers (dummy, here because
                      // context_switch_asm() will pop them)

  movl %esp, %eax     // Save and later return the stack pointer value

  movl %edx, %esp     // Restore original stack pointer value

  ret                 // Return

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
