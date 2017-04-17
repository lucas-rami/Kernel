/** @file bitmap.c
 *  @brief This file contains the definitions of functions used to modify a
 *    bitmap (the functions defined in this file are thread-safe)
 *  @author akanjani, lramire1
 */

#include <bitmap.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <simics.h>

/** @brief Initialize a bitmap
 *
 *  This function must be called only once for a given mipmap. It should be
 *  called before any other function used to manipulate the bitmap.
 *
 *  @param map    A bitmap
 *  @param length The length for the mipmap, in bytes 
 *
 *  @return 0 on success, a negative number on error
 */
int bitmap_init(bitmap_t *map, int length) {
  map->size = length;
  map->arr = (uint8_t*)malloc(sizeof(uint8_t) * length);
  if (map->arr == NULL) {
    return -1;
  }
  if (eff_mutex_init(&map->mp) < 0) {
    free(map->arr);
    return -1;
  }
  memset(map->arr, BITMAP_UNALLOCATED, sizeof(uint8_t) * length);
  return 0;
}

/** @brief Get the value of a particular bit in the mipmap
 *
 *  @param map    A bitmap
 *  @param index  The bit's index
 *
 *  @return The bit's value on success, a negative number on error
 */
int get_bit(bitmap_t *map, int index) {
  if (!map) {
    return -1;
  }

  int actual_index = (index / BITS_IN_UINT8_T);
  int bit_pos = (index % BITS_IN_UINT8_T);
  int ret = map->arr[actual_index] & (1 << (BITS_IN_UINT8_T - bit_pos - 1));
  return ret;
}

/** @brief Set the value of a particular bit in the mipmap
 *
 *  @param map    A bitmap
 *  @param index  The bit's index
 *
 *  @return 0 on success, a negative number on error
 */
int set_bit(bitmap_t *map, int index) {
  if (!map) {
    return -1;
  }

  eff_mutex_lock(&map->mp);
  if (get_bit(map, index) != BITMAP_UNALLOCATED) {
    eff_mutex_unlock(&map->mp);
    return -1;
  }
  int actual_index = (index / BITS_IN_UINT8_T);
  int bit_pos = (index % BITS_IN_UINT8_T);
  map->arr[actual_index] |= 
                        (BITMAP_ALLOCATED << (BITS_IN_UINT8_T - bit_pos - 1));
  eff_mutex_unlock(&map->mp);    
  return 0;
}

/** @brief Unset the value of a particular bit in the mipmap
 *
 *  @param map    A bitmap
 *  @param index  The bit's index
 *
 *  @return 0 on success, a negative number on error
 */
int unset_bit(bitmap_t *map, int index) {
  if (!map) {
    return -1;
  }

  eff_mutex_lock(&map->mp);
  if (get_bit(map, index) <= 0) {
    eff_mutex_unlock(&map->mp);
    return -1;
  }
    
  int actual_index = (index / BITS_IN_UINT8_T);
  int bit_pos = (index % BITS_IN_UINT8_T);
  map->arr[actual_index] &= 
                        (~(BITMAP_ALLOCATED << (BITS_IN_UINT8_T - bit_pos - 1)));
  eff_mutex_unlock(&map->mp);    
  return 0;
}
