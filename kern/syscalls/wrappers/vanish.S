/** @file vanish.S
 *  @brief Wrapper for vanish() system call
 *  @author akanjani, lramire1
 */

.global vanish

vanish:
  call save_state
  call kern_vanish
  call restore_state_and_iret

