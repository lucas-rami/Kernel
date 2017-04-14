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

/* VM system */
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <virtual_memory_defines.h>

/* Debugging */
#include <simics.h>

/* Constants for synchronization */
#define CONSOLE_IO_INIT_FALSE 0
#define CONSOLE_IO_INIT_TRUE 1
#define LOCKED 0
#define UNLOCKED 1
#define FALSE 0
#define TRUE 1

// TMP
#define MAX_LEN 32

static int read_tmp[MAX_LEN];

static void get_access(mutex_t* mutex, cond_t* cond, int lock);
static void release_lock(mutex_t* mutex, cond_t* cond, int lock);

int kern_readline(int len, char *buf) {
  
  // Synchronization variables
  static mutex_t mutex;
  static cond_t cond;
  static int mutex_initialized = CONSOLE_IO_INIT_FALSE;
  static int lock = UNLOCKED;

  // Synchronization primitives initialization
  if (mutex_initialized == CONSOLE_IO_INIT_FALSE) {
    assert(mutex_init(&mutex) == 0);
    assert(cond_init(&cond) == 0);
    mutex_initialized = CONSOLE_IO_INIT_TRUE;
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


  // Block other threads (no interleaving of readline() calls)
  get_access(&mutex, &cond, lock);

  int i = 0, done = FALSE;
  char c;

  while (done == FALSE) {
    
    while (done == FALSE && (c = readchar()) >= 0) {
    
      if (c == '\b' && i > 0) {
        --i;
        putbyte(c);
      } else if (c == '\r') {
        i = 0;
        putbyte(c);
      } else if (c == '\n') {

        // Commit into buffer and return
        read_tmp[i] = c;
        int min_len = (i + 1 < len) ? i + 1 : len;
        memcpy(buf, &read_tmp, min_len);
        done = TRUE;

      } else if (i < MAX_LEN - 1) {
        read_tmp[i] = c;
        ++i;
        putbyte(c);
      }

    }

    // TODO: add synchronization between keyboard handler and readline()
    // We should deschedule here until the handler signals that other characters
    // are available

  }

  // Allow other threads to run
  release_lock(&mutex, &cond, lock);

  return len;
}

int kern_print(int len, char *buf) {

  // Synchronization variables
  static mutex_t mutex;
  static cond_t cond;
  static int mutex_initialized = CONSOLE_IO_INIT_FALSE;
  static int lock = UNLOCKED;

  // Synchronization primitives initialization
  if (mutex_initialized == CONSOLE_IO_INIT_FALSE) {
    assert(mutex_init(&mutex) == 0);
    assert(cond_init(&cond) == 0);
    mutex_initialized = CONSOLE_IO_INIT_TRUE;
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
  get_access(&mutex, &cond, lock);

  // Print the buffer's content on the console
  int i;
  for (i = 0 ; i < len ; ++i) {
    putbyte(buf[i]);
  }

  // Allow other threads to run
  release_lock(&mutex, &cond, lock);

  return 0;
}

static void get_access(mutex_t* mutex, cond_t* cond, int lock) {
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

