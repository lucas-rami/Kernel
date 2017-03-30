/** @file pages.S
 *  @brief Wrapper for new_pages() and remove_pages() system calls
 *  @author akanjani, lramire1
 */

.global new_pages
.global remove_pages

new_pages:

  call save_state
  
  movl %edx, 4(%esi)   // Get first parameter
  pushl %edx           // Pass it as a parameter to exec
  movl %edx, (%esi)    // Get second parameter
  pushl %edx           // Pass it as a parameter to exec
  call kern_new_pages
  addl $8, %esp

  call restore_state_and_iret

remove_pages

  call save_state

  pushl %esi
  call kern_remove_pages
  addl $4, %esp
  
  call restore_state_and_iret