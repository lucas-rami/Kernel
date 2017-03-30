/** @file virtual_memory.c
 *  @brief This file contains the definitions for functions manipulating
 *  virtual memory
 *  @author akanjani, lramire1
 */

#include <bitmap.h>
#include <common_kern.h>
#include <cr.h>
#include <elf_410.h>
#include <loader.h>
#include <page.h>

/* Standard library */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* VM system */
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <virtual_memory_defines.h>

/* Debugging */
#include <simics.h>

/* Hold the number of user frames in the system */
unsigned int num_user_frames;
/* Bitmap holding the set of (un)allocated frames */ 
bitmap_t free_map;

/** @brief Initialize the virtual memory system
 *
 *  @return 0 on success, a negative number on error
 */
int vm_init() {

  // Figure out the number of user frames
  num_user_frames = machine_phys_frames() - NUM_KERNEL_FRAMES;

  int size = (num_user_frames / BITS_IN_UINT8_T) + 1;

  bitmap_init(&free_map, size);

  return 0;
}

/** @brief Setup the virtual memory for a single task
 *
 *  @param elf_info Data strucure holding the important features
 *  of the task's ELF header
 *
 *  @return The page table directory address on success, NULL on error
 */
unsigned int *setup_vm(const simple_elf_t *elf_info) {

  unsigned int *page_table_directory =
      (unsigned int *)smemalign(PAGE_SIZE, PAGE_SIZE);
  memset(page_table_directory, 0, PAGE_SIZE);

  // set cr3 to this value;
  set_cr3((uint32_t)page_table_directory);

  // Load kernel section
  int i;
  for (i = 0; i < USER_MEM_START; i += PAGE_SIZE) {
    load_frame(i, SECTION_KERNEL);
  }

  // Add stack area as well.
  if (load_every_segment(elf_info) < 0) {
    // TODO: free all the frames
    return NULL;
  }

  return page_table_directory;
}

/** @brief Load every segment of a task into virtual memory
 *
 *  @param elf_info Data strucure holding the important features
 *  of the task's ELF header
 *
 *  @return 0 on success, a negative number on error
 */
int load_every_segment(const simple_elf_t *elf) {
  load_segment(elf->e_fname, elf->e_txtoff, elf->e_txtlen, elf->e_txtstart,
               SECTION_TXT);
  load_segment(elf->e_fname, elf->e_datoff, elf->e_datlen, elf->e_datstart,
               SECTION_DATA);
  load_segment(elf->e_fname, elf->e_rodatoff, elf->e_rodatlen,
               elf->e_rodatstart, SECTION_RODATA);
  load_segment(elf->e_fname, 0, elf->e_bsslen, elf->e_bssstart, SECTION_BSS);
  load_segment(NULL, 0, STACK_SIZE, STACK_START_ADDR, SECTION_STACK);
  // Initialize the global variables and bss to zero once they are allocated
  // space for
  return 0;
}

/** @brief Load one segment of a task into virtual memory
 *
 *  @param fname Task's filename
 *  @param offset Offset in the file where the segment is located
 *  @param size Size of the segment in the file
 *  @param start_addr Starting address of segment in virtual memory
 *  @param type Segment's type
 *
 *  @return 0 on success, a negative number on error
 */
int load_segment(const char *fname, unsigned long offset, unsigned long size,
                 unsigned long start_addr, int type) {

  // get page table base register
  // assume it is base_addr

  char *buf;
  if (type != SECTION_STACK && type != SECTION_BSS) {
    buf = (char *)malloc(sizeof(char) * size);
    if (getbytes(fname, offset, size, buf) < 0) {
      return -1;
    }
  }
  unsigned int curr_offset = 0, remaining_size = size, addr = start_addr;
  int max_size = PAGE_SIZE;
  uint8_t *frame_addr = NULL;
  while (curr_offset < size) {
    if (remaining_size <= PAGE_SIZE) {
      max_size = remaining_size;
    }
    frame_addr = NULL;
    frame_addr = load_frame(addr, type);

    if (frame_addr == NULL) {
      return -1;
    }
    int temp_offset = ((unsigned int)frame_addr % PAGE_SIZE);
    unsigned int size_allocated = ((PAGE_SIZE - temp_offset) < max_size)
                                      ? (PAGE_SIZE - temp_offset)
                                      : max_size;

    if (type != SECTION_STACK && type != SECTION_BSS) {
      memcpy((frame_addr), buf + curr_offset, size_allocated);
    }
    if (type == SECTION_BSS) {
      memset(frame_addr, 0, size_allocated);
    }
    remaining_size -= size_allocated;
    curr_offset += size_allocated;
    addr += size_allocated;
  }

  if (type != SECTION_STACK && type != SECTION_BSS) {
    free(buf);
  }

  return 0;
}

