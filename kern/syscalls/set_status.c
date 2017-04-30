/** @file set_status.c
 *  @brief This file contains the definition for the set_status() system call.
 *  @author akanjani, lramire1
 */

#include <kernel_state.h>
#include <eff_mutex.h>
#include <pcb.h>
#include <atomic_ops.h>

/** @brief  Sets the exit status of the current task
 *
 *  @param  status  The new exit status
 *
 *  @return void
 */
void kern_set_status(int status) {
  atomic_exchange(&kernel.current_thread->task->return_status, status);
}
