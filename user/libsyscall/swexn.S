/** @file swexn.S
 *  @brief Stub for swexn system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global swexn

swexn:
	push %esi		# Save the old esi
	leal 8(%esp), %esi	# Copy the address of the first arg to esi
	int $SWEXN_INT		# Make a trap for swexn
	pop %esi		# Restore the esi to old value
	ret			# return
