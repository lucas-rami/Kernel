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

/** @brief  Sets the terminal print color for any future output to the console
 *
 *  @param  color   A color code
 *
 *  @return 0 on success, a negative number if color does not specify a valid
 *          color 
 */
int kern_set_term_color(int color) {
  eff_mutex_lock(&kernel.console_mutex);
  int ret = set_terminal_color(color);
  eff_mutex_unlock(&kernel.console_mutex);
  return ret;
}

/** @brief  Sets the cursor to the location (row, col)
 *
 *  @param  row   The new cursor's row
 *  @param  col   The new cursor's column
 *
 *  @return 0 on success, a negative number if the location is invalid
 */
int kern_set_cursor_pos(int row, int col) {
  eff_mutex_lock(&kernel.console_mutex);
  int ret = set_cursor(row, col);
  eff_mutex_unlock(&kernel.console_mutex);
  return ret;
}

/** @brief  Writes the current location of the cursor to the integers addressed 
 *          by the two arguments
 *
 *  @param  row   An address where to store the cursos's row
 *  @param  col   An address where to store the cursos's column
 *
 *  @return 0 on success, a negative number if either argument is invalid (in
 *          that case the values of both integers are undefined)
 */
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