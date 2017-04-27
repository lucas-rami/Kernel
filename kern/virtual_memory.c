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
#include <kernel_state.h>

/* Standard library */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* VM system */
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <virtual_memory_defines.h>
#include "virtual_memory_internal.h"

/* Debugging */
#include <simics.h>

#include <limits.h>

/* Hold the number of user frames in the system */
unsigned int num_user_frames;

/* Bitmap holding the set of (un)allocated frames */ 
bitmap_t free_map;

/** @brief  Initializes the virtual memory system
 *
 *  This function should be called once before any other function acting on
 *  virtual memory.
 *
 *  @return 0 on success, a negative number on error
 */
int vm_init() {

  // Figure out the number of user frames
  num_user_frames = kernel.free_frame_count;

  int size = (kernel.free_frame_count / BITS_IN_UINT8_T) + 1;

  bitmap_init(&free_map, size);
  return 0;
}

/** @brief  Setup the virtual memory for a single task
 *
 *  @param elf_info Data strucure holding the important features
 *                  of the task's ELF header
 *
 *  @return The page table directory address on success, NULL on error
 */
unsigned int *setup_vm(const simple_elf_t *elf_info) {

  // Check the argument
  if (elf_info == NULL) {
    return NULL;
  }

  // Allocate a page directory
  unsigned int *page_dir =
      (unsigned int *)smemalign(PAGE_SIZE, PAGE_SIZE);

  // Memset the page directory to 0
  memset(page_dir, 0, PAGE_SIZE);

  int is_first_task = FIRST_TASK_FALSE;
  if (!strcmp(elf_info->e_fname, FIRST_TASK)) {
    is_first_task = FIRST_TASK_TRUE;
  }

  // Load kernel section
  int i;
  for (i = 0; i < USER_MEM_START; i += PAGE_SIZE) {
    load_frame(i, SECTION_KERNEL, page_dir, is_first_task);
  }

  if (is_first_task == FIRST_TASK_TRUE) {
    set_cr3((uint32_t)page_dir);
    vm_enable();
  }

  // Add stack area as well.
  if (load_every_segment(elf_info, page_dir) < 0) {
    // TODO: test this
    free_address_space(page_dir, KERNEL_AND_USER_SPACE);
    return NULL;
  }

  // TODO: Make this atomic and reverse the changes if something goes bad
  // set cr3 to this value;

  kernel.current_thread->cr3 = (uint32_t)page_dir;
  set_cr3((uint32_t)page_dir);

  return page_dir;
}

/** @brief Load every segment of a task into virtual memory
 *
 *  @param elf_info Data strucure holding the important features
 *  of the task's ELF header
 *
 *  @return 0 on success, a negative number on error
 */
int load_every_segment(const simple_elf_t *elf, unsigned int *cr3) {
  if (load_segment(elf->e_fname, elf->e_txtoff, elf->e_txtlen, elf->e_txtstart,
               SECTION_TXT, cr3) < 0) {
    return -1;
  }
  if (load_segment(elf->e_fname, elf->e_datoff, elf->e_datlen, elf->e_datstart,
               SECTION_DATA, cr3) < 0) {
    return -1;
  }
  if (load_segment(elf->e_fname, elf->e_rodatoff, elf->e_rodatlen,
      elf->e_rodatstart, SECTION_RODATA, cr3) < 0) {
    return -1;
  }
  if (load_segment(elf->e_fname, 0, elf->e_bsslen, elf->e_bssstart, SECTION_BSS,
      cr3) < 0) {
    return -1;
  }
  if (load_segment(NULL, 0, STACK_SIZE, STACK_START_ADDR, SECTION_STACK, cr3)
       < 0) {
    return -1;
  }
  // Initialize the global variables and bss to zero once they are allocated
  // space for
  return 0;
}

