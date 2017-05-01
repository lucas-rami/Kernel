/** @file termianl.c
 *  @brief  This file contains the definition for the kern_set_term_color(),
 *          kern_set_cursor_pos() and kern_get_cursor_pos() system calls. 
 *  @author akanjani, lramire1
 */

#include <kernel_state.h>
#include <console.h>

/* VM system */
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <virtual_memory_defines.h>

int kern_set_term_color(int color) {
  eff_mutex_lock(&kernel.console_mutex);
  int ret = set_terminal_color(color);
  eff_mutex_unlock(&kernel.console_mutex);
  return ret;
}

int kern_set_cursor_pos(int row, int col) {
  eff_mutex_lock(&kernel.console_mutex);
  int ret = set_cursor(row, col);
  eff_mutex_unlock(&kernel.console_mutex);
  return ret;
}

int kern_get_cursor_pos(int *row, int *col) {

  if (is_buffer_valid((unsigned int)row, sizeof(int), READ_WRITE) < 0 ||
      is_buffer_valid((unsigned int)col, sizeof(int), READ_WRITE) < 0) {
    return -1;
  }

  eff_mutex_lock(&kernel.console_mutex);
  get_cursor(row, col);
  eff_mutex_unlock(&kernel.console_mutex);
  return 0;
}