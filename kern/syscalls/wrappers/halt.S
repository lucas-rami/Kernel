/** @file halt.S
 *  @brief Wrapper for halt() system call
 *  @author akanjani, lramire1
 */

.global halt

halt:

  call disable_interrupts       // Disable interrups
  call sim_halt                 // In case we are running in Simics
  hlt                           // In case we are not running in Simics
