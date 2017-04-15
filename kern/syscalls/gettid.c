/** @file gettid.c
 *  @brief This file contains the definition for the kern_gettid() system call.
 *  @author akanjani, lramire1
 */

#include <kernel_state.h>

int kern_gettid() {
  return kernel.current_thread->tid;
}
