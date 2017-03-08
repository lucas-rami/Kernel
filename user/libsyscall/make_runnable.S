/** @file make_runnable.S
 *  @brief Stub for make_runnable system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global make_runnable

make_runnable:
	push %esi		# Save the old esi
	movl 8(%esp), %esi	# Copy the first arg to esi
	int $MAKE_RUNNABLE_INT	# Make a trap for make_runnable
	pop %esi		# Restore the esi to old value
	ret			# return
