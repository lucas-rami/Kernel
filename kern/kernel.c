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
#include <virtual_memory_helper.h>
#include <page_fault_handler.h>
#include <syscalls.h>
#include <assert.h>
#include <string.h>
#include <page.h>

// tmp
#include <cr.h>
#include <eflags.h>

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
  handler_install(wake_up_threads);
  page_fault_init();
 
  if (idt_syscall_install() < 0) {
    lprintf("kernel_main(): Failed to register syscall handlers");
    while (1) {
      continue;
    }
  }

  // Virtual memory initialized
  if (vm_init() < 0) {
    lprintf("VM init failed");
  }

  lprintf("\tkernel_main(): Creating first task");

  kernel.zeroed_out_frame = (unsigned int)allocate_frame();
  lprintf("The zeroed out frame is %p", (char*)kernel.zeroed_out_frame);
  if (kernel.zeroed_out_frame == 0) {
    lprintf("Zeroed out frame couldn't be allocated");
  }
  lprintf("The first int at %p is %p", (char*)kernel.zeroed_out_frame, (char*)(*(unsigned int *)kernel.zeroed_out_frame));
  memset((char*)kernel.zeroed_out_frame, 0, PAGE_SIZE);

  // Create the initial task and load everything into memory
  uint32_t entrypoint;
  if ( (entrypoint = create_task_from_executable(FIRST_TASK, FALSE, NULL, 0)) == 0 ) {
    lprintf("Failed to create user task");
    while (1) {
      continue;
    }
  }

  // Clear the console before running anything
  clear_console();

  // Enable interrupts
  // enable_interrupts();

  lprintf("\tkernel_main(): Running idle thread...");

  // Run the idle thread
  run_idle(kernel.idle_thread->esp);

  // We never get here
  lprintf("kernel_main(): THIS IS REALLY BAD");
  assert(0);

  while (1) {
    continue;
  }

  return 0;
}

