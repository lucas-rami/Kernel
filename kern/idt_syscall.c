/** @file idt_syscall.h
 *  @brief This file contains the defintions for functions used to register
 *  the system calls handlers into the IDT.
 *  @author akanjani, lramire1
 */

#include <idt_syscall.h>
#include <syscall_wrappers.h>

#include <asm.h>
#include <interrupts.h>
#include <seg.h>

#include <common_kern.h>
#include <syscall.h>
#include <syscall_int.h>

// Debuging
#include <simics.h>

#define IDT_ENTRY_SIZE_BYTES 8 /* The size of an entry in the IDT */
#define ENTRY_INTERRUPT 0x00008E00
#define ENTRY_TRAP 0x00008F00

/** @brief Register the software interrupt handlers in the IDT
 *
 *  @return 0 on success, a negative number of error
 */
int idt_syscall_install() {

  // As of now only the gettid() system call handler is registered
  if (register_syscall_handler((uint8_t)TRAP_GATE_IDENTIFIER,
                               (uintptr_t)sys_gettid_wrapper, GETTID_INT) < 0) {
    lprintf("Failed to register gettid() handler in IDT");
    return -1;
  }

  return 0;
}

/** @brief Register an handler in the IDT for a software interrupt
 *
 *  @return 0 on success, a negative number on error
 */
int register_syscall_handler(uint8_t gate_type, uintptr_t handler_addr,
                             uint32_t idt_index) {

  if (gate_type != TRAP_GATE_IDENTIFIER &&
      gate_type != INTERRUPT_GATE_IDENTIFIER) {
    lprintf("Invalid argument to register_syscall_handler(): Gate type is "
            "neither TRAP nor INTERRUPT");
  }

  return register_handler(handler_addr, gate_type, idt_index,
                          USER_PRIVILEGE_LEVEL, SEGSEL_KERNEL_CS);
}
