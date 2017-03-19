/** @file get_tcb.S
 *  @brief This file contains the definition for the get_esp() function.
 *  @author akanjani, lramire1
 */

.global get_esp

get_esp:
  movl %esp, %eax	# Move esp to eax
  ret			# Return
