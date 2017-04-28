#include <atomic_ops.h>
#include <cond_var.h>
#include <malloc.h>
#include <malloc_internal.h>
#include <stddef.h>
#include <kernel_state.h>
#include <eff_mutex.h>

void *malloc(size_t size) {
  eff_mutex_lock(&kernel.malloc_mutex);
  void* ret = _malloc(size);
  eff_mutex_unlock(&kernel.malloc_mutex);
  return ret;
}

void *memalign(size_t alignment, size_t size) {
  eff_mutex_lock(&kernel.malloc_mutex);
  void* ret = _memalign(alignment, size);
  eff_mutex_unlock(&kernel.malloc_mutex);
  return ret;
}

void *calloc(size_t nelt, size_t eltsize) {
  eff_mutex_lock(&kernel.malloc_mutex);
  void* ret = _calloc(nelt, eltsize);
  eff_mutex_unlock(&kernel.malloc_mutex);
  return ret;
}

void *realloc(void *buf, size_t new_size) {
  eff_mutex_lock(&kernel.malloc_mutex);
  void* ret = _realloc(buf, new_size);
  eff_mutex_unlock(&kernel.malloc_mutex);
  return ret;
}

void free(void *buf) {
  eff_mutex_lock(&kernel.malloc_mutex);
  _free(buf);
  eff_mutex_unlock(&kernel.malloc_mutex);
}

void *smalloc(size_t size) {
  eff_mutex_lock(&kernel.malloc_mutex);
  void* ret = _smalloc(size);
  eff_mutex_unlock(&kernel.malloc_mutex);
  return ret;
}

void *smemalign(size_t alignment, size_t size) {
  eff_mutex_lock(&kernel.malloc_mutex);
  void* ret = _smemalign(alignment, size);
  eff_mutex_unlock(&kernel.malloc_mutex);
  return ret;
}

void sfree(void *buf, size_t size) {
  eff_mutex_lock(&kernel.malloc_mutex);
  _sfree(buf, size);
  eff_mutex_unlock(&kernel.malloc_mutex);
}
