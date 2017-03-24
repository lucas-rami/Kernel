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
#include <static_queue.h>
#include <task_create.h>
#include <virtual_memory.h>

// tmp
#include <cr.h>
#include <eflags.h>

#define FIRST_TASK "idle"

void tick(unsigned int numTicks);

/** @brief Kernel entrypoint.
 *
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp) {

  disable_interrupts();

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

  // Virtual memory initialized
  if (vm_init() < 0) {
    lprintf("VM init failed\n");
  }

  // Create the initial task and load everything into memory
  uint32_t entrypoint;
  if ((entrypoint = create_task_from_executable(FIRST_TASK)) == 0) {
    lprintf("Failed to create user task\n");
    while (1) {
      continue;
    }
  }

  if ((entrypoint = create_task_from_executable(FIRST_TASK)) == 0) {
    lprintf("Failed to create user task\n");
    while (1) {
      continue;
    }
  }

  // Enable virtual memory
  vm_enable();

  // Clear the console before running anything
  clear_console();

  lprintf("Interupts enabled - VM enabled - Ready to run some programs !");

  run_idle(kernel.idle_thread->esp);

  // We never get here

  while (1) {
    continue;
  }

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
