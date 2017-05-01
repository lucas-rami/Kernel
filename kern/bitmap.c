/** @file bitmap.c
 *  @brief This file contains the definitions of functions used to modify a
 *    bitmap (the functions defined in this file are thread-safe)
 *  @author akanjani, lramire1
 */

#include <bitmap.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <simics.h>

/* Static functions prototypes */
static int get_bit(bitmap_t *map, int index);

/** @brief  Initializes a bitmap
 *
 *  This function must be called only once for a given mipmap. It should be
 *  called before any other function used to manipulate the bitmap.
 *
 *  @param  map    A bitmap
 *  @param  length The length for the mipmap, in bytes 
 *
 *  @return 0 on success, a negative number on error
 */
int bitmap_init(bitmap_t *map, int length) {
  
  // Check the arguments
  if (map == NULL || length < 0) {
    return -1;
  }
  
  // Allocate the necessary space for the bitmap
  map->size = length;
  map->arr = malloc(sizeof(uint8_t) * length);
  if (map->arr == NULL) {
    return -1;
  }

  // Initialize the mutex on the bitmap
  if (eff_mutex_init(&map->mp) < 0) {
    free(map->arr);
    return -1;
  }

  // Memset the bitmap to BITMAP_UNALLOCATED
  memset(map->arr, BITMAP_UNALLOCATED, sizeof(uint8_t) * length);

  map->init = BITMAP_INITIALIZED;

  return 0;
}

/** @brief  Destroys a bitmap
 *
 *  The bitmap should have been initialized using bitmap_init() before this 
 *  function is called. After this function has been called, one needs to call 
 *  bitmap_init() again to reuse the bitmap.
 *
 *  @param  map A bitmap
 *  
 *  @return void
 */
void bitmap_destoy(bitmap_t* map) {

  // Check argument
  assert(map != NULL && map->init == BITMAP_INITIALIZED);

  // Free the bitmap
  free(map->arr);

  // Destroy the mutex
  eff_mutex_destroy(&map->mp);

  // Reset status
  map->init = BITMAP_UNINITIALIZED;
}

/** @brief  Gets the value of a particular bit in the mipmap
 *
 *  @param  map    A bitmap
 *  @param  index  The bit's index
 *
 *  @return The bit's value on success, a negative number on error
 */
static int get_bit(bitmap_t *map, int index) {
  
  // Check argument  
  assert(map != NULL && map->init == BITMAP_INITIALIZED);

  int actual_index = (index / BITS_IN_UINT8_T);
  int bit_pos = (index % BITS_IN_UINT8_T);
  int ret = map->arr[actual_index] & (1 << (BITS_IN_UINT8_T - bit_pos - 1));

  return ret;

}

/** @brief  Sets the value of a particular bit in the mipmap
 *
 *  If the bit is already set, then the bitmap is left unchanged.
 *
 *  @param  map    A bitmap
 *  @param  index  The bit's index in the bitmap
 *
 *  @return 0 if the bit was successfully set, a negative number if the bit
 *          was already set 
 */
int set_bit(bitmap_t *map, int index) {
  
  // Check argument  
  assert(map != NULL && map->init == BITMAP_INITIALIZED);

  eff_mutex_lock(&map->mp);
  if (get_bit(map, index) != BITMAP_UNALLOCATED) {
    // If the bit was already set
    eff_mutex_unlock(&map->mp);
    return -1;
  }

  // Set the bit
  int actual_index = (index / BITS_IN_UINT8_T);
  int bit_pos = (index % BITS_IN_UINT8_T);
  map->arr[actual_index] |= 
                        (BITMAP_ALLOCATED << (BITS_IN_UINT8_T - bit_pos - 1));

  eff_mutex_unlock(&map->mp);    
  return 0;

}

/** @brief  Unsets the value of a particular bit in the mipmap
 *
 *  @param  map    A bitmap
 *  @param  index  The bit's index in the bitmap
 *
 *  @return 0 if the bit was successfully unset, a negative number if the bit
 *          was already unset
 */
int unset_bit(bitmap_t *map, int index) {

  // Check argument  
  assert(map != NULL && map->init == BITMAP_INITIALIZED);

  eff_mutex_lock(&map->mp);
  if (get_bit(map, index) <= 0) {
    // If the bit was already unset    
    eff_mutex_unlock(&map->mp);
    return -1;
  }
    
  // Unset the bit
  int actual_index = (index / BITS_IN_UINT8_T);
  int bit_pos = (index % BITS_IN_UINT8_T);
  map->arr[actual_index] &= 
    (~(BITMAP_ALLOCATED << (BITS_IN_UINT8_T - bit_pos - 1)));

  eff_mutex_unlock(&map->mp);    
  return 0;
}
