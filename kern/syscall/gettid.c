/** @file gettid.c
 *  @brief This file contains the definition for the sys_gettid() system call.
 *  @author akanjani, lramire1
 */

#include <kernel_state.h>

#include <simics.h>

int sys_gettid() {
  lprintf("GETTID");
  return kernel.current_thread->tid;
}
