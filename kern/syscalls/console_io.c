/** @file console_io.c
 *  @brief This file contains the definition for the kern_readline() and
 *  kern_print() system calls.
 *  @author akanjani, lramire1
 */

#include <eff_mutex.h>
#include <mutex.h>
#include <console.h>
#include <assert.h>
#include <common_kern.h>
#include <keyboard.h>
#include <string.h>
#include <stdlib.h>
#include <tcb.h>
#include <kernel_state.h>
#include <asm.h>
#include <scheduler.h>
#include <atomic_ops.h>
#include <syscalls.h>
#include <context_switch.h>

/* VM system */
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <virtual_memory_defines.h>

/* Debugging */
#include <simics.h>

int kern_readline(int len, char *buf) {
  
  // Synchronization primitives
  static eff_mutex_t mutex;
  static int mutex_initialized = CONSOLE_IO_FALSE;

  // Synchronization primitives initialization
  if (atomic_exchange(&mutex_initialized, CONSOLE_IO_TRUE) == CONSOLE_IO_FALSE){
    assert(eff_mutex_init(&mutex) == 0);
  }

  // TODO: buf must fall in non-read-only memory region of the task

  // Check length validity
  if (len < 0 || len > CONSOLE_IO_MAX_LEN) {
    lprintf("readline(): Invalid length");
    return -1;
  }

  // Check validity of buffer
  if (is_buffer_valid((unsigned int)buf, len, READ_WRITE) < 0) {
    lprintf("readline(): Invalid buffer");     
    return -1;
  }

  // Block concurrent threads
  eff_mutex_lock(&mutex);

  // Update readline_t data structure in kernel state
  kernel.rl.buf = buf;
  kernel.rl.len = len;
  kernel.rl.caller = kernel.current_thread;

  // Deschedule myself until a line of input is available
  disable_interrupts();
  kernel.current_thread->thread_state = THR_BLOCKED;
  context_switch(kernel.keyboard_consumer_thread);

  /* buf now contains the line of input
   * kernel.rl.len contains the number of bytes written in the buffer
   * kernel.rl.caller has been reset to NULL */
  
  int ret = kernel.rl.len;   

  // Allow other threads to run
  eff_mutex_unlock(&mutex);

  return ret;
}


int kern_print(int len, char *buf) {

  // Synchronization primitives
  static eff_mutex_t mutex;
  static int mutex_initialized = CONSOLE_IO_FALSE;

  // Synchronization primitives initialization
  if (atomic_exchange(&mutex_initialized, CONSOLE_IO_TRUE) == CONSOLE_IO_FALSE){
    assert(eff_mutex_init(&mutex) == 0);
  }
  
  // Check length validity
  if (len < 0 || len > CONSOLE_IO_MAX_LEN) {
    lprintf("print(): Invalid length");
    return -1;
  }

  // Check validity of buffer
  if (is_buffer_valid((unsigned int)buf, len, READ_ONLY) < 0) {
    lprintf("print(): Invalid buffer");     
    return -1;
  }
  
  // Block concurrent threads
  eff_mutex_lock(&mutex);

  // Lock the mutex on the console
  eff_mutex_lock(&kernel.console_mutex);

  // Print the buffer's content on the console
  int i;
  for (i = 0 ; i < len ; ++i) {
    putbyte(buf[i]);
  }

  // Unlock the mutex on the console
  eff_mutex_unlock(&kernel.console_mutex);

  // Allow other threads to run
  eff_mutex_unlock(&mutex);

  return 0;
}

