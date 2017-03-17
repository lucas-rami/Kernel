/** @file atomic_ops.S
 *  @brief This file contains the definitions for functions that perform
 *  different atomic operations
 *  @author akanjani, lramire1
 */

.global atomic_add_and_update
.global atomic_exchange

atomic_add_and_update:
	movl 0x4(%esp), %ecx	  // Move the first argument to ecx
	movl 0x8(%esp), %eax	  // Move the second argument to eax
	lock xadd %eax, (%ecx)	// Atomically add the value in eax to
				                  // the memory address pointed to by ecx
				                  // and retrieve the old value of it in eax
	ret 			              // return


atomic_exchange:
  movl  4(%esp), %edx   // Move address to read to into edx
  movl  8(%esp), %eax   // Move the new value into eax
  xchg  %eax, (%ecx)    // Perform the exchange operation
  ret                   // Return from procedure (eax contains the old value)
