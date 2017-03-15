/** @file context_switch_asm.h
 *  @brief This file contains the declarations for assembly functions related
 *  to context switching
 *  @author akanjani, lramire1
 */

#include <stdint.h>

void context_switch_asm(uint32_t cr3, void* addr_from_esp, void* to_esp);
void run_first_thread(uint32_t entry_point, uint32_t esp, uint32_t eflags);