/** @brief Get the physical address associated with a virtual address, if the
 *    page directory/table entries for this virtual address do not exist yet, 
 *    create them 
 *
 *  @param address A virtual address
 *  @param type The address's type (e.g. code, data...)
 *
 *  @return 0 The physical address associated with the virtual one
 */
void *load_frame(unsigned int address, unsigned int type) {

  unsigned int *page_directory_entry_addr = 
        get_page_directory_addr_with_offset(address);

  // If there is no page table associated with this entry, create it
  if (!is_entry_present(page_directory_entry_addr)) {
    create_page_table(page_directory_entry_addr, PAGE_TABLE_FLAGS);
  }

  unsigned int *page_table_entry = 
        get_page_table_addr_with_offset(page_directory_entry_addr, address);

  // If there is no physical frame associated with this entry, allocate it
  if (!is_entry_present(page_table_entry)) {
    if (type != SECTION_KERNEL) {
      unsigned int *physical_frame_addr = allocate_frame();
      *page_table_entry = ((unsigned int)physical_frame_addr & PAGE_ADDR_MASK);
      // TODO: Set appropriate flag based on the type passed
      // NOTE: When this is done, we should use create_page_table_entry() to
      // allocate the frame
      *page_table_entry |= PAGE_TABLE_FLAGS;
    } else {
      *page_table_entry = address & PAGE_ADDR_MASK;
      // TODO: Make it kernel accessible only
      *page_table_entry |= PAGE_TABLE_FLAGS;
    }
  }

  uint8_t *frame_base_addr = (uint8_t *)(*page_table_entry & PAGE_ADDR_MASK);
  unsigned int offset = ((unsigned int)address & FRAME_OFFSET_MASK);

  // TODO: This should be uint8_t *  and not unsigned int *, right?
  return (frame_base_addr + offset);
}

/** @brief Allocate multiple frames starting from a given virtual address
 *
 *  If one of the page within the virtual address range 
 *  [address, address + (nb_frmaes - 1) * PAGE_SIZE] is already allocated before
 *  the call, then the function won't allocate any new page and return with an
 *  error code.
 *
 *  @param address    The virtual address to start with
 *  @param nb_frames  The number of frames to allocate
 *  @param type       The frame's type (e.g. code, data...)
 *
 *  @return 0 on success, a negative number on error
 */
