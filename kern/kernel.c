/** @file kernel.c
 *  @brief An initial kernel.c
 *
 *  You should initialize things in kernel_main(),
 *  and then run stuff.
 *
 *  @author Harry Q. Bovik (hqbovik)
 *  @author Fred Hacker (fhacker)
 *  @bug No known bugs.
 */

#include <common_kern.h>

/* libc includes. */
#include <stdio.h>
#include <simics.h>                 /* lprintf() */

/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* x86 specific includes */
#include <x86/asm.h>                /* enable_interrupts() */

#include <interrupts.h>
#include <idt_syscall.h>
#include <console.h>
#include <virtual_memory.h>
#include <task_create.h>
#include <context_switch_asm.h>
#include <eflags.h>
#include <kernel_state.h>
#include <static_queue.h>

#define FIRST_TASK "idle"
#define ESP 0xFFFFFFFF
#define QUEUE_SIZE 32

void tick(unsigned int numTicks);

/** @brief Kernel entrypoint.
 *
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp) {

    // Initialize kernel state
    if (kernel_init() < 0) {
      lprintf("kernel_main(): Failed to initialize kernel state");
      while (1) {
        continue;
      }
    }

    // Initialize the IDT
    handler_install(tick);
    idt_syscall_install();

    // Clear the console before running anything
    clear_console();

    // Enable interrupts
    enable_interrupts();

    // Virtual memory initialize
    if (vm_init() < 0) {
      lprintf("VM init failed\n");
    }

    // Create the initial task and load everything into memory
    uint32_t entrypoint;
    if ( (entrypoint = create_task_executable(FIRST_TASK)) == 0 ) {
      lprintf("Failed to create user task\n");
      while (1) {
        continue;
      }
    }

    // Enable virtual memory
    vm_enable();

    // Create EFLAGS for the user task
    uint32_t eflags = get_eflags();
    eflags |= EFL_RESV1;          // Set bit 1
    eflags &= !EFL_AC;            // Alignment checking off
    eflags &= !EFL_IOPL_RING3;    // Clear current privilege level
    eflags |= EFL_IOPL_RING3;     // Set privilege level to 3
    eflags |= EFL_IF;             // Enable interrupts

    // Run the user task
    run_first_thread(entrypoint, ESP, eflags);

    // We never get here

    while (1) {
        continue;
    }

    return 0;
}

/** @brief Initialize the kernel state
 *
 *  @return 0 on success, a negative number on error
 */
int kernel_init() {

  // Set various fields of the state to their initial value
  kernel.current_thread = NULL;
  kernel.task_id = 0;
  kernel.thread_id = 0;

  // Initialize queue for runnable threads
  if (static_queue_init(&kernel.runnable_threads, QUEUE_SIZE) < 0) {
    lprintf("kernel_init(): Failed to initialize runnable threads queue");
    return -1;
  }

  if (kern_mutex_init(&kernel.mutex) < 0) {
    lprintf("kernel_init(): Failed to initialize mutex");
    return -1;
  }

  // Mark the kernel state as initialized
  kernel.init = KERNEL_INIT_TRUE;

  return 0;

}


/** @brief Tick function, to be called by the timer interrupt handler
 *   This function sets the game_tick variable equal to the numTicks
 *   Also, sets the refresh time which is a flag for the game to update
 *   the time being shown on the screen.
 *
 *  @param numTicks The number of ticks that have occured till now
 *
 *  @return void
 **/
void tick(unsigned int numTicks) {
  numTicks++;
  return;
}
