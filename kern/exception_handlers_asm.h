#ifndef _EXCEPTION_HANDLERS_ASM_H_
#define _EXCEPTION_HANDLERS_ASM_H_

void divide_handler();
void debug_handler();
void breakpoint_handler();
void overflow_handler();
void boundcheck_handler();
void opcode_handler();
void nofpu_handler();
void segfault_handler();
void stackfault_handler();
void protfault_handler();
void fpufault_handler();
void alignfault_handler();
void simdfault_handler();

#endif
