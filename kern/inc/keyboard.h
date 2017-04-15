/** @file keyboard.h
 *  @brief The file which contains the defintions of the helper functions
 *   needed by the keyboard driver
 *
 *  @author Anirudh Kanjani
 */

#ifndef __KEYBOARD_H_
#define __KEYBOARD_H_

#define CONSOLE_IO_FALSE 0
#define CONSOLE_IO_TRUE 1

int keyboard_init();

void keyboard_c_handler();

/*********************************************************************/
/*                                                                   */
/* Keyboard driver interface                                         */
/*                                                                   */
/*********************************************************************/

/** @brief Returns the next character in the keyboard buffer
 *
 *  This function does not block if there are no characters in the keyboard
 *  buffer
 *
 *  @return The next character in the keyboard buffer, or -1 if the keyboard
 *          buffer is currently empty
 **/
int readchar(void);

#endif

