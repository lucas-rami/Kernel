/** @file set_term_color.S
 *  @brief Stub for set_term_color system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global set_term_color

set_term_color:
	push %esi		# Save the old esi
	movl 8(%esp), %esi	# Copy the first arg to esi
	int $SET_TERM_COLOR_INT	# Make a trap for set_term_color
	pop %esi		# Restore the esi to old value
	ret			# return
