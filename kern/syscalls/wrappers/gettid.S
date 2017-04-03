/** @file gettid.S
 *  @brief Wrapper for gettid() system call
 *  @author akanjani, lramire1
 */

.global gettid

gettid:

  call save_state
  call kern_gettid
  call restore_state_and_iret
