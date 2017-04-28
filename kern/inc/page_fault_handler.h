/** @file page_fault_handler.h
 *  @brief  This file contains the declarations for the page fault handler
 *          and the function to register it in the IDT
 *  @author akanjani, lramire1
 */

#ifndef _PAGE_FAULT_HANDLER_H_
#define _PAGE_FAULT_HANDLER_H_

int page_fault_init();
void page_fault_c_handler(char *stack_ptr);

#endif /* _PAGE_FAULT_HANDLER_H_ */