int load_multiple_frames(unsigned int address, unsigned int nb_frames, 
                            unsigned int type) {
  
  // Check that the number of frames requested is valid
  if (nb_frames <= 0 || nb_frames > num_user_frames) {
    lprintf("load_multiple_frames(): invalid number of frames");
    return -1;
  } 

  // Get page directory entry address
  unsigned int *page_directory_entry_addr = 
        get_page_directory_addr_with_offset(address);
  
  // If there is no page table associated with this entry, create it
  if (!is_entry_present(page_directory_entry_addr)) {
    create_page_table(page_directory_entry_addr, PAGE_TABLE_FLAGS);
  }
  
  // Get page table entry address
  unsigned int *page_table_entry_addr = 
        get_page_table_addr_with_offset(page_directory_entry_addr, address);

  unsigned int * start_dir_addr = page_directory_entry_addr,
               * start_table_addr = page_table_entry_addr;

  while(1) {

    if (is_entry_present(page_table_entry_addr)) {
      // Something is already occupying this virtual address, free everything
      // previously allocated and return
      free_frames_range(start_dir_addr, start_table_addr,
                        page_directory_entry_addr, page_table_entry_addr);
      return -1;
    } else {

      // Create a new table entry
      if (type != SECTION_KERNEL) {
        unsigned int *physical_frame_addr = allocate_frame();
        *page_table_entry_addr = ((unsigned int)physical_frame_addr & PAGE_ADDR_MASK);
        // TODO: Set appropriate flag based on the type passed
        // NOTE: When this is done, we should use create_page_table_entry() to
        // allocate the frame
        *page_table_entry_addr |= PAGE_TABLE_FLAGS;
      } else {
        *page_table_entry_addr = address & PAGE_ADDR_MASK;
        // TODO: Make it kernel accessible only
        *page_table_entry_addr |= PAGE_TABLE_FLAGS;
      }

      if (--nb_frames == 0) {
        // We have been able to allocate all frames, we can return
        return 0; 
      }

      ++page_table_entry_addr;
      if (!((unsigned int)page_table_entry_addr & PAGE_SIZE)) {
        // This was the last entry in the page table, go to the next page
        // directory entry
        ++page_directory_entry_addr;

        if (!((unsigned int)page_directory_entry_addr & PAGE_SIZE)) {
          // This was the last entry in the page directory, we cannot allocate
          // more frames beyond this address, free everything allocated up to
          // this point and return
          free_frames_range(start_dir_addr, start_table_addr,
                        page_directory_entry_addr, page_table_entry_addr);
        } else {
          // If there is no page table associated with this entry, create it
          if (!is_entry_present(page_directory_entry_addr)) {
            create_page_table(page_directory_entry_addr, PAGE_TABLE_FLAGS);
          }
          // Get the first entry in the new page table
          page_table_entry_addr = get_page_table_addr(page_directory_entry_addr);
        }
      }
    }
  } 

  return -1;
}

/** @brief Free all the physical frames pointed to by a given page directory
 *
 *  If all the frames pointed to by the page directory have been deallocated, 
 *  then the page directory itself is freed. 
 *  A task should not call this function with the address of its page directory
 *  and the 'free_kernel_space' parameter set to KERNEL_AND_USER_SPACE.
 *  A task calling this function with the address of its page directory and the 
 *  'free_kernel_space' parameter set to USER_SPACE_ONLY should never return to 
 *  user space afterwards.
 *
 *  @param page_directory_addr    The page table's address
 *  @param free_kernel_space  An integer indicating whether direct mapped kernel
 *    memory should be deallocated too
 *
 *  @return 0 if all the frames pointed to by the page have been deallocated
 *   (the page directory has been deallocated too), 1 otherwise (there is at
 *   least one valid entry remaining in the page PAGE_TABLE_DIRECTORY_MASK)
 */
int free_address_space(unsigned int *page_directory_addr, 
                        int free_kernel_space) {

  unsigned int nb_entries = PAGE_SIZE / SIZE_ENTRY_BYTES;
  unsigned int *page_directory_entry_addr;
  int something_remaining = 0;
  
  // Iterate over the page directory entries
  for (page_directory_entry_addr = page_directory_addr ;
       page_directory_entry_addr < page_directory_addr + nb_entries ;
       ++page_directory_entry_addr) {

    // Check if the entry is present 
    if (is_entry_present(page_directory_entry_addr)) {

      if (free_page_table(page_directory_entry_addr, free_kernel_space) == 0) {
        // If the page table has been freed, invalidate the page dir. entry
        set_entry_invalid(page_directory_entry_addr);
      } else {
        something_remaining = 1;
      }
    }

  }
  if (!something_remaining) {
    // If all entries have been invalidated, free the page directory from memory
    sfree(page_directory_addr, PAGE_SIZE);
    return 0;
  }

  return 1;
}

/** @brief Free all the physical frames pointed to by a given page table
 *
 *  If all the frames pointed to by the page table have been deallocated, then
 *  the page table itself is freed. 
 *  A task should not call this function with the address of one of its page 
 *  table and the 'free_kernel_space' parameter set to KERNEL_AND_USER_SPACE.
 *
 *  @param page_table_addr    The page table's address
 *  @param free_kernel_space  An integer indicating whether direct mapped kernel
 *    memory should be deallocated too
 *
 *  @return 0 if all the frames pointed to by the page have been deallocated
 *   (the page table has been deallocated too), 1 otherwise (there is at
 *   least one valid entry remaining in the page table)
 */
