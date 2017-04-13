
.global page_fault_handler


page_fault_handler:
  call save_state
  call page_fault_c_handler
  call restore_state_and_iret_with_errcode
