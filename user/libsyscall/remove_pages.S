/** @file remove_pages.S
 *  @brief Stub for remove_pages system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global remove_pages

remove_pages:
	push %esi		# Save the old esi
	movl 8(%esp), %esi	# Copy the first arg to esi
	int $REMOVE_PAGES_INT	# Make a trap for remove_pages
	pop %esi		# Restore the esi to old value
	ret			# return

