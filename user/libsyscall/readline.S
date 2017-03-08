/** @file readline.S
 *  @brief Stub for readline system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global readline

readline:
	push %esi		# Save the old esi
	leal 8(%esp), %esi	# Copy the address of the first arg to esi
	int $READLINE_INT	# Make a trap for readline
	pop %esi		# Restore the esi to old value
	ret			# return
