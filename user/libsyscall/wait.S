/** @file wait.S
 *  @brief Stub for wait system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global wait

wait:
	push %esi		# Save the old esi
	movl 8(%esp), %esi	# Copy the first arg to esi
	int $WAIT_INT		# Make a trap for wait
	pop %esi		# Restore the esi to old value
	ret			# return
