/** @file run_first_task.S
 *  @brief This file contains the definition for the run_first_task()
 *  assembly function
 *  @author akanjani, lramire1
 */

.global run_first_task

run_first_task:

  /* Change the value of data segment selectors */

  movw 16(%esp),  %cx   // %cx contains the value for data segment selectors
  movw %ecx,      %ds   // Change the value of %ds
  movw %ecx,      %es   // Change the value of %es
  movw %ecx,      %fs   // Change the value of %fs
  movw %ecx,      %gs   // Change the value of %gs

  /* Store function arguments into registers to initialize the stack */

  movl 4(%esp),   %eax  // %eax contains the thread's entry point
  movl 8(%esp),   %ecx  // %ecx contains the thread's stack pointer
  movl 12(%esp),  %edx  // %edx contains eflags

  /* Craft the stack for the new thread */

  pushw %ss             // SS
  pushl %ecx            // ESP
  pushl %edx            // EFLAGS
  pushw %cs             // CS
  pushl %eax            // EIP
  iret                  // Return from "interrupt"
