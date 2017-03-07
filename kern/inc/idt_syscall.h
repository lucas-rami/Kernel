/** @file idt_syscall.h
 *  @brief This file contains the declarations for functions used to register
 *  the system calls handlers into the IDT.
 *  @author akanjani, lramire1
 */

#ifndef _IDT_SYSCALL_H_
#define _IDT_SYSCALL_H_

int idt_syscall_install();
int register_syscall_handler(int gate_type, unsigned int handler_addr,
                             int indt_index);

#endif /* _IDT_SYSCALL_H_ */
