/** @file task_vanish.S
 *  @brief Stub for task_vanish system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global task_vanish

task_vanish:
	push %esi		# Save the old esi
	movl 8(%esp), %esi	# Copy the first arg to esi
	int $TASK_VANISH_INT	# Make a trap for task_vanish
	pop %esi		# Restore the esi to old value
	ret			# return

