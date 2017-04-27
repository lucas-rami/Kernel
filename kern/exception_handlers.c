#include <idt.h>
#include <syscalls.h>
#include <seg.h>
#include <interrupts.h>
#include <page_fault_handler.h>
#include <exception_handlers.h>
#include <simics.h>
#include "exception_handlers_asm.h"
#include <cr.h>

int exception_handlers_init() {

  if (page_fault_init() < 0) {
    lprintf("Failed to register page fault handler");
    return -1;
  }
  uintptr_t exception_handlers[] = {(uintptr_t)divide_handler, 
                                    (uintptr_t)debug_handler, 
                                    (uintptr_t)breakpoint_handler,
                                    (uintptr_t)overflow_handler,
                                    (uintptr_t)boundcheck_handler,
                                    (uintptr_t)opcode_handler, 
                                    (uintptr_t)nofpu_handler,
                                    (uintptr_t)segfault_handler,
                                    (uintptr_t)stackfault_handler,
                                    (uintptr_t)protfault_handler,
                                    (uintptr_t)fpufault_handler,
                                    (uintptr_t)alignfault_handler,
                                    (uintptr_t)simdfault_handler};

  uint32_t exception_idt_indices[] = {IDT_DE, IDT_DB, IDT_BP, IDT_OF, IDT_BR,
                                      IDT_UD, IDT_NM, IDT_NP, IDT_SS, IDT_GP,
                                      IDT_MF, IDT_AC, IDT_XF};

  int nb_exceptions = sizeof(exception_handlers) / sizeof(uintptr_t);
  int i;
  for (i = 0; i < nb_exceptions; i++) {
    if (register_handler((uintptr_t)exception_handlers[i], TRAP_GATE,
        exception_idt_indices[i], USER_PRIVILEGE_LEVEL, SEGSEL_KERNEL_CS) <0) {
      lprintf("Failed to register %u in IDT", (unsigned int)exception_idt_indices[i]);
      return -1;
    }
  }
  return 0;
}

void generic_exception_handler(int cause, char *stack_ptr) {
  lprintf("Exception other than Page fault. Cause %d at address %p", cause, (char*)get_cr2());
  create_stack_sw_exception(cause, stack_ptr);
  kern_set_status(-2);
  kern_vanish();
}

void divide_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_DIVIDE, stack_ptr);
}

void debug_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_DEBUG, stack_ptr);
}

void breakpoint_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_BREAKPOINT, stack_ptr);
}

void overflow_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_OVERFLOW, stack_ptr);
}

void boundcheck_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_BOUNDCHECK, stack_ptr);
}

void opcode_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_OPCODE, stack_ptr);
}

void nofpu_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_NOFPU, stack_ptr);
}

void segfault_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_SEGFAULT, stack_ptr);
}

void stackfault_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_STACKFAULT, stack_ptr);
}

void protfault_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_PROTFAULT, stack_ptr);
}

void fpufault_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_FPUFAULT, stack_ptr);
}

void alignfault_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_ALIGNFAULT, stack_ptr);
}

void simdfault_c_handler(char *stack_ptr) {
  generic_exception_handler(SWEXN_CAUSE_SIMDFAULT, stack_ptr);
}
