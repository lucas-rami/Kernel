/** @file console_io.c
 *  @brief This file contains the definition for the kern_readline() and
 *  kern_print() system calls.
 *  @author akanjani, lramire1
 */

#include <mutex.h>
#include <console.h>
#include <assert.h>
#include <cond_var.h>
#include <common_kern.h>
#include <keyboard.h>
#include <string.h>
#include <stdlib.h>
#include <tcb.h>
#include <kernel_state.h>
#include <asm.h>
#include <scheduler.h>

/* VM system */
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <virtual_memory_defines.h>

/* Debugging */
#include <simics.h>

/* Constants for synchronization */
#define LOCKED 0
#define UNLOCKED 1

// TMP
#define MAX_LEN 64

int input_available = CONSOLE_IO_FALSE;
tcb_t* waiting_input_tcb = NULL;

static void get_lock(mutex_t* mutex, cond_t* cond, int lock);
static void release_lock(mutex_t* mutex, cond_t* cond, int lock);

int kern_readline(int len, char *buf) {
  
  // Synchronization variables
  static mutex_t mutex;
  static cond_t cond;
  static int mutex_initialized = CONSOLE_IO_FALSE;
  static int lock = UNLOCKED;

  static int read_tmp[MAX_LEN];
  static int i = 0;

  // Synchronization primitives initialization
  if (mutex_initialized == CONSOLE_IO_FALSE) {
    assert(mutex_init(&mutex) == 0);
    assert(cond_init(&cond) == 0);
    mutex_initialized = CONSOLE_IO_TRUE;
  }

  // TODO: "reasonable" maximum bound on len ? Console size ?
  // TODO: buf must fall in non-read-only memory region of the task

  // Check length validity
  if (len < 0) {
    lprintf("readline(): Invalid length");
    return -1;
  }

  // Check validity of buffer
  if ((unsigned int) buf < USER_MEM_START ||
      is_buffer_valid((unsigned int)buf, len) < 0) {
    lprintf("readline(): Invalid buffer");     
    return -1;
  }


  // Block other threads (no interleaving of readline() calls)
  get_lock(&mutex, &cond, lock);

  int done = CONSOLE_IO_FALSE, min_len = len;
  char c;

  while (done == CONSOLE_IO_FALSE) {
    
    while (done == CONSOLE_IO_FALSE && (c = readchar()) >= 0) {
    
      if (c == '\b' && i > 0) {
        --i;
      } else if (c == '\r') {
        i = 0;
      } else if (c == '\n') {
        // Commit into buffer and return
        if (i < MAX_LEN) {
          read_tmp[i] = c;
          ++i;
        }
        min_len = (i < len) ? i : len;
        memcpy(buf, read_tmp, min_len);
        done = CONSOLE_IO_TRUE;

        // Shift remaining elements to beginning of array for next calls to
        // readline()
        int nb_remaining = MAX_LEN - i;
        memcpy(read_tmp, &read_tmp[MAX_LEN - nb_remaining], nb_remaining);

      } else if (i < MAX_LEN - 1) {
        read_tmp[i] = c;
        ++i;
      }

      // Echo the character on the console
      putbyte(c);

      // Clear the input_available flag
      input_available = CONSOLE_IO_FALSE;
    }

    // Synchronization with keyboard handler
    if (done == CONSOLE_IO_FALSE) {
      disable_interrupts();
      if (input_available == CONSOLE_IO_FALSE) {
        waiting_input_tcb = kernel.current_thread;
        block_and_switch(HOLDING_MUTEX_FALSE);
        waiting_input_tcb = NULL;
      } else {
        enable_interrupts();
      }
    }

  }

  // Allow other threads to run
  release_lock(&mutex, &cond, lock);

  return min_len;
}

int kern_print(int len, char *buf) {

  // Synchronization variables
  static mutex_t mutex;
  static cond_t cond;
  static int mutex_initialized = CONSOLE_IO_FALSE;
  static int lock = UNLOCKED;

  // Synchronization primitives initialization
  if (mutex_initialized == CONSOLE_IO_FALSE) {
    assert(mutex_init(&mutex) == 0);
    assert(cond_init(&cond) == 0);
    mutex_initialized = CONSOLE_IO_TRUE;
  }

  // TODO: "reasonable" maximum bound on len ? Console size ?
  
  // Check length validity
  if (len < 0) {
    lprintf("print(): Invalid length");
    return -1;
  }

  // Check validity of buffer
  if ((unsigned int) buf < USER_MEM_START ||
      is_buffer_valid((unsigned int)buf, len) < 0) {
    lprintf("print(): Invalid buffer");     
    return -1;
  }

  // Block other threads (no interleaving of print() outputs)
  get_lock(&mutex, &cond, lock);

  // Print the buffer's content on the console
  int i;
  for (i = 0 ; i < len ; ++i) {
    putbyte(buf[i]);
  }

  // Allow other threads to run
  release_lock(&mutex, &cond, lock);

  return 0;
}

static void get_lock(mutex_t* mutex, cond_t* cond, int lock) {
  mutex_lock(mutex);
  while (lock == LOCKED) {
    cond_wait(cond, mutex);
  }
  lock = LOCKED;
  mutex_unlock(mutex);
}

static void release_lock(mutex_t* mutex, cond_t* cond, int lock) {
  mutex_lock(mutex);
  lock = UNLOCKED;
  cond_signal(cond);
  mutex_unlock(mutex);
}

