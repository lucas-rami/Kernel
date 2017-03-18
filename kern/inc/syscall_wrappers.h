/** @file syscall_wrappers.h
 *  @brief This file contains the declarations for the system calls
 *  wrappers functions.
 *  @author akanjani, lramire1
 */

#ifndef _SYSCALLS_WRAPPERS_H_
#define _SYSCALLS_WRAPPERS_H_

int sys_gettid_wrapper();

/* Scheduling related calls */
int sys_yield_wrapper(int tid);
int sys_deschedule_wrapper(int *reject);
int sys_make_runnable_wrapper(int tid);

#endif /* _SYSCALLS_WRAPPERS_H_ */
