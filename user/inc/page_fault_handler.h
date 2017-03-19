/** @file page_fault_handler.h
 *
 *  @brief This file contains the declaration of software exception 
 *   handlers for single threaded tasks and multi threaded ones
 *
 *  @author akanjani, lramire1
 */

#ifndef _PAGE_FAULT_HANDLER_H_
#define _PAGE_FAULT_HANDLER_H_

#include <ureg.h>

void singlethread_handler(void* arg, ureg_t *ureg);
void multithread_handler(void* arg, ureg_t *ureg);

#endif /* _PAGE_FAULT_HANDLER_H_ */
