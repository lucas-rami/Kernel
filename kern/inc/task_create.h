/** @file task_create.h
 *  @brief This file contains the declarations for functions related to task
 *  creation from executable files
 *  @author akanjani, lramire1
 */

#ifndef _TASK_CREATE_H_
#define _TASK_CREATE_H_

#include <elf_410.h>

unsigned int create_task_from_executable(char* task_name, int is_exec, char **argv, int count);

unsigned int request_frames_needed_by_program(simple_elf_t *elf);

#endif /* _TASK_CREATE_H_ */
