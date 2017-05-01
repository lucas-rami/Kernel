/** @file console.c
 *  @brief Implementation of APIs provided to the user to interact with
 *  the console
 *
 *  This file contains the implementation of all console APIs and other 
 *  internal functions needed by those APIs.
 *
 *  The APIs help the user to write something on the console at a particular
 *  offset with the specified color, change the cursor position, hide/unhide
 *  the cursor, change the default color of the words on the screen and 
 *  reading a value from a position on the screen.
 *
 *  @author Anirudh Kanjani
 */

/* The type include */
#include <stdint.h>

/* Includes for the functions used by the APIs */
#include <ctype.h>
#include <asm.h>
#include <console.h>
#include "prechecks.h"
#include <simics.h>

#define CURSOR_INVISIBLE 0
#define CURSOR_VISIBLE 1
#define BLACK_BG_WHITE_FG 0xF
#define LSB_MASK 0xFF
#define SECOND_LSB_MASK 0xFF00
#define BITS_IN_A_BYTE 8
#define FIRST_ROW 0
#define FIRST_COL 0
#define SPACE ' '
#define NEWLINE '\n'
#define CARRIAGE_RETURN '\r'
#define BACKSPACE '\b'
#define NUM_BYTES_PER_POSITION 2

/* The structure to maintain the cursor position */
typedef struct cursor_position {
        int row, col;
} cursor_t;

/* The structure to maintain the cursor state including current position,
 * color and the visibility flag
 */
typedef struct console_state {
	cursor_t global_cursor;
	int term_color;
	uint8_t cursor_isvisible;
} console_state_t;

/* The console_state_t struct type global variable to maintain the state of the
 * cursor
 */
console_state_t state_;

/** @brief The API to print a character on the screen at the current
 *  cursor position.
 *  
 *  Prints character ch at the current location of the cursor. If the 
 *  character is a newline ('\n'), the cursor is moved to the first 
 *  column of the next line (scrolling if necessary). If the character is a 
 *  carriage return ('\r'), the cursor is immediately reset to the 
 *  beginning of the current line( the row in the console buffer where the
 *  current cursor is. Note this line maybe different than a line in the user
 *  context in the cases where the user line wraps around and takes up the 
 *  next line for the console), causing any future output to overwrite 
 *  any existing output on the line. If backspace ('\b') is encountered, the 
 *  previous character is erased. If backspace is called at the beginning of 
 *  a file, this function does not do anything. 
 *
 *  Other than the above cases, if the character is printable, it is displayed
 *  on the screen at the current cursor position.
 *
 *  @return The input character ch
 */
int putbyte( char ch )
{
	int row, col, color;
	get_cursor( &row, &col );
	get_term_color( &color );
	
	if ( ch == NEWLINE ) {
		// change the cursor to the beginning of the next line
		// and scroll up if necessary
		if ( row == CONSOLE_HEIGHT - 1 ) {
			scroll_up();
		} else {
			set_cursor( row + 1, FIRST_COL );

		}
	} else if ( ch == CARRIAGE_RETURN ) {
		// move the cursor to the beginning of the same line
		set_cursor( row, FIRST_COL );
	} else if ( ch == BACKSPACE ) {
		// put ' ' on the column just before the current cursor position
		if ( col == FIRST_COL ) {
			return ch;
		}

		draw_char( row, col - 1, SPACE, color );

		// move the cursor one column to the left
		set_cursor( row, col - 1 ); 
	} else if ( isprint( ch ) ) {
		// put the character at the cursor
		draw_char( row, col, ch, color );
		
		// move the cursor by one to the right
		if ( col == CONSOLE_WIDTH - 1 ) {
			if ( row == CONSOLE_HEIGHT - 1 ) {
				scroll_up();
			} else {
				set_cursor( row + 1, FIRST_COL );
			}
		} else {
			set_cursor( row, col + 1 );
		}
	}
	return ch;
}

/** @brief The API to print a string on the screen at the current
 *  cursor position.
 *  
 *  Prints the character array s, starting at the current location of the 
 *  cursor. If there are more characters than there are spaces remaining 
 *  on the current line, the characters fill up the current line 
 *  and then continue on the next line. If the characters exceed available 
 *  space on the entire console, the screen scrolls up one line, and 
 *  then printing continue on the new line. If '\n', '\r', and '\b' 
 *  are encountered within the string, they are handled as per putbyte.
 *  If len is not a positive integer or s is null, the function has no effect.
 *  When we scroll the screen, we are not going to remember characters 
 *  which are "pushed off" of the screen.
 *
 *  @return Does not return
 */
