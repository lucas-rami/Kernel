/** @file run_first_task.S
 *  @brief This file contains the definition for the run_first_task()
 *  assembly function
 *  @author akanjani, lramire1
 */

#include <seg.h>

.global run_first_task

run_first_task:

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
