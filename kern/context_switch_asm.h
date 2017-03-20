/** @file context_switch_asm.h
 *  @brief This file contains the declarations for assembly functions related
 *  to context switching
 *  @author akanjani, lramire1
 */

#include <tcb.h>

/** @brief Performs a context switch to another thread
 *
 *  @param cr3            Value of the cr3 register (invoking thread)
 *  @param addr_from_esp  Address where to write the new value for the stack
 *  pointer of the invoking thread
 *  @param to_esp         Stack pointer for the thread switched to
 *
 *  @return void
 */
void context_switch_asm(uint32_t *addr_from_esp, uint32_t to_esp);

/** @brief Craft the stack for the root thread of a newly created task
 *
 *  @param esp_new_task     Stack pointer for the kernel stack of the new task's
 *  root thread
 *  @param eflags           EFLAGS value for new task
 *  @param esp_user         Highest address of user-space stack for new task
 *  @param entry_point      Entry point for new task
 *  @param root_tcb         Pointer to the new task's root thread TCB
 *  @param init_thread      Function pointer to the function to be called after
 *  context_switch_asm()
 *  @param run_first_thread Fucntion pointer to the function to be called after
 *  init_thread()
 *
 *  @return The stack pointer value that should be put in the 'esp' field of
 *  root_tcb
 */
uint32_t init_new_task(uint32_t esp_new_task, uint32_t eflags,
                       uint32_t esp_user, uint32_t entry_point, tcb_t *root_tcb,
                       uintptr_t init_thread, uintptr_t run_first_thread);

/** @brief Run the first thread in a newly created task
 *
 *  TODO: avoid usage for now, not sure it is working
 *  BUG: avoid usage for now, not sure it is working
 *
 *  @param entry_point  Adrress of the function to first run
 *  @param esp          Highest address of the thread's stack
 *  @param eflags       Value for the EFLAGS register
 *
 *  @return void
 */
void run_first_thread(uint32_t entry_point, uint32_t esp, uint32_t eflags);
