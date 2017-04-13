
.global page_fault_handler


page_fault_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call page_fault_c_handler
  addl $4, %esp
  call restore_state_and_iret_with_errcode
