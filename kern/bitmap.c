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
#include <atomic_ops.h>
#include <simics.h>

/* Static functions prototypes */
static uint8_t * get_bit(bitmap_t *map, int index);

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

  // Reset status
  map->init = BITMAP_UNINITIALIZED;
}

/** @brief  Gets the value of a particular bit in the mipmap
 *
 *  @param  map    A bitmap
 *  @param  index  The bit's index
 *
 *  @return The address of the uint8_t containing the bit of interest
 */
uint8_t * get_bit(bitmap_t *map, int index) {
  
  // Check argument  
  assert(map != NULL && map->init == BITMAP_INITIALIZED);

  int actual_index = (index / BITS_IN_UINT8_T);
  return &map->arr[actual_index];

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

  int bit_pos = (index % BITS_IN_UINT8_T);
  uint8_t mask = BITMAP_ALLOCATED << (BITS_IN_UINT8_T - bit_pos - 1);

  // Set the bit
  uint8_t * val_addr = get_bit(map, index);
  uint8_t old_val = *val_addr;
  
  while (!(old_val & mask)) {
    
    uint8_t new_val = old_val | mask;
    if (atomic_compare_and_exchange_8(val_addr, old_val, new_val)) {
      return 0;
    }

    old_val = *val_addr;

  }

  return -1;

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

  int bit_pos = (index % BITS_IN_UINT8_T);
  uint8_t mask = BITMAP_ALLOCATED << (BITS_IN_UINT8_T - bit_pos - 1);  

  uint8_t* val_addr = get_bit(map, index);
  if (!(*val_addr & mask)) {
    return -1;
  }

  *val_addr &= ~mask;

  return 0;
}
