/** @file syscalls_helper.h
 *  @brief This file contains the declarations for assembly functions that are
 *  used by each of the system calls wrappers to save and restore the state of
 *  registers when entering/leaving the kernel.
 *  @author akanjani, lramire1
 */

#ifndef _SYSCALLS_HELPER_H_
#define _SYSCALLS_HELPER_H_

/** @brief Save the invoking thread's state when entering a syscall handler
 *
 *  This function pushes the general purpose registers and the data segment
 *  selectors on the stack of the invoking thread. This function should only
 *  be used from within a system call wrapper and should
 *  always be used in combination with the restore_state_and_iret() function
 *  to restore the thread's state. Doing otherwise will lead to incorrect
 *  behavior.
 *
 *  @return void
 */
void save_state();


/** @brief Restore the invoking thread's state and return from interrupt
 *
 *  This function pops the general purpose registers and the data segment
 *  selectors from the stack of the invoking thread. This function should only
 *  be used from within a system call wrapper and should
 *  always be used in combination with the save_state() function
 *  to save the thread's state. Doing otherwise will lead to incorrect
 *  behavior.
 *
 *  @return void
 */
void restore_state_and_iret();

#endif /* _SYSCALLS_HELPER_H_ */
