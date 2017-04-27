.global divide_handler
.global debug_handler
.global breakpoint_handler
.global overflow_handler
.global boundcheck_handler
.global opcode_handler
.global nofpu_handler
.global segfault_handler
.global stackfault_handler
.global protfault_handler
.global fpufault_handler
.global alignfault_handler
.global simdfault_handler

divide_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call divide_c_handler
  # Should never reach here
  addl $4, %esp
  call restore_state_and_iret_with_errcode

debug_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call debug_c_handler
  # Should never reach here
  addl $4, %esp
  call restore_state_and_iret_with_errcode
  
breakpoint_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call breakpoint_c_handler
  # Should never reach here
  addl $4, %esp
  call restore_state_and_iret_with_errcode

overflow_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call overflow_c_handler
  # Should never reach here
  addl $4, %esp
  call restore_state_and_iret_with_errcode

boundcheck_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call boundcheck_c_handler
  # Should never reach here
  addl $4, %esp
  call restore_state_and_iret_with_errcode

opcode_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call opcode_c_handler
  # Should never reach here
  addl $4, %esp
  call restore_state_and_iret_with_errcode
  
nofpu_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call nofpu_c_handler
  # Should never reach here
  addl $4, %esp
  call restore_state_and_iret_with_errcode

segfault_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call segfault_c_handler
  # Should never reach here
  addl $4, %esp
  call restore_state_and_iret_with_errcode
  
stackfault_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call stackfault_c_handler
  # Should never reach here
  addl $4, %esp
  call restore_state_and_iret_with_errcode

protfault_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call protfault_c_handler
  # Should never reach here
  addl $4, %esp
  call restore_state_and_iret_with_errcode

fpufault_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call fpufault_c_handler
  # Should never reach here
  addl $4, %esp
  call restore_state_and_iret_with_errcode

alignfault_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call alignfault_c_handler
  # Should never reach here
  addl $4, %esp
  call restore_state_and_iret_with_errcode

simdfault_handler:
  call save_state
  movl %esp, %edx
  pushl %edx
  call simdfault_c_handler
  # Should never reach here
  addl $4, %esp
  call restore_state_and_iret_with_errcode
