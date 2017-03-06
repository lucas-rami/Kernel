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

void tick(unsigned int numTicks);

/** @brief Kernel entrypoint.
 *  
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
    /*
     * When kernel_main() begins, interrupts are DISABLED.
     * You should delete this comment, and enable them --
     * when you are ready.
     */

    lprintf( "Hello from a brand new kernel!" );

    handler_install(tick);

    enable_interrupts();

    // Virtual memory initialize

    // Create the initial task and load everything into memory

    // Enable virtual memory
    
    // Run the task in user mode
    // Call gettid from the task
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
void tick(unsigned int numTicks)
{
  numTicks++;
  return;
}
