/** @file kernel.c
 *  @brief This file contains the entrypoint function for the kernel.
 *  @author akanjani, lramire1
 */

#include <common_kern.h>

/* libc includes. */
#include <simics.h> /* lprintf() */
#include <stdio.h>

/* multiboot header file */
#include <multiboot.h> /* boot_info */

/* x86 specific includes */
#include <x86/asm.h> /* enable_interrupts() */

#include <console.h>
#include <context_switch_asm.h>
#include <idt_syscall.h>
#include <interrupts.h>
#include <kernel_state.h>
#include <task_create.h>
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <exception_handlers.h>
#include <syscalls.h>
#include <assert.h>
#include <string.h>
#include <page.h>
#include <exception_handlers.h>
#include <scheduler.h>
#include <keyboard.h>
#include <context_switch.h>
#include <cr.h>

/* Static functions prototypes */
static void idle();

void tick(unsigned int numTicks);

/** @brief  Kernel entrypoint
 *
 *  This is the entrypoint for the kernel.
 *
 *  @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp) {

  disable_interrupts();

  // Initialize kernel state
  if (kernel_init() < 0) {
    lprintf("kernel_main(): Failed to initialize kernel state");
    assert(0);
  }

  // Initialize the IDT
  handler_install(wake_up_threads);
  exception_handlers_init();
 
  if (idt_syscall_install() < 0) {
    lprintf("kernel_main(): Failed to register syscall handlers");
    assert(0);
  }

  // Virtual memory initialized
  if (vm_init() < 0) {
    lprintf("VM init failed");
    assert(0);
  }

  kernel.zeroed_out_frame = (unsigned int) allocate_frame();
  if (kernel.zeroed_out_frame == 0) {
    lprintf("Zeroed out frame couldn't be allocated");
    assert(0);
  }

  memset((char*)kernel.zeroed_out_frame, 0, PAGE_SIZE);

  // Create the initial task and load everything into memory
  if (create_task_from_executable(FIRST_TASK) < 0 ) {
    lprintf("Failed to create user task");
    assert(0);
  }

  // Clear the console before running anything
  clear_console();

  kernel.kernel_ready = KERNEL_READY_TRUE;

  // Run the idle thread
  idle();

  // We never reach here
  assert(0);

  return 0;
}

/** @brief  Idle function for the idle thread
 *
 *  @return Does not return
 */
static void idle() {
  enable_interrupts();
  
  while (1) {
    continue;
  }
}

/** @brief  Main function for the keyboard consumer thread
 *
 *  @return Does not return
 */
void keyboard_consumer() {

  while (1) {

    char ch;
    int row, col;

    // Lock the mutex on the console
    eff_mutex_lock(&kernel.console_mutex);

    // Echo again characters that were not consumed by the previous readline
    int k;
    for ( k = 0 ; k < kernel.rl.key_index ; ++k) {
      putbyte(kernel.rl.key_buf[k]);
    }

    // Unlock the mutex on the console
    eff_mutex_unlock(&kernel.console_mutex);

    do {
      
      // Get the character from the keyboard buffer
      while ((ch = readchar()) < 0) {
        continue;
      }

      // Lock the mutex on the console
      eff_mutex_lock(&kernel.console_mutex);

      if (!(ch == '\b' && kernel.rl.key_index == 0)) {
        // Echo the character to the console
        putbyte(ch);
      }

      switch (ch) {
        case '\b':

          if (kernel.rl.key_index > 0) {
            --kernel.rl.key_index;
          }

          break;
        case '\r':

          get_cursor(&row, &col);
          if (col > kernel.rl.key_index) {
            kernel.rl.key_index = 0;
          } else {
            kernel.rl.key_index -= col;
          }

          break;
        default:

          // Store the character in the key buffer
          if (kernel.rl.key_index < CONSOLE_IO_MAX_LEN) {
            kernel.rl.key_buf[kernel.rl.key_index] = ch;
          }
          ++kernel.rl.key_index;
          
          break;
      }

      // Unlock the mutex on the console
      if (ch != '\n') {
        eff_mutex_unlock(&kernel.console_mutex);
      }

    } while (ch != '\n');

    /* ----- Commit into user buffer ----- */

    // Compute length
    int len = (kernel.rl.len > kernel.rl.key_index) ? 
                kernel.rl.key_index : kernel.rl.len;

    // Fill the user buffer
    uint32_t old_cr3 = get_cr3();
    kernel.current_thread->cr3 = kernel.rl.caller->cr3;
    set_cr3(kernel.current_thread->cr3);

    memcpy(kernel.rl.buf, kernel.rl.key_buf, len);
    
    kernel.current_thread->cr3 = old_cr3;
    set_cr3(old_cr3);


    // Shift remaining characters at beginning of merged buffer
    int i, j;
    int limit = (kernel.rl.key_index < CONSOLE_IO_MAX_LEN) ?
                kernel.rl.key_index : CONSOLE_IO_MAX_LEN;
    for( i = len, j = 0 ; i < limit ; ++i, ++j) {
      kernel.rl.key_buf[j] = kernel.rl.key_buf[i];
    }

    // Reset the index
    kernel.rl.key_index = j;

    // Set the number of characters written in the user buffer
    kernel.rl.len = len;

    // Make the descheduled thread runnable again
    tcb_t * tmp = kernel.rl.caller;
    kernel.rl.caller = NULL;

    eff_mutex_unlock(&kernel.console_mutex);    
    
    disable_interrupts();
    kernel.keyboard_consumer_thread->thread_state = THR_BLOCKED;
    context_switch(tmp);

  }
}
