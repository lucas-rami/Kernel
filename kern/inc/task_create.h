/** @file task_create.h
 *  @brief This file contains the declarations for functions related to task
 *  creation from executable files
 *  @author akanjani, lramire1
 */

#ifndef _TASK_CREATE_H_
#define _TASK_CREATE_H_

#include <pcb.h>
#include <tcb.h>

int create_task_executable(const char* task_name);
void create_new_pcb();
void create_new_root_tcb();

#endif /* _TASK_CREATE_H_ */
