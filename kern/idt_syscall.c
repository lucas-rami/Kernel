/** @file idt_syscall.h
 *  @brief  This file contains the defintions for functions used to register
 *          the system calls handlers into the IDT.
 *  @author akanjani, lramire1
 */

#include <idt_syscall.h>

#include <asm.h>
#include <interrupts.h>
#include <seg.h>

#include <syscall.h>
#include <syscall_int.h>
#include <syscalls.h>

// Debuging
#include <simics.h>

#define IDT_ENTRY_SIZE_BYTES 8 /* The size of an entry in the IDT */

/** @brief  Registers the software interrupt handlers in the IDT
 *
 *  In case of error, the number of handlers registered when the function 
 *  returns is undefined.
 *
 *  @return 0 on success, a negative number of error
 */
int idt_syscall_install() {

  // List of syscalls
  uintptr_t syscalls[] = {(uintptr_t)gettid, (uintptr_t)deschedule,
                          (uintptr_t)make_runnable, (uintptr_t)yield,
                          (uintptr_t)fork, (uintptr_t)exec, 
                          (uintptr_t)thread_fork, 
                          (uintptr_t)new_pages, (uintptr_t)remove_pages,
                          (uintptr_t)readline, (uintptr_t)print,
                          (uintptr_t)swexn,
                          (uintptr_t)vanish, (uintptr_t)wait,
                          (uintptr_t)sleep, (uintptr_t)set_status,
                          (uintptr_t)get_ticks, (uintptr_t)halt,
                          (uintptr_t)readfile, (uintptr_t)set_term_color,
                          (uintptr_t)set_cursor_pos, (uintptr_t)get_cursor_pos
                          };

  // List of offsets in the IDT corresponding to syscalls
  uint32_t idt_indexes[] = {GETTID_INT, DESCHEDULE_INT, MAKE_RUNNABLE_INT,
                            YIELD_INT, FORK_INT, EXEC_INT, THREAD_FORK_INT,
                            NEW_PAGES_INT, REMOVE_PAGES_INT, READLINE_INT, 
                            PRINT_INT, SWEXN_INT, VANISH_INT, WAIT_INT, 
                            SLEEP_INT, SET_STATUS_INT, GET_TICKS_INT, HALT_INT,
                            READFILE_INT, SET_TERM_COLOR_INT, 
                            SET_CURSOR_POS_INT, GET_CURSOR_POS_INT
                            };

  // TODO: create a similar array to differentiate between TRAP and INTERRUPT
  // gates

  int nb_syscalls = sizeof(syscalls) / sizeof(uintptr_t);
  int i;

  // Register the handlers
  for (i = 0; i < nb_syscalls; ++i) {
    if (register_syscall_handler((uint32_t)TRAP_GATE_IDENTIFIER, syscalls[i],
                                 idt_indexes[i]) < 0) {
      lprintf("Failed to register handler %u in IDT",
              (unsigned int)idt_indexes[i]);
      return -1;
    }
  }

  return 0;
}

/** @brief  Registers an handler in the IDT for a software interrupt
 *
 *  @param  gate_type    Type of the interrupt gate
 *  @param  handler_addr Address of the handler function
 *  @param  idt_index    Gate index in the IDT
 *
 *  @return 0 on success, a negative number on error
 */
int register_syscall_handler(uint32_t gate_type, uintptr_t handler_addr,
                             uint32_t idt_index) {

  if (gate_type != TRAP_GATE_IDENTIFIER &&
      gate_type != INTERRUPT_GATE_IDENTIFIER) {
    return -1;
  }

  return register_handler(handler_addr, gate_type, idt_index,
                          USER_PRIVILEGE_LEVEL, SEGSEL_KERNEL_CS);
}
