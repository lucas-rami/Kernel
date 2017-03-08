/** @file set_cursor_pos.S
 *  @brief Stub for set_cursor_pos system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global set_cursor_pos

set_cursor_pos:
	push %esi		# Save the old esi
	leal 8(%esp), %esi	# Copy the address of the first arg to esi
	int $SET_CURSOR_POS_INT	# Make a trap for set_cursor_pos
	pop %esi		# Restore the esi to old value
	ret			# return
