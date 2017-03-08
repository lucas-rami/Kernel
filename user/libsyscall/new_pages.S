/** @file new_pages.S
 *  @brief Stub for new_pages system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global new_pages

new_pages:
	push %esi		# Save the old esi
	leal 8(%esp), %esi	# Copy the address of the first arg to esi
	int $NEW_PAGES_INT	# Make a trap for new_pages
	pop %esi		# Restore the esi to old value
	ret			# return
