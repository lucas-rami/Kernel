/** @file get_ticks.S
 *  @brief Stub for get_ticks system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global get_ticks

get_ticks:
	int $GET_TICKS_INT	# Make a trap for get_ticks
	ret			# return

