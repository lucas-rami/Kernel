/** @file print.S
 *  @brief Stub for print system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global print

print:
	push %esi		# Save old esi on stack
	leal 0x8(%esp), %esi	# Move the argument to esi
	int $PRINT_INT		# Make a trap for print
	pop %esi		# Restore to old esi
	ret			# return
