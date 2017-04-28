/** @file bitmap.h
 *  @brief This file defines a bitmap data structure, as well as functions to
 *    act on the bitmap
 *  @author akanjani, lramire1
 */

#ifndef _BITMAP_H_
#define _BITMAP_H_

#define BITMAP_UNINITIALIZED 0
#define BITMAP_INITIALIZED 1
#define BITMAP_UNALLOCATED 0
#define BITMAP_ALLOCATED 1
#define BITS_IN_UINT8_T 8

#include <stdint.h>
#include <eff_mutex.h>

/** @brief A structure representing a bitmap */
typedef struct bitmap {

  /** @brief An array storing the actual bitmap */
  uint8_t *arr;
  
  /** @brief The bitmap's size, in bytes*/
  int size;
  
  /** @brief A mutex to make the bitmap implememtation thread-safe */
  eff_mutex_t mp;

  /** @brief Indiactes whether the bitmap is initialized or not */
  int init;

} bitmap_t;

int bitmap_init(bitmap_t *map, int size);
void bitmap_destroy(bitmap_t *map);
int set_bit(bitmap_t *map, int index);
int unset_bit(bitmap_t *map, int index);

#endif
