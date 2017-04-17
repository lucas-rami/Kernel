/** @file set_status.c
 *  @brief This file contains the definition for the set_status() system call.
 *  @author akanjani, lramire1
 */

#include <kernel_state.h>
#include <eff_mutex.h>
#include <pcb.h>

void kern_set_status(int status) {
  pcb_t * pcb = kernel.current_thread->task;
  eff_mutex_lock(&pcb->mutex);
  pcb->return_status = status;
  eff_mutex_unlock(&pcb->mutex);
}
