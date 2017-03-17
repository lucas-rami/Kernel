/** @file syscalls.h
 *  @brief This file contains the declarations for the system calls handler
 *  functions.
 *  @author akanjani, lramire1
 */

#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

int sys_gettid();

/* Scheduling related calls */
int sys_yield(int tid);
int sys_deschedule(int *reject);
int sys_make_runnable(int tid);

#endif /* _SYSCALLS_H_ */
