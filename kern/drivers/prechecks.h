/** @file prechecks.h
 *  @brief The file which contains the defintions of the helper functions
 *   needed by the drivers to do some sanity prechecks
 *
 *  @author Anirudh Kanjani
 */

#ifndef __PRECHECKS_H_
#define __PRECHECKS_H_

#include <stdint.h>

uint8_t is_valid_pointer( const void *address );

uint8_t is_valid_color( int color );

uint8_t is_valid_pixel( int row, int col );

uint8_t is_valid_char( int ch );

#endif
