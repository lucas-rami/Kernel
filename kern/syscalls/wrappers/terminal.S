/** @file terminal.S
 *  @brief  Wrappers for set_term_color(), set_cursor_pos() and
 *          get_cursor_pos() system calls
 *  @author akanjani, lramire1
 */

.global set_term_color
.global set_cursor_pos
.global get_cursor_pos

set_term_color:
  
  call save_state
  
  pushl %esi
  call kern_set_term_color
  addl $4, %esp

  call restore_state_and_iret

set_cursor_pos:

  call save_state
  
  movl 4(%esi), %edx   // Get second parameter
  pushl %edx           // Pass it as a parameter to set_cursor_pos
  movl (%esi), %edx    // Get first parameter
  pushl %edx           // Pass it as a parameter to set_cursor_pos
  call kern_set_cursor_pos
  addl $8, %esp

  call restore_state_and_iret

get_cursor_pos:

  call save_state
  
  movl 4(%esi), %edx   // Get second parameter
  pushl %edx           // Pass it as a parameter to get_cursor_pos
  movl (%esi), %edx    // Get first parameter
  pushl %edx           // Pass it as a parameter to get_cursor_pos
  call kern_get_cursor_pos
  addl $8, %esp

  call restore_state_and_iret
