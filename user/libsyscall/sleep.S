/** @file sleep.S
 *  @brief Stub for sleep system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global sleep

sleep:
	push %esi		# Save the old esi
	movl 8(%esp), %esi	# Copy the first arg to esi
	int $SLEEP_INT		# Make a trap for sleep
	pop %esi		# Restore the esi to old value
	ret			# return
