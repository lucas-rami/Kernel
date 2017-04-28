/** @file context_switch_asm.h
 *  @brief  This file contains the declarations for assembly functions related
 *          to context switching
 *  @author akanjani, lramire1
 */

#include <tcb.h>

/** @brief  Performs a context switch to another thread
 *
 *  @param  cr3             Value of the cr3 register for the invoking thread
 *  @param  addr_from_esp   Address where to write the new value for the stack
 *                          pointer of the invoking thread
 *  @param  to_esp          Stack pointer for the thread switched to
 *
 *  @return void
 */
void context_switch_asm(uint32_t *addr_from_esp, uint32_t *to_esp);

/** @brief  Runs the first thread in a newly created task
 *
 *  @param  entry_point  Address of the function to first run
 *  @param  esp          Highest address of the thread's stack
 *  @param  eflags       Value for the EFLAGS register
 *
 *  @return void
 */
void run_first_thread(uint32_t entry_point, uint32_t esp, uint32_t eflags);
