/** @file gettid.S
 *  @brief Wrapper for sys_gettid procedure
 *  @author akanjani, lramire1
 */

.global sys_gettid_wrapper

sys_gettid_wrapper:
  pusha
  call sys_gettid   // call sys_gettid procedure
  popa
  iret			        // return from software interrupt
