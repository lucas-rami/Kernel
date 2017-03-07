/** @file deschedule.S
 *  @brief Stub for deschedule system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global deschedule

deschedule:
	push %esi		# Save esi on stack
	movl 8(%esp), %esi	# Copy the first arg to esi
	int $DESCHEDULE_INT	# Make a trap for deschedule
	pop %esi		# restore esi
	ret			# return
