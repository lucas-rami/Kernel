/** @file readfile.S
 *  @brief Stub for readfile system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global readfile

readfile:
	push %esi		# Save the old esi
	leal 8(%esp), %esi	# Copy the address of the first arg to esi
	int $READFILE_INT	# Make a trap for readfile
	pop %esi		# Restore the esi to old value
	ret			# return
