/** @file gettid.S
 *  @brief Stub for gettid system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global gettid

gettid:
	int $GETTID_INT		# Make a trap for gettid
	ret			# return
