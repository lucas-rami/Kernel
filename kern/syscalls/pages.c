/** @file pages.c
 *  @brief This file contains the definition for the kern_new_pages() and
 *  kern_remove_pages() system calls.
 *  @author akanjani, lramire1
 */

#include <page.h>
#include <common_kern.h>

/* VM system */
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <virtual_memory_defines.h>

/* Debugging */
#include <simics.h>

// TODO
int kern_new_pages(void *base, int len) {

  // TODO: We need to remember the pair (base, len) for kern_remove_pages() to 
  // work

  // Check that the 'len' argument is valid
  if (len <= 0 || len % PAGE_SIZE != 0) {
    lprintf("kern_new_pages(): Invalid len argument");
    return -1;
  }

  // Check that the 'base' argument is valid
  if ((unsigned int)base < USER_MEM_START ||
      ((unsigned int)base & PAGE_SIZE) != 0) {
    lprintf("kern_new_pages(): Invalid base argument");        
    return -1;
  }

  // Number of pages that should be allocated
  int nb_pages = len / PAGE_SIZE;
  
  // Try to allocate nb_pages new pages
  if (load_multiple_frames((unsigned int) base, nb_pages, SECTION_DATA) < 0) {
    lprintf("kern_new_pages(): Failed to allocate new pages");
    return -1;
  }

  return 0;
   
}


int kern_remove_pages(void *base) {
  // TODO
  return -1;
}
