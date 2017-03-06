/** @file queue.h
 *  @brief The file which contains the defintions of the helper functions
 *   needed by the kernel to maintain the static buffer of characters 
 *   pressed on the keyboard
 *
 *  @author Anirudh Kanjani
 */

#ifndef __QUEUE_H_
#define __QUEUE_H_

#include <stdint.h>

int enqueue( uint8_t ch );
int dequeue();

#endif
