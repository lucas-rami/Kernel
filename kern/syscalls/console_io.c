/** @file console_io.c
 *  @brief  This file contains the definition for the kern_readline() and
 *          kern_print() system calls.
 *  @author akanjani, lramire1
 */

#include <eff_mutex.h>
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

/** @brief  Reads the next line from the console and copies it into the buffer 
 *          pointed to by buf
 *
 *  If there is no line of input currently available, the calling thread is 
 *  descheduled until one is. If some other thread is descheduled on a 
 *  readline(), then the calling thread blocks and waits its turn to access the 
 *  input stream. If the line is smaller than the buffer, then the complete line
 *  including the newline character is copied into the buffer. If the length of 
 *  the line exceeds the length of the buffer, only len characters are copied 
 *  into buf.
 *
 *  @param  len     The buffer's length, in bytes (= number of characters)
 *  @param  buf     A buffer where to write the input line 
 *
 *  @return The number of bytes copied into the buffer. An integer error code 
 *          less than zero is returned if buf is not a valid memory address, if 
 *          buf falls in a read-only memory region of the task, or if len is 
 *          larger than the console's size
 */
int kern_readline(int len, char *buf) {

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
  eff_mutex_lock(&kernel.readline_mutex);

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
  eff_mutex_unlock(&kernel.readline_mutex);

  return ret;
}


/** @brief  Prints len bytes of memory, starting at buf, to the console
 *
 *  Outputs of two concurrent call to print() cannot be intermixed. Characters 
 *  printed to the console invoke standard newline, backspace, and scrolling 
 *  behaviors. The calling thread does not continue until all characters have
 *  been printed to the console.
 *
 *  @param  len     The number of bytes of memory to write
 *  @param  buf     The starting memory address
 *
 *  @return 0 on success, an integer code less than zero if len is larger than
 *          the console's size or if buf is not a valid memory address 
 */
int kern_print(int len, char *buf) {
  
  // Check length validity
  if (len < 0 || len > CONSOLE_IO_MAX_LEN) {
    lprintf("print(): Invalid length");
    return -1;
  }

  // Check validity of buffer
  if (is_buffer_valid((unsigned int)buf, len, AT_LEAST_READ) < 0) {
    lprintf("print(): Invalid buffer");     
    return -1;
  }
  
  // Block concurrent threads
  eff_mutex_lock(&kernel.print_mutex);

  // Lock the mutex on the console
  eff_mutex_lock(&kernel.console_mutex);

  // Print the buffer's content on the console
  int i;
  for (i = 0 ; i < len ; ++i) {
    putbyte(buf[i]);
    if (kernel.rl.caller != NULL) {
      if (kernel.rl.key_index < CONSOLE_IO_MAX_LEN) {
      kernel.rl.key_buf[kernel.rl.key_index] = buf[i];
    }
      ++kernel.rl.key_index;
    }
  }

  // Unlock the mutex on the console
  eff_mutex_unlock(&kernel.console_mutex);

  // Allow other threads to run
  eff_mutex_unlock(&kernel.print_mutex);

  return 0;
}

/** @brief  Not implemented
 *  @return -1
 */
int kern_getchar(void) {
  return -1;
}

