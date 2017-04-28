/** @file   malloc_wrappers.c
 *  @brief  This file contains the definitions for the malloc library wrappers 
 *          functions, which make these calls thread-safe
 *  @author akanjani, lramire1
 */

#include <atomic_ops.h>
#include <cond_var.h>
#include <malloc.h>
#include <malloc_internal.h>
#include <stddef.h>
#include <kernel_state.h>
#include <eff_mutex.h>

/** @brief  Wrapper function for _malloc() (thread-safe)
 *
 *  @param  size  The amount of space to allocate, in bytes
 *
 *  @return The start address of the newly allocated memory on success, NULL
 *          otherwise 
 */
void *malloc(size_t size) {
  eff_mutex_lock(&kernel.malloc_mutex);
  void* ret = _malloc(size);
  eff_mutex_unlock(&kernel.malloc_mutex);
  return ret;
}

/** @brief  Wrapper function for _memalign() (thread-safe)
 *
 *  @param  alignment The alignment to use (must be a multiple of sizeof(void*))
 *  @param  size      The amount of space to allocate, in bytes
 *
 *  @return The start address of the newly allocated memory on success, NULL
 *          otherwise 
 */
void *memalign(size_t alignment, size_t size) {
  eff_mutex_lock(&kernel.malloc_mutex);
  void* ret = _memalign(alignment, size);
  eff_mutex_unlock(&kernel.malloc_mutex);
  return ret;
}

/** @brief  Wrapper function for _calloc() (thread-safe)
 *
 *  @param  nelt    The number of elements
 *  @param  eltsize The amount of space needed per element, in bytes
 *
 *  @return The start address of the newly allocated memory on success, NULL
 *          otherwise 
 */
void *calloc(size_t nelt, size_t eltsize) {
  eff_mutex_lock(&kernel.malloc_mutex);
  void* ret = _calloc(nelt, eltsize);
  eff_mutex_unlock(&kernel.malloc_mutex);
  return ret;
}

/** @brief  Wrapper function for _realloc() (thread-safe)
 *
 *  @param  buf       The starting address of previously dynamically allocated
 *                    memory using malloc(), realloc() or memalign()
 *  @param  new_size  The resized amount of space, in bytes
 *
 *  @return The start address of the newly allocated memory on success, NULL
 *          otherwise 
 */
void *realloc(void *buf, size_t new_size) {
  eff_mutex_lock(&kernel.malloc_mutex);
  void* ret = _realloc(buf, new_size);
  eff_mutex_unlock(&kernel.malloc_mutex);
  return ret;
}

/** @brief  Wrapper function for _free() (thread-safe)
 *
 *  @param  buf       The starting address of previously dynamically allocated
 *                    memory using malloc(), realloc() or memalign()
 *  @return void
 */
void free(void *buf) {
  eff_mutex_lock(&kernel.malloc_mutex);
  _free(buf);
  eff_mutex_unlock(&kernel.malloc_mutex);
}

/** @brief  Wrapper function for _smalloc() (thread-safe)
 *
 *  @param  size  The amount of space to allocate, in bytes
 *
 *  @return The start address of the newly allocated memory on success, NULL
 *          otherwise 
 */
void *smalloc(size_t size) {
  eff_mutex_lock(&kernel.malloc_mutex);
  void* ret = _smalloc(size);
  eff_mutex_unlock(&kernel.malloc_mutex);
  return ret;
}

/** @brief  Wrapper function for _smemalign() (thread-safe)
 *
 *  @param  alignment The alignment to use (must be a multiple of sizeof(void*))
 *  @param  size      The amount of space to allocate, in bytes
 *
 *  @return The start address of the newly allocated memory on success, NULL
 *          otherwise 
 */
void *smemalign(size_t alignment, size_t size) {
  eff_mutex_lock(&kernel.malloc_mutex);
  void* ret = _smemalign(alignment, size);
  eff_mutex_unlock(&kernel.malloc_mutex);
  return ret;
}

/** @brief  Wrapper function for _sfree() (thread-safe)
 *
 *  @param  buf       The starting address of previously dynamically allocated
 *                    memory using smalloc() or smemalign()
 *  @param  size      The amount of memory that was allocated, in bytes
 *
 *  @return void
 */
void sfree(void *buf, size_t size) {
  eff_mutex_lock(&kernel.malloc_mutex);
  _sfree(buf, size);
  eff_mutex_unlock(&kernel.malloc_mutex);
}
