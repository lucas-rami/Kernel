/** @file atomic_ops.S
 *  @brief  This file contains the definitions for functions that perform
 *          various atomic operations
 *  @author akanjani, lramire1
 */

.global atomic_add_and_update
.global atomic_exchange
.global atomic_compare_and_exchange_32
.global atomic_compare_and_exchange_8

atomic_add_and_update:
  movl 0x4(%esp), %ecx	  // Move the first argument to ecx
  movl 0x8(%esp), %eax	  // Move the second argument to eax
  lock xadd %eax, (%ecx)  // Atomically add the value in eax to
	                  // the memory address pointed to by ecx
                          // and retrieve the old value of it in eax
  ret                     // return


atomic_exchange:
  movl  4(%esp), %edx   // Move address to read to into edx
  movl  8(%esp), %eax   // Move the new value into eax
  xchg  %eax, (%edx)    // Perform the exchange operation
  ret                   // Return from procedure (eax contains the old value)


atomic_compare_and_exchange_32:
  movl 4(%esp), %edx          // edx contains the variable's address
  movl 8(%esp), %eax          // eax contains the old value
  movl 12(%esp), %ecx         // ecx contains the new value
  LOCK cmpxchg %ecx, (%edx)   // if %eax = (%edx) then set (%edx) = %ecx 
  je success32
    movl $0, %eax
    ret
  success32:
    movl $1, %eax
    ret

atomic_compare_and_exchange_8:
  movl 4(%esp), %edx          // edx contains the variable's address
  movb 8(%esp), %al           // eax contains the old value
  movb 12(%esp), %cl           // ecx contains the new value
  LOCK cmpxchg %cl, (%edx)   // if %eax = (%edx) then set (%edx) = %ecx 
  je success8
    movl $0, %eax
    ret
  success8:
    movl $1, %eax
    ret