/** @brief  Load one segment of a task into virtual memory
 *
 *  @param  fname       Task's filename
 *  @param  offset      Offset in the file where the segment is located
 *  @param  size        Size of the segment in the file
 *  @param  start_addr  Starting address of segment in virtual memory
 *  @param  type        Segment's type
 *
 *  @return 0 on success, a negative number on error
 */
int load_segment(const char *fname, unsigned long offset, unsigned long size,
                 unsigned long start_addr, int type, 
                 unsigned int *page_table_directory) {

  // get page table base register
  // assume it is base_addr
  uint32_t old_cr3 = get_cr3();
  char *buf;
  if (type != SECTION_STACK && type != SECTION_BSS) {
    buf = (char *)malloc(sizeof(char) * size);
    if (buf == NULL) {
      lprintf("NULL buf");
      return -1;
    }
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
    frame_addr = load_frame(addr, type, page_table_directory, FIRST_TASK_FALSE);

    if (frame_addr == NULL) {
      lprintf("Error\n");
      return -1;
    }
    int temp_offset = ((unsigned int)frame_addr % PAGE_SIZE);
    unsigned int size_allocated = ((PAGE_SIZE - temp_offset) < max_size)
                                      ? (PAGE_SIZE - temp_offset)
                                      : max_size;

    kernel.current_thread->cr3 = (uint32_t)page_table_directory;
    set_cr3((uint32_t)page_table_directory);
    if (type == SECTION_BSS) {
      memset((char*)addr, 0, size_allocated);
    } else if (type != SECTION_STACK && type != SECTION_BSS) {
      memcpy((char*)addr, buf + curr_offset, size_allocated);
    }
    remaining_size -= size_allocated;
    curr_offset += size_allocated;
    addr += size_allocated;
  }

  if (type != SECTION_STACK && type != SECTION_BSS) {
    free(buf);
  }
  kernel.current_thread->cr3 = old_cr3;
  set_cr3(old_cr3);
  return 0;
}

/** @brief  Get the physical address associated with a virtual address, if the
 *          page directory/table entries for this virtual address do not exist 
 *          yet, create them 
 *
 *  @param  address A virtual address
 *  @param  type    The address's type (e.g. code, data...)
 *
 *  @return 0 The physical address associated with the virtual one
 */
