/* Test new_pages() and remove_pages() */

#include <syscall.h>
#include <simics.h>
#include <malloc.h>
#include <stdlib.h>

static void loop(int ret);
static int malloc_without_write();

int main() {

  if (malloc_without_write() < 0) {
    lprintf("new_remove_without_write failed");
    loop(-1);
  }

  loop(0);
  
}

static int malloc_without_write() {
 
  void* alloc1 = malloc(PAGE_SIZE);

  lprintf("malloc_without_write(): First malloc() returned");

  if (alloc1 == NULL) {
    lprintf("malloc_without_write(): First malloc failed");
    return -1;    
  }

  void* alloc2 = calloc(10, PAGE_SIZE);

  lprintf("malloc_without_write(): Second malloc() returned");

  if (alloc2 == NULL) {
    lprintf("malloc_without_write(): malloc_without_write(): Second malloc failed");
    return -1;
  }

  free(alloc1);
  free(alloc2);

  return 0;
}

static void loop(int ret) {
  if (ret == 0) {
    lprintf("pages_alloc_test() completed successfully !");
  } else {
    lprintf("pages_alloc_test() failed !");    
  }
  while(1);
}