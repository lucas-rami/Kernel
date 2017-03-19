/** @file malloc.c
 *  @brief This file contains the definitions for thread safe malloc functions
 *  @author akanjani, lramire1
 */

#include <stdlib.h>
#include <types.h>
#include <stddef.h>
#include <mutex.h>
#include <atomic_ops.h>

/** @brief A macro for 0 being treated as FALSE
 */
#define FALSE 0

/** @brief A macro for 1 being treated as TRUE
 */
#define TRUE 1

/** @brief State of the mutex
 */
static int initialized = FALSE;

/** @brief Global mutex for malloc
 */
static mutex_t alloc_mutex;

/** @brief A thread-safe malloc
 *
 *  @param __size The size to be dynamically allocated
 *
 *  @return void* The pointer to the starting of the allocated space on success
 *                 NULL otherwise
 */
void *malloc(size_t __size) {

  if (initialized == FALSE && atomic_exchange(&initialized, TRUE) == FALSE) {
    // The mutex has not been initialized yet. Initialize it
    mutex_init(&alloc_mutex);
  }

  // Make the malloc call guarded by the mutex
  mutex_lock(&alloc_mutex);
  void* ptr = _malloc(__size);
  mutex_unlock(&alloc_mutex);

  return ptr;
}

/** @brief A thread-safe calloc
 *
 *  @param __nelt Number of elements to allocate
 *  @param __eltsize Size of each element
 *
 *  @return void* The pointer to the starting of the allocated space on success
 *                 NULL otherwise
 */
void *calloc(size_t __nelt, size_t __eltsize) {

  if (initialized == FALSE && atomic_exchange(&initialized, TRUE) == FALSE) {
    // The mutex has not been initialized yet. Initialize it
    mutex_init(&alloc_mutex);
  }

  // Make the calloc call guarded by the mutex
  mutex_lock(&alloc_mutex);
  void* ptr = _calloc(__nelt, __eltsize);
  mutex_unlock(&alloc_mutex);

  return ptr;
}

/** @brief A thread-safe realloc
 *
 *  @param __buf Pointer to a memory block previously allocated with 
 *               malloc, calloc or realloc.
 *  @param __new_size New size for the memory block, in bytes.
 *
 *  @return void* The pointer to the starting of the allocated space on success
 *                if the __new_size > 0, NULL otherwise
 */
void *realloc(void *__buf, size_t __new_size) {

  if (initialized == FALSE && atomic_exchange(&initialized, TRUE) == FALSE) {
    // The mutex has not been initialized yet. Initialize it
    mutex_init(&alloc_mutex);
  }

  // Make the realloc call guarded by the mutex
  mutex_lock(&alloc_mutex);
  void* ptr = _realloc(__buf, __new_size);
  mutex_unlock(&alloc_mutex);

  return ptr;
}

/** @brief A thread-safe free
 *
 *  @param __buf Pointer to a memory block previously allocated with 
 *               malloc, calloc or realloc.
 *
 *  @return void
 */
void free(void *__buf) {

  if (initialized == FALSE && atomic_exchange(&initialized, TRUE) == FALSE) {
    // The mutex has not been initialized yet. Initialize it
    mutex_init(&alloc_mutex);
  }

  // Make the free call guarded by the mutex
  mutex_lock(&alloc_mutex);
  _free(__buf);
  mutex_unlock(&alloc_mutex);
}
