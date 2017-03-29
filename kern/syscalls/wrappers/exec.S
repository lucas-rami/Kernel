.global exec

exec:

  call save_state

  movl %edx, 4(%esi)   // Get first parameter
  pushl %edx           // Pass it as a parameter to fork
  movl %edx, (%esi)    // Get second parameter
  pushl %edx
  call kern_exec
  addl $8, %esp

  call restore_state_and_iret
