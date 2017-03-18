/** @file syscalls_helper.S
 *  @brief This file contains the definitions for assembly functions that are
 *  used by each of the system calls wrappers to save and restore the state of
 *  registers when entering/leaving the kernel.
 *  @author akanjani, lramire1
 */

.global save_state
.global restore_state_and_iret

save_state:

  pusha                 // Save general purpose registers

  movl 12(%esp), %ecx   // Get the original value of esp before pusha
  movl (%ecx), %ecx     // Get the return address in the wrapper

  pushl %ds             // Push data segment selectors
  pushl %es
  pushl %fs
  pushl %gs

  pushl %ecx            // Push the return address in the wrapper
  ret                   // Return in the wrapper

restore_state_and_iret:

  addl $4, %esp         // Ignore the return address in the wrapper

  popl %gs              // Pop data segment selectors
  popl %fs
  popl %es
  popl %ds
  popa                  // Pop general purpose registers

  addl $4, %esp         // Put back the stack pointer to where it was when the
                        // wrapper started

  iret                  // Return from software interrupt
