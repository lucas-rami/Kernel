/** @file getchar.S
 *  @brief Stub for getchar system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global getchar

getchar:
	int $GETCHAR_INT	# Make a trap for getchar
	ret			# return
