/** @file task_create.h
 *  @brief This file contains the declarations for functions related to task
 *  creation from executable files
 *  @author akanjani, lramire1
 */

#ifndef _TASK_CREATE_H_
#define _TASK_CREATE_H_

#include <pcb.h>
#include <tcb.h>

int create_task_from_executable(const char* task_name, int is_exec, char **argv, int count);

#endif /* _TASK_CREATE_H_ */
