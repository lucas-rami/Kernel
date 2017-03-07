/** @file exec.S
 *  @brief Stub for exec system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global exec

exec:
	push %esi		# Save the old esi
	leal 8(%esp), %esi	# Copy the address of the first arg to esi
	int $EXEC_INT		# Make a trap for exec
	pop %esi		# Restore the esi to old value
	ret			# return
