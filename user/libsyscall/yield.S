/** @file yield.S
 *  @brief Stub for yield system call
 *  @author akanjani, lramire1
 */


#include <syscall_int.h>

.global yield

yield:
	push %esi		# Save the old esi
	movl 8(%esp), %esi	# Copy the first arg to esi
	int $YIELD_INT		# Make a trap for yield
	pop %esi		# Restore the esi to old value
	ret			# return
