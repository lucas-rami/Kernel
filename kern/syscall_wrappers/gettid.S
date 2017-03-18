/** @file gettid.S
 *  @brief Wrapper for sys_gettid procedure
 *  @author akanjani, lramire1
 */

#include <seg.h>

.global sys_gettid_wrapper

sys_gettid_wrapper:

  call save_state
  call sys_gettid
  call restore_state_and_iret
