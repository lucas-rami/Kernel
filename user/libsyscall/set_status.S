/** @file set_status.S
 *  @brief Stub for set_status system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global set_status

set_status:
	push %esi		# Save the old esi
	movl 8(%esp), %esi	# Move the argument to esi
	int $SET_STATUS_INT	# Make a trap for set_status
	pop %esi		# Restore the esi to old value
	ret			# return
