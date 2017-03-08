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

int idt_syscall_install() {

  // As of now only the gettid() system call handler is registered
  // if (register_syscall_handler(TRAP_GATE_IDENTIFIER, (unsigned int)
  // sys_gettid_wrapper, GETTID_INT) < 0) {
  //   lprintf("Failed to register gettid() handler in IDT");
  //   return -1;
  // }
  if (register_handler((void (*)(void))sys_gettid_wrapper,
                       (uint8_t)TRAP_GATE_IDENTIFIER, (uint32_t)GETTID_INT,
                       (uint8_t)KERNEL_PRIVILEGE_LEVEL,
                       (uint16_t)SEGSEL_KERNEL_CS) < 0) {
    lprintf("Failed to register gettid() handler in IDT");
    return -1;
  }

  return 0;
}

int register_syscall_handler(int gate_type, unsigned int handler_addr,
                             int idt_index) {

  // Check that gate type is correct
  if (gate_type != TRAP_GATE_IDENTIFIER &&
      gate_type != INTERRUPT_GATE_IDENTIFIER) {
    lprintf("Failed to register syscall handler: gate type is neither trap "
            "or interrupt\n");
    return -1;
  }

  // Check that the handler function address is correct
  if (handler_addr >= USER_MEM_START) {
    lprintf("Failed to register syscall handler: handler function address "
            "located in user space\n");
    return -1;
  }

  unsigned int low32 = (SEGSEL_KERNEL_CS << 16) | (handler_addr & TWO_LSB_MASK);
  unsigned int high32 =
      (handler_addr & TWO_MSB_MASK) | (gate_type == TRAP_GATE_IDENTIFIER)
          ? ENTRY_TRAP
          : ENTRY_INTERRUPT;

  unsigned int idt_addr =
      ((unsigned int)idt_base() + IDT_ENTRY_SIZE_BYTES * idt_index);
  *(unsigned int *)idt_addr = low32;
  *(unsigned int *)(idt_addr + IDT_ENTRY_SIZE_BYTES / 2) = high32;

  return 0;
}
