/** @file vanish.S
 *  @brief Stub for vanish system call
 *  @author akanjani, lramire1
 */

#include <syscall_int.h>

.global vanish

vanish:
        int $VANISH_INT		# Make a trap for vanish
        ret                     # return
