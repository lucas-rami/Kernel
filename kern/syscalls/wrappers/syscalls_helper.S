/** @file syscalls_helper.S
 *  @brief This file contains the definitions for assembly functions that are
 *  used by each of the system calls wrappers to save and restore the state of
 *  registers when entering/leaving the kernel.
 *  @author akanjani, lramire1
 */

#include <simics.h>

str:	
	.string	"(%p) = %p. Return value %d"

.global save_state
.global restore_state_and_iret
.global restore_state_and_iret_with_errcode
.global get_esp

save_state:

  pusha                 // Save general purpose registers

  movl 12(%esp), %ecx   // Get the original value of esp before pusha
  movl (%ecx), %ecx     // Get the return address in the wrapper

  pushl %gs             // Push data segment selectors
  pushl %fs
  pushl %es
  pushl %ds

  pushl %ecx            // Push the return address in the wrapper
  ret                   // Return in the wrapper

restore_state_and_iret:

  addl $4, %esp         // Ignore the return address in the wrapper

  popl %ds              // Pop data segment selectors
  popl %es
  popl %fs
  popl %gs
  movl %eax, 28(%esp)   // Replace the value of %eax on the stack

  movl 28(%esp), %edx
  movl 36(%esp), %edi   // DEBUG: edi contains eip in user space
  movl (%edi), %esi     // DEBUG: esi contains the instruction @ edi
  pushl %edx
  pushl %esi
  pushl %edi
  pushl $str
  call custom_print
  popl %edi
  popl %edi
  popl %edi
  popl %edi

  popa                  // Pop general purpose registers

  addl $4, %esp         // Put back the stack pointer to where it was when the
                        // wrapper started

  iret                  // Return from software interrupt

restore_state_and_iret_with_errcode:

  addl $4, %esp         // Ignore the return address in the wrapper

  popl %ds              // Pop data segment selectors
  popl %es
  popl %fs
  popl %gs
  /* DO NOT COPY THE RETURN STATUS OF THE EXCEPTION HANDLER?? */
  // movl %eax, 28(%esp)   // Replace the value of %eax on the stack
  popa                  // Pop general purpose registers

  addl $4, %esp         // Put back the stack pointer to where it was when the
                        // wrapper started

  addl $4, %esp
  iret                  // Return from software interrupt


get_esp:
  movl %esp, %eax
  ret
