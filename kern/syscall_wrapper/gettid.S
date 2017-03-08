/** @file gettid.S
 *  @brief Wrapper for sys_gettid procedure
 *  @author akanjani, lramire1
 */

.global sys_gettid_wrapper

sys_gettid_wrapper:
  call sys_gettid   // call sys_gettid procedure
	iret			        // return from software interrupt
