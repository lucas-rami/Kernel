.global vanish

vanish:
  call save_state
  call kern_vanish
  call restore_state_and_iret
