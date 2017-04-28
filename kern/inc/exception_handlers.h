/** @file exception_handlers.h
 *  @brief  This file contains the declarations for all the exception handlers
 *          nd the function used to register the handlers in the IDT
 *  @author akanjani, lramire1
 */

#ifndef _EXCEPTION_HANDLERS_H_
#define _EXCEPTION_HANDLERS_H_

int exception_handlers_init();

void generic_exception_handler(int cause, char *stack_ptr);
void divide_c_handler(char *stack_ptr);
void debug_c_handler(char *stack_ptr);
void breakpoint_c_handler(char *stack_ptr);
void overflow_c_handler(char *stack_ptr);
void boundcheck_c_handler(char *stack_ptr);
void opcode_c_handler(char *stack_ptr);
void nofpu_c_handler(char *stack_ptr);
void segfault_c_handler(char *stack_ptr);
void stackfault_c_handler(char *stack_ptr);
void protfault_c_handler(char *stack_ptr);
void fpufault_c_handler(char *stack_ptr);
void alignfault_c_handler(char *stack_ptr);
void simdfault_c_handler(char *stack_ptr);

#endif /* _EXCEPTION_HANDLERS_H_ */
