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

  // lprintf("\tkernel_main(): Creating first task");

  kernel.zeroed_out_frame = (unsigned int) allocate_frame();
  lprintf("The zeroed out frame is %p", (char*)kernel.zeroed_out_frame);
  if (kernel.zeroed_out_frame == 0) {
    lprintf("Zeroed out frame couldn't be allocated");
    assert(0);
  }
  // lprintf("The first int at %p is %p", (char*)kernel.zeroed_out_frame, (char*)(*(unsigned int *)kernel.zeroed_out_frame));
  memset((char*)kernel.zeroed_out_frame, 0, PAGE_SIZE);

  // Create the initial task and load everything into memory
  if (create_task_from_executable(FIRST_TASK, NULL, 0) < 0 ) {
    lprintf("Failed to create user task");
    assert(0);
  }

  // Clear the console before running anything
  clear_console();

  lprintf("\tkernel_main(): Running idle thread...");

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
  lprintf("\tidle(): Idle task running");
  enable_interrupts();
  while (1) {
    continue;
  }
}
