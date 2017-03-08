/** @file run_first_task.h
 *  @brief This file contains the declaration for the run_first_task()
 *  assembly function
 *  @author akanjani, lramire1
 */

#ifndef _RUN_FIRST_TASK_H_
#define _RUN_FIRST_TASK_H_

#include <stdint.h>

void run_first_task(uint32_t entry_point, uint32_t esp, uint32_t eflags,
                    uint16_t data_segsel);

#endif /* _RUN_FIRST_TASK_H_ */
