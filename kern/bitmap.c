#include <bitmap.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int bitmap_init(bitmap_t *map, int length) {
  map->size = length;
  map->arr = (uint8_t*)malloc(sizeof(uint8_t) * length);
  if (map->arr == NULL) {
    return -1;
  }
  memset(map->arr, BITMAP_UNALLOCATED, sizeof(uint8_t) * length);
  return 0;
}

int get_bit(bitmap_t *map, int index) {
  if (!map) {
    return -1;
  }
  int actual_index = (index / BITS_IN_UINT8_T);
  int bit_pos = (index % BITS_IN_UINT8_T);
  int ret = map->arr[actual_index] & (1 << (BITS_IN_UINT8_T - bit_pos - 1));
  return ret;
}

int set_bit(bitmap_t *map, int index) {
  if (!map) {
    return -1;
  }
  int actual_index = (index / BITS_IN_UINT8_T);
  int bit_pos = (index % BITS_IN_UINT8_T);
  map->arr[actual_index] |= (BITMAP_ALLOCATED << (BITS_IN_UINT8_T - bit_pos - 1));
  return 0;
}
