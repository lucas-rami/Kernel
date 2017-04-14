/** @file get_ticks.c
 *  @brief This file contains the definition for the get_ticks() system call.
 *  @author akanjani, lramire1
 */

#include <timer.h>

unsigned int kern_get_ticks() {
  return get_global_counter();
}
