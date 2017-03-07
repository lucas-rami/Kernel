/** @file misbehave.S
 *  @brief Stub for misbehave system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global misbehave

misbehave:
	push %esi		# Save the old esi
	movl 8(%esp), %esi	# Copy the first arg to esi
	int $MISBEHAVE_INT	# Make a trap for misbehave
	pop %esi		# Restore the esi to old value
	ret			# return

