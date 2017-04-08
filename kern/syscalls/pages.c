/** @file pages.c
 *  @brief This file contains the definition for the kern_new_pages() and
 *  kern_remove_pages() system calls.
 *  @author akanjani, lramire1
 */

#include <page.h>
#include <common_kern.h>
#include <kernel_state.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>

/* VM system */
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <virtual_memory_defines.h>
#include <assert.h>

/* Panic */
#include <assert.h>

/* Debugging */
#include <simics.h>

/* Prototypes for static functions in this file */
static int reserve_frames_zfod(void* base, int nb_pages);
static int free_frames_zfod(void* base);

/** @brief Allocates new memory to the invoking task, starting at base and 
 *    extending for len bytes
 *
 *  new pages() will fail, returning a negative integer error code, if base is 
 *  not page-aligned, if len is not a positive integral multiple of the system 
 *  page size, if any portion of the region represents memory already in the 
 *  task’s address space, if any portion of the region intersects a part of the 
 *  address space reserved by the kernel, 1 or if the operating system has 
 *  insufficient resources to satisfy the request.
 *
 *  @param base   The memory starting address
 *  @param len    The allocation's length (in bytes)
 *
 *  @return 0 on success, a negative number on error
 */
int kern_new_pages(void *base, int len) {

  lprintf("Base addr is %p, Length is %d", base, len);
  // Check that the 'len' argument is valid
  if (len <= 0 || len % PAGE_SIZE != 0) {
    lprintf("kern_new_pages(): Invalid len argument");
    return -1;
  }

  // Check that the 'base' argument is valid
  if ((unsigned int)base < USER_MEM_START ||
      ((unsigned int)base % PAGE_SIZE) != 0) {
    lprintf("kern_new_pages(): Invalid base argument");        
    return -1;
  }

  // Number of pages that should be allocated
  int nb_pages = len / PAGE_SIZE;

  // Reserve nb_pages of ZFOD space
  if (reserve_frames_zfod(base, nb_pages) < 0) {
    lprintf("kern_new_pages(): Failed to reserve ZFOD space");
    return -1;
  }

  lprintf("kern_new_pages(): returning success on new page");

  return 0;
   
}


/** @brief Deallocates a specific memory region 
 *
 *  Deallocates the specified memory region, which must presently be allocated 
 *  as the result of a previous call to new pages() which specified the same 
 *  value of base.
 *
 *  @param base   The base address of the allocation
 *
 *  @return 0 on success, a negative number on error
 */
int kern_remove_pages(void *base) {
  
  // Check that the 'base' argument is valid
  if ((unsigned int)base < USER_MEM_START ||
      ((unsigned int)base & PAGE_SIZE) != 0) {
    lprintf("kern_remove_pages(): Invalid base argument");        
    return -1;
  }

  return free_frames_zfod(base);
}

/** @brief Reserve a particular number of ZFOD frames at a given address
 *
 *  @param base     The base address for allocation
 *  @param nb_pages The number of pages to reserve
 *
 *  @return 0 on success, a negative number on error
 */
static int reserve_frames_zfod(void* base, int nb_pages) {

  // Try to reserve nb_pages 
  mutex_lock(&kernel.mutex);
  if (kernel.free_frame_count < nb_pages) {
    mutex_unlock(&kernel.mutex);
    return -1;
  }
  kernel.free_frame_count -= nb_pages;
  mutex_unlock(&kernel.mutex);

  // Allocate space for new allocation structure
  alloc_t * new_alloc = malloc(sizeof(alloc_t));
  if (new_alloc == NULL) {
    return -1;
  }
  new_alloc->base = base;
  new_alloc->len = nb_pages * PAGE_SIZE;

  // Mark the pages as requested
  if (mark_adrress_range_requested((unsigned int)base, (unsigned int)nb_pages) < 0) {
    mutex_lock(&kernel.mutex);
    kernel.free_frame_count += nb_pages;  
    mutex_unlock(&kernel.mutex);
    free(new_alloc);
    lprintf("reserve_frames_zfod(): mark_address_range_requested failed");
    return -1;
  }

  // Register the allocation
  pcb_t * current_pcb = kernel.current_thread->task;
  if (linked_list_insert_node(&current_pcb->allocations, new_alloc) < 0) {
    mutex_lock(&kernel.mutex);
    kernel.free_frame_count += nb_pages;  
    mutex_unlock(&kernel.mutex);
    free(new_alloc);
    lprintf("reserve_frames_zfod(): Registration of new allocation failed");
    return -1;
  }

  // Update the total number of frames requested by the invoking thread
  mutex_lock(&kernel.current_thread->mutex);
  kernel.current_thread->num_of_frames_requested += nb_pages;
  mutex_unlock(&kernel.current_thread->mutex);

  // Update the total number of frames requested by the invoking task
  mutex_lock(&current_pcb->mutex);
  current_pcb->num_of_frames_requested += nb_pages;
  mutex_unlock(&current_pcb->mutex);

  return 0;

}

/** @brief Free a memory region previously reserves usign reserve_frames_zfod()
 *
 *  @param base   The base address that was use during reservation
*
 *  @return 0 on success, a negative number on error
 */
static int free_frames_zfod(void* base) {

  // Retrieve the allocation from the linked list
  alloc_t * alloc = 
      linked_list_delete_node(&kernel.current_thread->task->allocations, base);

  if (alloc == NULL) {
    lprintf("Allocation can't be found in linked list");
    return -1;
  }

  int len = alloc->len;
  free(alloc);

  // Free the frames
  free_frames_range((unsigned int) base, len);
  
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
/*static int check_range_free(unsigned int address, int nb_pages) {

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
}*/

