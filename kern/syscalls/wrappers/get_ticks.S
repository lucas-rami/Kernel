/** @file get_ticks.S
 *  @brief Wrapper for get_ticks() system call
 *  @author akanjani, lramire1
 */

.global get_ticks

get_ticks:

  call save_state
  call kern_get_ticks
  call restore_state_and_iret
