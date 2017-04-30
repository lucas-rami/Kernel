/** @file get_ticks.c
 *  @brief This file contains the definition for the get_ticks() system call.
 *  @author akanjani, lramire1
 */

#include <timer.h>

/** @brief  Returns the number of timer ticks which have occurred since 
 *          system boot
 *
 *  @return The number of timer ticks since system boot
 */
unsigned int kern_get_ticks() {
  return get_global_counter();
}
