/** @file bitmap.h
 *  @brief This file defines a bitmap data structure, as well as functions to
 *    act on the bitmap
 *  @author akanjani, lramire1
 */

#ifndef _BITMAP_H_
#define _BITMAP_H_

#define BITMAP_UNALLOCATED 0
#define BITMAP_ALLOCATED 1
#define BITS_IN_UINT8_T 8

#include <stdint.h>

typedef struct {
  uint8_t *arr;
  int size;
} bitmap_t;

int bitmap_init(bitmap_t *map, int size);
int get_bit(bitmap_t *map, int index);
int set_bit(bitmap_t *map, int index);
int unset_bit(bitmap_t *map, int index);

#endif
