#include <atomic_ops.h>
#include <cond_var.h>
#include <malloc.h>
#include <malloc_internal.h>
#include <stddef.h>

#define LOCKED 0
#define UNLOCKED 1

static void get_lock();
static void release_lock();

static int lock = UNLOCKED;
mutex_t mutex_malloc;
cond_t cond_malloc;

void *malloc(size_t size) {
  get_lock();
  void* ret = _malloc(size);
  release_lock();
  return ret;
}

void *memalign(size_t alignment, size_t size) {
  get_lock();
  void* ret = _memalign(alignment, size);
  release_lock();
  return ret;
}

void *calloc(size_t nelt, size_t eltsize) {
  get_lock();
  void* ret = _calloc(nelt, eltsize);
  release_lock();
  return ret;
}

void *realloc(void *buf, size_t new_size) {
  get_lock();
  void* ret = _realloc(buf, new_size);
  release_lock();
  return ret;
}

void free(void *buf) {
  get_lock();
  _free(buf);
  release_lock();
}

void *smalloc(size_t size) {
  get_lock();
  void* ret = _smalloc(size);
  release_lock();
  return ret;
}

void *smemalign(size_t alignment, size_t size) {
  get_lock();
  void* ret = _smemalign(alignment, size);
  release_lock();
  return ret;
}

void sfree(void *buf, size_t size) {
  get_lock();
  _sfree(buf, size);
  release_lock();
}

static void get_lock() {
  mutex_lock(&mutex_malloc);
  while (lock == LOCKED) {
    cond_wait(&cond_malloc, &mutex_malloc);
  }
  lock = LOCKED;
  mutex_unlock(&mutex_malloc);
}

static void release_lock() {
  mutex_lock(&mutex_malloc);
  lock = UNLOCKED;
  cond_signal(&cond_malloc);
  mutex_unlock(&mutex_malloc);
}
