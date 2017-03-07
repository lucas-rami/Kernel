/** @file halt.S
 *  @brief Stub for halt system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global halt

halt:
	int $HALT_INT		# Make a trap for halt
	ret			# return