void putbytes( const char *s, int len )
{
	if ( !is_valid_pointer( s ) || len < 0 ) {
		// invalid string or len
		return;
	}
	int i;
	for ( i = 0; i < len; i++ ) {
		putbyte( s[i] );
	}
}

/** @brief The API to set the color of the characters to be printed on the 
 *  screen.
 *  
 *  Changes the foreground and background color of future characters printed 
 *  on the console. If the color code is invalid, the function has no effect.
 *
 *  @return 0 on success, or -1 if color code is invalid.
 */
int set_terminal_color( int color )
{
	if ( !is_valid_color( color ) ) {
		// invalid color
		return -1;
	}
	state_.term_color = color;
	return 0;
}

/** @brief The function to set the color of the characters that are printed on 
 *  the screen.
 *  
 *  Changes the foreground and background color of the character already 
 *  printed on the console. If the color code or position is invalid, the 
 *  function has no effect.
 *
 *  @return 0 on success, or -1 if color code or the position is invalid.
 */
int set_color_for_pixel( int row, int col, int color )
{
	if ( !is_valid_pixel( row, col ) || !is_valid_color( color ) ) {
		// invalid position or color
		return -1;
	}

	// Change the color at the specified row and col
	*( char * ) ( CONSOLE_MEM_BASE + 
		( row * CONSOLE_WIDTH * NUM_BYTES_PER_POSITION ) + 
		( col * NUM_BYTES_PER_POSITION ) + 1 ) = color;
	return 0;	
}


/** @brief The API to get the color of the characters to be printed on the 
 *  screen.
 *  
 *  Writes the current foreground and background color of characters printed 
 *  on the console into the argument color.
 *
 *  @return void
 */
void get_term_color( int *color )
{
	if ( !is_valid_pointer( color ) ) {
		// invalid input argument
		return;
	}
	*color = state_.term_color;
}


/** @brief The API to set the cursor for the console.
 *
 *  Sets the position of the cursor to the position (row, col). Subsequent 
 *  calls to putbyte or putbytes should cause the console output to begin 
 *  at the new position. If the cursor is currently hidden, a call to 
 *  set_cursor() does not show the cursor.
 *  
 *  @return 0 on success -1 on invalid position
 */
int set_cursor( int row, int col )
{
	if ( !is_valid_pixel( row, col ) ) {
		// invalid position
		return -1;
	}

	// save the state
	state_.global_cursor.row = row;
	state_.global_cursor.col = col;

	// Hanlde the case where the cursor is visible
	if ( state_.cursor_isvisible == CURSOR_VISIBLE ) {
		// Figure out the offset from the screen start
		unsigned int offset = ( row * CONSOLE_WIDTH ) + ( col );

		// Extract the lsb and the second lsb
		uint8_t offset_lsb = ( offset & LSB_MASK );
		uint8_t offset_msb = 
			( offset & SECOND_LSB_MASK ) >> BITS_IN_A_BYTE;

		// Talk to the port
		outb( CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX );
		outb( CRTC_DATA_REG, offset_lsb );

		outb( CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX );
		outb( CRTC_DATA_REG, offset_msb );
	}
	return 0;
}

/** @brief The API to get the current cursor position for the console.
 *
 *  Writes the current position of the cursor into the arguments row and col.
 *  The function doesn't do anything if the pointers passed are invalid.
 *  
 *  @return void
 */
void get_cursor( int *row, int *col )
{
	if ( !is_valid_pointer( row ) || !is_valid_pointer( col ) ) {
		// invalid arguments
		return;
	}

	// extract the cursor position from the maintained state struct
	*row = state_.global_cursor.row;
	*col = state_.global_cursor.col;
}

/** @brief The API to hide the cursor on the console.
 *
 *  Causes the cursor to become invisible, without changing its location. 
 *  Subsequent calls to putbyte or putbytes do not cause the cursor to 
 *  become visible again. If the cursor is already invisible, the function 
 *  has no effect.
 *  
 *  @return void
 */
void hide_cursor()
{
	if ( state_.cursor_isvisible == CURSOR_INVISIBLE ) {
		// The cursor is already hidden. Don't do anything
		return;
	}

	// update the state information maintained by us
	state_.cursor_isvisible = CURSOR_INVISIBLE;

	// update the hardware cursor to a position out of the console
	// region to hide it
	set_cursor( CONSOLE_HEIGHT - 1, CONSOLE_WIDTH );
}

