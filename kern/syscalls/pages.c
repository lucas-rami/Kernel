/** @file pages.c
 *  @brief This file contains the definition for the kern_new_pages() and
 *  kern_remove_pages() system calls.
 *  @author akanjani, lramire1
 */

#include <page.h>
#include <common_kern.h>
#include <kernel_state.h>
#include <assert.h>

/* VM system */
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <virtual_memory_defines.h>

/* Debugging */
#include <simics.h>

static int check_range_free(unsigned int address, int nb_pages);

/* Temporary */
static int reserve_frames_zfod(void* base, int nb_pages);
static void free_frames_zfod(void* base);

static int reserve_frames_zfod(void* base, int nb_pages) {

  // TODO: register the triple (kernel.current_thread->pcb, base, nb_pages) in 
  // a datastructre 

  // Try to reserve nb_pages 
  mutex_lock(&kernel.mutex);
  if (kernel.free_frame_count < nb_pages) {
    mutex_unlock(&kernel.mutex);
    return -1;
  }
  kernel.free_frame_count -= nb_pages;
  mutex_unlock(&kernel.mutex);

  // Update the total number of frames requested by the invoking thread
  mutex_lock(&kernel.current_thread->mutex);
  kernel.current_thread->num_of_frames_requested += nb_pages;
  mutex_unlock(&kernel.current_thread->mutex);

  // Update the total number of frames requested by the task
  pcb_t * current_pcb = kernel.current_thread->task;
  mutex_lock(&current_pcb->mutex);
  current_pcb->num_of_frames_requested += nb_pages;
  mutex_unlock(&current_pcb->mutex);

  return 0;

}

static void free_frames_zfod(void* base) {

  // TODO: Lookup in the datastructure to see if we can find a matching entry

  // Temporary
  int found_matching_entry = 0;
  int nb_pages_entry = 0;

  if (!found_matching_entry) {
    // Should not happen
    panic("free_frames_zfod(): Unable to find matching entry in ZFOD");
  }

  if (free_frames_range((unsigned int) base, nb_pages_entry) < 0) {
    panic("free_frames_zfod(): Unable to free the frames");    
  }


}


// TODO
int kern_new_pages(void *base, int len) {

  // Check that the 'len' argument is valid
  if (len <= 0 || len % PAGE_SIZE != 0) {
    lprintf("kern_new_pages(): Invalid len argument");
    return -1;
  }

  // Check that the 'base' argument is valid
  if ((unsigned int)base < USER_MEM_START ||
      (unsigned int)base > machine_phys_frames() * PAGE_SIZE ||
      ((unsigned int)base & PAGE_SIZE) != 0) {
    lprintf("kern_new_pages(): Invalid base argument");        
    return -1;
  }

  // Number of pages that should be allocated
  int nb_pages = len / PAGE_SIZE;
  
  // Check that the space requested by the invoking thread is not already 
  // allocated
  if (check_range_free((unsigned int) base, nb_pages) < 0) {
    lprintf("kern_new_pages(): Requested space already allocated in part");        
    return -1;    
  }

  // Reserve nb_pages of ZFOD space
  if (reserve_frames_zfod(base, nb_pages) < 0) {
    lprintf("kern_new_pages(): Failed to reserve ZFOD space");
    return -1;
  }

  return 0;
   
}


int kern_remove_pages(void *base) {
  
  // Check that the 'base' argument is valid
  if ((unsigned int)base < USER_MEM_START ||
      (unsigned int)base > machine_phys_frames() * PAGE_SIZE ||
      ((unsigned int)base & PAGE_SIZE) != 0) {
    lprintf("kern_remove_pages(): Invalid base argument");        
    return -1;
  }

  free_frames_zfod(base);

  return 0;
}

/** @brief Checks that a certain number of pages are unallocated starting from
 *    a given virtual address
 *
 *  @param address    A virtual address
 *  @param nb_pages   The number of pages to check for
 *
 *  @return 0 is the range is fress on the given number of pages, -1 otherwise
 */
static int check_range_free(unsigned int address, int nb_pages) {

  // Get page directory entry address
  unsigned int *page_directory_entry_addr = 
        get_page_directory_addr_with_offset(address);

  int first_entry = 0;

  // If there is no page table associated with this entry, return with an error
  while (!is_entry_present(page_directory_entry_addr)) {
    
    if (first_entry) {
        unsigned int offset = (address & PAGE_TABLE_MASK) >> PAGE_TABLE_RIGHT_SHIFT;
        nb_pages -= (NB_ENTRY_PER_PAGE - offset);
        first_entry = 0;
    } else {
        nb_pages -= NB_ENTRY_PER_PAGE;
    }

    if (nb_pages <= 0) {
      return 0;
    }

    // Get the next entry in the page directory
    ++page_directory_entry_addr;
    if (!((unsigned int)page_directory_entry_addr & PAGE_SIZE)) {
      return -1;
    }

  }
  

  // If the page table still exists, it means that at least one frame that
  // it maps to is allocated
  if (nb_pages >= NB_ENTRY_PER_PAGE) {
    return -1;
  }

  // Get page table address
  unsigned int * page_table_entry_addr = 
            get_page_table_addr(page_directory_entry_addr);

  int i;

  // Iterate over the entries
  for (i = 0 ; i < nb_pages ; ++i) {
    
    if (is_entry_present(page_table_entry_addr)) {
      return -1;
    }
    ++page_table_entry_addr;

  }
  
  return 0;
}

