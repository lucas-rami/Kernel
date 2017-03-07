/** @file get_cursor_pos.S
 *  @brief Stub for get_cursor_pos system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global get_cursor_pos

get_cursor_pos:
	push %esi		# Save the old esi
	leal 8(%esp), %esi	# Copy the address of the first arg to esi
	int $GET_CURSOR_POS_INT	# Make a trap for get_cursor_pos
	pop %esi		# restore esi
	ret			# return
