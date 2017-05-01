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
#include <cr.h>

/* Panic */
#include <assert.h>

/* Debugging */
#include <simics.h>

/* Static functions prototypes */
static int reserve_frames_zfod(void* base, int nb_pages);
static int free_frames_zfod(void* base);

/** @brief  Allocates new memory to the invoking task, starting at base and 
 *          extending for len bytes
 *
 *  new pages() will fail, returning a negative integer error code, if base is 
 *  not page-aligned, if len is not a positive integral multiple of the system 
 *  page size, if any portion of the region represents memory already in the 
 *  task’s address space, if any portion of the region intersects a part of the 
 *  address space reserved by the kernel, 1 or if the operating system has 
 *  insufficient resources to satisfy the request.
 *
 *  @param  base   The memory starting address
 *  @param  len    The allocation's length (in bytes)
 *
 *  @return 0 on success, a negative number on error
 */
int kern_new_pages(void *base, int len) {

  lprintf("\tkern_new_pages(): Base addr is %p, Length is %d", base, len);
  // Check that the 'len' argument is valid
  if (len <= 0 || len % PAGE_SIZE != 0) {
    lprintf("\tkern_new_pages(): Invalid len argument");
    return -1;
  }

  // Check that the 'base' argument is valid
  if ((unsigned int)base < USER_MEM_START ||
      ((unsigned int)base % PAGE_SIZE) != 0) {
    lprintf("\tkern_new_pages(): Invalid base argument");        
    return -1;
  }

  // Number of pages that should be allocated
  int nb_pages = len / PAGE_SIZE;

  // Reserve nb_pages of ZFOD space
  if (reserve_frames_zfod(base, nb_pages) < 0) {
    lprintf("\tkern_new_pages(): Failed to reserve ZFOD space");
    return -1;
  }

  return 0;
   
}


/** @brief Deallocates a specific memory region 
 *
 *  Deallocates the specified memory region, which must presently be allocated 
 *  as the result of a previous call to new pages() which specified the same 
 *  value of base.
 *
 *  @param  base   The base address of the allocation
 *
 *  @return 0 on success, a negative number on error
 */
int kern_remove_pages(void *base) {
  
  // Check that the 'base' argument is valid
  if ((unsigned int)base < USER_MEM_START ||
      ((unsigned int)base & PAGE_SIZE) != 0) {
    lprintf("\tkern_remove_pages(): Invalid base argument");        
    return -1;
  }

  return free_frames_zfod(base);
}

/** @brief  Reserves a particular number of ZFOD frames at a given address
 *
 *  @param  base     The base address for allocation
 *  @param  nb_pages The number of pages to reserve
 *
 *  @return 0 on success, a negative number on error
 */
static int reserve_frames_zfod(void* base, int nb_pages) {

  // Try to reserve frames
  if (reserve_frames(nb_pages) < 0) {
    return -1;
  }

  // Allocate space for new allocation structure
  alloc_t * new_alloc = malloc(sizeof(alloc_t));
  if (new_alloc == NULL) {
    return -1;
  }
  new_alloc->base = base;
  new_alloc->len = nb_pages;

  // Register the allocation
  pcb_t * current_pcb = kernel.current_thread->task;
  if (linked_list_insert_node(&current_pcb->allocations, new_alloc) < 0) {
    release_frames(nb_pages);
    free(new_alloc);
    lprintf("reserve_frames_zfod(): Registration of new allocation failed");
    return -1;
  }

  // Mark the pages as requested
  if (mark_address_range_requested((unsigned int)base, (unsigned int)nb_pages) < 0) {
    release_frames(nb_pages);
    free(new_alloc);
    linked_list_delete_node(&current_pcb->allocations, new_alloc);
    lprintf("reserve_frames_zfod(): mark_address_range_requested failed");
    return -1;
  }

  // Update the total number of frames requested by the invoking thread
  eff_mutex_lock(&kernel.current_thread->mutex);
  kernel.current_thread->num_of_frames_requested += nb_pages;
  eff_mutex_unlock(&kernel.current_thread->mutex);

  // Update the total number of frames requested by the invoking task
  eff_mutex_lock(&current_pcb->mutex);
  current_pcb->num_of_frames_requested += nb_pages;
  eff_mutex_unlock(&current_pcb->mutex);

  return 0;

}

/** @brief  Frees a memory region previously reserves usign reserve_frames_zfod()
 *
 *  @param  base   The base address that was used during reservation
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

  eff_mutex_lock(&kernel.current_thread->mutex);
  kernel.current_thread->num_of_frames_requested -= (len/PAGE_SIZE);
  eff_mutex_unlock(&kernel.current_thread->mutex);

  eff_mutex_lock(&kernel.current_thread->task->mutex);
  kernel.current_thread->task->num_of_frames_requested -= (len/PAGE_SIZE);
  eff_mutex_unlock(&kernel.current_thread->task->mutex);

  return 0;

}
