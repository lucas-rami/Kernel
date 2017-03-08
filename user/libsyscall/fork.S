/** @file fork.S
 *  @brief Stub for fork system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global fork

fork:
	int $FORK_INT		# Make a trap for fork
	ret			# return