/** @brief The API to show the cursor on the console.
 *
 *  Causes the cursor to become visible, without changing its location. 
 *  If the cursor is already visible, the function has no effect.
 *  
 *  @return void
 */
void show_cursor()
{
	if ( state_.cursor_isvisible == CURSOR_VISIBLE ) {
		// Cursor is already visible. Don't do anything
		return;
	}

	int row, col;

	// extract the current cursor position
	get_cursor( &row, &col );

	// update the state
	state_.cursor_isvisible = CURSOR_VISIBLE;

	// set the hardware cursor so that it shows up on the console
	set_cursor( row, col );
}

/** @brief The API to clear the console.
 *
 *  Clears the entire console and resets the cursor to the home position 
 *  (top left corner). If the cursor is currently hidden it stays hidden.
 *  
 *  @return void
 */
void clear_console()
{
	int i, j;
	for ( i = 0; i < CONSOLE_HEIGHT; i++ ) {
		for( j = 0; j < CONSOLE_WIDTH; j++ ) {
			// put spaces on the whole console
			draw_char( i, j, SPACE, state_.term_color );
		}
	}

	// reset the cursor position to its original state
	set_cursor( FIRST_ROW, FIRST_COL );
}

/** @brief The API to display the character ch on a particular position on
 *  the console.
 *
 *  Prints character ch with the specified color at position (row, col). 
 *  If any argument is invalid, the function has no effect. 
 *  
 *  @return void
 */
void draw_char( int row, int col, int ch, int color )
{
	if ( !is_valid_pixel( row, col ) || !is_valid_color( color ) || 
		!is_valid_char( ch ) ) {
		// invalid arguments
		return;
	}

	// write the character and color on the correct offsets to the base 
	// of the memory mapped IO region
	*( char * ) ( CONSOLE_MEM_BASE + 
		( row * CONSOLE_WIDTH * NUM_BYTES_PER_POSITION ) + 
		( col * NUM_BYTES_PER_POSITION ) ) = ch;

	*( char * ) ( CONSOLE_MEM_BASE + 
		( row * CONSOLE_WIDTH * NUM_BYTES_PER_POSITION ) + 
		( col * NUM_BYTES_PER_POSITION ) + 1 ) = color;

}

/** @brief The API to get the character ch on a particular position on
 *  the console.
 *
 *  Returns the character displayed at position (row, col).
 *  
 *  @return The character at the position (row, col) or -1 if ( row, col ) is 
 *  not on the console
 */
char get_char( int row, int col )
{
	if ( !is_valid_pixel( row, col ) ) {
		// invalid position on the console
		return -1;
	}
	// Read from the memory address
	return *( char * ) ( CONSOLE_MEM_BASE + 
		( row * CONSOLE_WIDTH * NUM_BYTES_PER_POSITION ) + 
		( col * NUM_BYTES_PER_POSITION ) );
}

/** @brief The function to scroll the screen up by 1 line on the console.
 *
 *  Scrolls up the whole screen by 1 line and then sets the cursor position
 *  to the first column of the last line( the new empty line)  on the console
 *  
 *  @return void
 */
void scroll_up()
{
	int i, j;
	for ( i = 1; i < CONSOLE_HEIGHT; i++ ) {
		for ( j = 0; j < CONSOLE_WIDTH; j++ ) {
			// move the character to the same column of the i-1th
			// row
			draw_char( i - 1, j, get_char( i, j ), 
				state_.term_color );
		}
	}
	for ( j = 0; j < CONSOLE_WIDTH; j++ ) {
		// draw spaces on the last line of the console to clean it
		draw_char( i - 1, j, SPACE, state_.term_color );
	}

	// set the cursor to the first column of the last line
	set_cursor( i - 1, FIRST_COL );
}

/** @brief The function to initialize the console.
 *
 *  Clears the console and initializes the state of the console.
 *  Sets the cursor position to (0, 0) and visibility to true and the color
 *  to black background and white foreground 
 *  
 *  @return void
 */
void console_init()
{
	// Clear the console
	clear_console();

	// initialize the state
	state_.global_cursor.row = FIRST_ROW;
	state_.global_cursor.col = FIRST_COL;
	state_.cursor_isvisible = CURSOR_VISIBLE;
	state_.term_color = BLACK_BG_WHITE_FG;
}
