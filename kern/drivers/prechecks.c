/** @file prechecks.c
 *  @brief The prechecks file.
 *
 *  This file contains the implementations of the sanity prechecks that
 *  are used by the functions in the driver library
 *
 *  @author Anirudh Kanjani
 */

#include <console.h>
#include <stdint.h>
#include <video_defines.h>

/** @brief Checks if the pointer passes as an argument is valid.
 *   Valid means the pointer passed is not a NULL pointer
 *
 *  @param address The pointer that needs to be verified
 *   
 *  @return A uint8_t value which is 0 if the validation check fails, or
 *   a positive integer on success
 **/
uint8_t is_valid_pointer( const void *address )
{
	if ( !address ) {
		return 0;
	}
	return 1;
}

/** @brief Checks if the color passed as an argument is valid.
 *   Valid means the color is one of the valid permuatations of the foreground
 *   and the background colors defined in the video_defines.h file
 *
 *  @param color The integer representing the color to be validated
 *   
 *  @return A uint8_t value which is 0 if the validation check fails, or
 *   a positive integer on success
 **/
uint8_t is_valid_color( int color )
{
	if ( ( unsigned ) color > 0x8F ) {
                return 0;
        }
	return 1;
}

/** @brief Checks if the position passed as an argument is valid.
 *   Valid means the position (row, col) lies within
 *   the bounds of our console. The position (CONSOLE_HEIGHT-1, CONSOLE_WIDTH)
 *   is considered valid as that is where the cursor is written when we want
 *   to hide it.
 *
 *  @param row The row of the position on the console
 *  @param col The column of the position on the console
 *   
 *  @return A uint8_t value which is 0 if the validation check fails, or
 *   a positive integer on success
 **/
uint8_t is_valid_pixel( int row, int col )
{
	if ( row < 0 || col < 0 || col >= CONSOLE_WIDTH || 
		row >= CONSOLE_HEIGHT ) {
		if ( !( ( row == CONSOLE_HEIGHT - 1 ) && 
			( col == CONSOLE_WIDTH ) ) ) {
			return 0;
		}
	}
	return 1;
}

/** @brief Checks if the character passed as an argument is valid.
 *   Valid means the character is actually a character in the valid range
 *   i.e. -128 to 127 for 1 byte characters.
 *
 *  @param ch The character that needs to be validated
 *   
 *  @return A uint8_t value which is 0 if the validation check fails, or
 *   a positive integer on success
 **/
uint8_t is_valid_char( int ch )
{
	if ( ( char )ch != ch ) {
		return 0;
	}
	return 1;
}
