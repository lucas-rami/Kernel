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

/* VM system */
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <virtual_memory_defines.h>

/* Debugging */
#include <simics.h>

/* Constants for synchronization */
#define PRINT_INIT_FALSE 0
#define PRINT_INIT_TRUE 1
#define LOCKED 0
#define UNLOCKED 1

int kern_readline(int len, char *buf) {
  // TODO
  return -1;
}

int kern_print(int len, char *buf) {

  // Synchronization variables
  static mutex_t mutex;
  static cond_t cond;
  static int mutex_initialized = PRINT_INIT_FALSE;
  static int lock = UNLOCKED;

  // Synchronization primitives initialization
  if (mutex_initialized == PRINT_INIT_FALSE) {
    assert(mutex_init(&mutex) == 0);
    assert(cond_init(&cond) == 0);
    mutex_initialized = PRINT_INIT_TRUE;
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
  mutex_lock(&mutex);
  while (lock == LOCKED) {
    cond_wait(&cond, &mutex);
  }
  lock = LOCKED;
  mutex_unlock(&mutex);

  // Print the buffer's content on the console
  int i;
  for (i = 0 ; i < len ; ++i) {
    putbyte(buf[i]);
  }

  // Allow other threads to run
  mutex_lock(&mutex);
  lock = UNLOCKED;
  cond_signal(&cond);
  mutex_unlock(&mutex);

  return 0;
}