int free_page_table(unsigned int *page_table_addr, int free_kernel_space) {

  unsigned int nb_entries = PAGE_SIZE / SIZE_ENTRY_BYTES;
  unsigned int *page_table_entry_addr;  
  int something_remaining = 0;

  // Iterate over the page table entries
  for (page_table_entry_addr = page_table_addr ;
       page_table_entry_addr < page_table_addr + nb_entries ;
       ++page_table_entry_addr) {
  
    // Check if the entry is present
    if (is_entry_present(page_table_entry_addr)) {
      unsigned int *frame_addr = get_frame_addr(page_table_entry_addr);
      
      // Deallocate the frame if appropriate
      if (free_kernel_space == KERNEL_AND_USER_SPACE ||
          (unsigned int) frame_addr > USER_MEM_START) { 

        // Free the frame
        if (free_frame(frame_addr) < 0) {
          panic("free_page_table(): Failed to free frame");
        }

        // Invalidate the entry
        set_entry_invalid(page_table_entry_addr);

      } else {
        something_remaining = 1;
      }
    }
  }

  if (!something_remaining) {
    // If all entries have been invalidated, free the page table from memory
    sfree(page_table_addr, PAGE_SIZE);
    return 0;
  }

  return 1;

}

/** @brief Free all the frames allocated within a given range of virtual
 *    memory addresses 
 *
 *  The range lower bound is inclusive and the upper bound is exclusive. 
 *
 *  @param start_dir_entry    Starting page directory entry
 *  @param start_table_entry  Starting page table entry
 *  @param end_dir_entry      Ending page directory entry
 *  @param end_table_entry    Ending page table entry
 *
 *  @return void
 */
void free_frames_range(unsigned int *start_dir_entry, 
                      unsigned int *start_table_entry,
                      unsigned int *end_dir_entry,
                      unsigned int *end_table_entry) {

  while (start_dir_entry != end_dir_entry &&
          start_table_entry != end_table_entry) {

    if (is_entry_present(start_table_entry)) {

      // If the entry maps to a frame, free it
      unsigned int *frame_addr = get_frame_addr(start_table_entry);
    
      // Free the frame
      if (free_frame(frame_addr) < 0) {
        panic("free_frames_range(): Failed to free frame");
      }

      // Invalidate the entry
      set_entry_invalid(start_table_entry);

    }

    // Go to the next entry
    ++start_table_entry;
    if (!((unsigned int)start_table_entry & PAGE_SIZE)) {
      
      // Get the next entry in the page directory
      ++start_dir_entry;
      
      // Get the first entry in the new page table
      start_table_entry = get_page_table_addr(start_dir_entry);
    }
  }
  
}

/** @brief Enable paging and the "Page Global Enable" bit in %cr4
 *
 *  @return void
 */
void vm_enable() {
  set_cr0(get_cr0() | PAGING_ENABLE_MASK);
  set_cr4(get_cr4() | PAGE_GLOBAL_ENABLE_MASK);
}

// Receives an address and validates that the string pointed to by it is in the 
// address space of this task. Basically, ensure the page table entry is valid
// from the starting address to the point where '\0' is written. If '\0' is 
// encountered, it is valid. If we find that it needs another frame and there 
// is no page table entry for that address, we return false(0) for invalid.
int is_valid_string(char *addr) {
  do {
    unsigned int *page_directory_entry_addr = 
                get_page_directory_addr_with_offset((unsigned int) addr);

    // If there is no page table associated with this entry, return false
    if (!is_entry_present(page_directory_entry_addr)) {
      return FALSE;
    }

    unsigned int *page_table_entry = 
                get_page_table_addr(page_directory_entry_addr);

    // If there is no physical frame associated with this entry, return false
    if (!is_entry_present(page_table_entry)) {
      return FALSE;
    }

    unsigned int next_frame_boundary = ((unsigned int)addr / PAGE_SIZE) + 1;
    while((unsigned int)addr < next_frame_boundary && *addr != '\0') {
      addr++;
    }
  } while((*addr) != '\0');

  return TRUE;
}