void *load_frame(unsigned int address, unsigned int type, unsigned int *cr3,
                 int is_first_task) {

  unsigned int *page_directory_entry_addr = get_page_directory_addr(
                                            (unsigned int*)address, cr3);

  int page_table_allocated = 0;

  // If there is no page table associated with this entry, create it
  if (!is_entry_present(page_directory_entry_addr)) {

    // In case there is no kernel memory left
    if (create_page_table(page_directory_entry_addr, DIRECTORY_FLAGS, is_first_task) == NULL) {
      return NULL;
    }

    if (is_first_task == FIRST_TASK_TRUE && type == SECTION_KERNEL) {
      if (address == 0) {
        kernel_page_table_1 = (unsigned int)get_page_table_addr_with_offset(page_directory_entry_addr, address);
      } else if (address == (USER_MEM_START/4)) {
        kernel_page_table_2 = (unsigned int)get_page_table_addr_with_offset(page_directory_entry_addr, address);
      } else if (address == (USER_MEM_START/2)) {
        kernel_page_table_3 = (unsigned int)get_page_table_addr_with_offset(page_directory_entry_addr, address);
      } else if (address == ((USER_MEM_START/4)*3)) {
        kernel_page_table_4 = (unsigned int)get_page_table_addr_with_offset(page_directory_entry_addr, address);
      }
    }

    page_table_allocated = 1;
  }

  unsigned int *page_table_entry = 
        get_page_table_addr_with_offset(page_directory_entry_addr, address);

  // If there is no physical frame associated with this entry, allocate it
  if (!is_entry_present(page_table_entry)) {
    
    if (type != SECTION_KERNEL) {
      
      // Create flags for new entry depending on segment
      unsigned int flags = (type == SECTION_RODATA || type == SECTION_TXT) ?
                            PAGE_USER_RO_FLAGS : PAGE_USER_FLAGS;
     
      // Create page table entry
      if (create_page_table_entry(page_table_entry, flags) == NULL) {
        if (page_table_allocated) {
          sfree(get_page_table_addr(page_directory_entry_addr), PAGE_SIZE);
        }
        return NULL;
      }

      // Zero out old frame
      // TODO: should we do this every time or only for SECTION_BSS ?
      uint32_t old_cr3 = get_cr3();
      kernel.current_thread->cr3 = (uint32_t)cr3;
      set_cr3((uint32_t)cr3);
      memset((char*)((address/PAGE_SIZE) * PAGE_SIZE), 0, PAGE_SIZE);
      kernel.current_thread->cr3 = (uint32_t)old_cr3;
      set_cr3(old_cr3);
    
    } else {

      *page_table_entry = address & PAGE_ADDR_MASK;
      *page_table_entry |= PAGE_KERN_FLAGS; 

    }
  }

  uint8_t *frame_base_addr = (uint8_t *)(*page_table_entry & PAGE_ADDR_MASK);
  unsigned int offset = address & FRAME_OFFSET_MASK;

  // TODO: This should be uint8_t *  and not unsigned int *, right?
  return (frame_base_addr + offset);
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
  for (page_directory_entry_addr = (page_directory_addr + 4);
       page_directory_entry_addr < page_directory_addr + nb_entries ;
       ++page_directory_entry_addr) {

    // Check if the entry is present 
    if (is_entry_present(page_directory_entry_addr)) {

      if (free_page_table(page_directory_entry_addr, 
            get_page_table_addr(page_directory_entry_addr), 
             free_kernel_space) == 0) {
        // If the page table has been freed, invalidate the page dir. entry
        set_entry_invalid(page_directory_entry_addr, get_virtual_address(
          page_directory_addr, get_page_table_addr(page_directory_entry_addr)));
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
 *  @param page_dir_entry_addr  The page directory entry
 *  @param page_table_addr      The page table's address
 *  @param free_kernel_space    An integer indicating whether direct mapped 
 *  kernel memory should be deallocated too
 *
 *  @return 0 if all the frames pointed to by the page have been deallocated
 *   (the page table has been deallocated too), 1 otherwise (there is at
 *   least one valid entry remaining in the page table)
 */
int free_page_table(unsigned int * page_dir_entry_addr, 
                    unsigned int *page_table_addr, int free_kernel_space) {

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
      
      if (free_kernel_space == USER_SPACE_ONLY &&
          (unsigned int) frame_addr < USER_MEM_START) {
        something_remaining = 1;
        continue;
      }

      // Deallocate the frame if appropriate
      if ((unsigned int)frame_addr >= USER_MEM_START) { 
        // Free the frame
        if (free_frame(frame_addr) < 0) {
          panic("free_page_table(): Failed to free frame");
        }
      }

      // Invalidate the entry
      set_entry_invalid(page_table_entry_addr, 
          get_virtual_address(page_dir_entry_addr, page_table_entry_addr));
    }
  }

  if (!something_remaining) {
    // If all entries have been invalidated, free the page table from memory
    sfree(page_table_addr, PAGE_SIZE);
    return 0;
  }

  return 1;
}

/** @brief Free a given number of frames starting from a virtual address
 *
 *  The lower bound (frame mapped to by start_dir_address and 
 *  start_table_address) is inclusive. If a frame in the given range is not
 *  allocated when we try to free it, then the function continue its execution 
 *  normally.
 *
 *  @param address    A virtual address
 *  @param nb_frames  The number of frames to free from the starting address     
 *
 *  @return void
 */
void free_frames_range(unsigned int address, unsigned int nb_frames) {

  // Make sure the address is page aligned
  address &= ~FRAME_OFFSET_MASK;

  int i;
  for (i = 0 ; i < nb_frames ; ++i, address += PAGE_SIZE) {

    // Get page directory entry address
    unsigned int *page_dir_entry_addr = 
        get_page_directory_addr_with_offset(address);

    // TODO: Make sure we do not free memory in kernel space ??

    if (is_entry_present(page_dir_entry_addr)) {

      unsigned int *page_table_entry_addr =
        get_page_table_addr_with_offset(page_dir_entry_addr, address);

      if (is_entry_present(page_table_entry_addr)) {
        
        // If the entry is present, free the frame
        free_frame(get_frame_addr(page_table_entry_addr));
        
        // Invalidate the entry
        set_entry_invalid(page_table_entry_addr, address);
        
      }      
    }

    // TODO: free page directory entry when all page table entries are invalid ?

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

/** @brief Check whether the memory starting at a particular address and on a
 *    certain length lies withing the current task's address space
 *
 *  @param address  A virtual address (the starting address)
 *  @param len      The length, in bytes, to check for validity
 *
 *  @return 0 if the buffer is valid, a negative number otherwise
 */
int is_buffer_valid(unsigned int address, int len) {

  // Get page directory entry of starting address
  unsigned int * page_dir_entry_addr = 
              get_page_directory_addr_with_offset(address);

  // Check that the entry is valid 
  if (!is_entry_present(page_dir_entry_addr)) {
    return -1;
  }

  // Get page table entry of starting address
  unsigned int * page_table_entry_addr =
              get_page_table_addr_with_offset(page_dir_entry_addr, address);

  // Check that the entry is valid 
  if (!is_entry_present(page_table_entry_addr)) {
      return -1;
  }

  // Decrement length by the remaining amount of space in the frame 
  len -= (PAGE_SIZE - (address & FRAME_OFFSET_MASK));

  while (len > 0) {

    // Go to next table entry
    ++page_table_entry_addr;

    if (!((unsigned int)page_table_entry_addr & FRAME_OFFSET_MASK)) {
      // If the entry was the last one in the page table,
      // go to next page directory entry 
      ++page_dir_entry_addr;
      if (!((unsigned int)page_table_entry_addr & FRAME_OFFSET_MASK) ||
          !is_entry_present(page_dir_entry_addr)) {
        // If the directory entry was the last one or the current one is invalid
        // then the buffer is invalid

        return -1;
      }
      // Get next page table 
      page_table_entry_addr = get_page_table_addr(page_dir_entry_addr);
    }
  
    // Check that the page table entry is valid
    if (!is_entry_present(page_table_entry_addr)) {

      return -1;
    }

    // Decrement remaining length
    len -= PAGE_SIZE;

  }

  // Buffer is valid
  return 0;

}

/** @brief Check whether the string starting at a particular address
 *   lies withing the current task's address space
 *
 *  @param addr     A virtual address (the starting address)
 *
 *  @return FALSE(0) if the buffer is invalid, TRUE(1) otherwise
 */
int is_valid_string(char *addr) {
  unsigned int *base_addr = (unsigned int*)get_cr3();
  do {
    unsigned int *page_directory_entry_addr = get_page_directory_addr(
                                              (unsigned int*)addr, base_addr);

    // If there is no page table associated with this entry, return false
    if (!is_entry_present(page_directory_entry_addr)) {
      return FALSE;
    }

    unsigned int *page_table_entry = get_page_table_addr_with_offset(
                                     page_directory_entry_addr,
                                     (unsigned int)addr);

    // If there is no physical frame associated with this entry, return false
    if (!is_entry_present(page_table_entry)) {
      lprintf("page_table_entry_addr not present");
      return FALSE;
    }

    unsigned int next_frame_boundary = 
                      (((unsigned int)addr / PAGE_SIZE) + 1) * PAGE_SIZE;
    while(((unsigned int)addr <= next_frame_boundary-1) && (*addr != '\0')) {
      addr++;
    }
  } while((*addr) != '\0');

  return TRUE;
}
