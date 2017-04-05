/** @file virtual_memory_helper.c
 *  @brief This file contains the declarations for helper functions used to 
 *      access and/or modify the virtual memory system's state
 *  @author akanjani, lramire1
 */

#include <page.h>
#include <malloc.h>
#include <string.h>
#include <bitmap.h>
#include <common_kern.h>
#include <cr.h>
#include <kernel_state.h>

/* VM system */
#include <virtual_memory.h>
#include <virtual_memory_helper.h>
#include <virtual_memory_defines.h>

/* Debugging */
#include <simics.h>

/* Hold the number of user frames in the system */
extern unsigned int num_user_frames;
/* Bitmap holding the set of (un)allocated frames */ 
extern bitmap_t free_map;

/** @brief Check if the given entry is valid (maps to something meaningful)
 *
 *  @param The entry's address
 *
 *  @return A positive integer if the entry is valid, 0 otherwise
 */
int is_entry_present(unsigned int *entry_addr) {
  return *entry_addr & PRESENT_BIT_MASK;
}

/** @brief Invalidate an entry
 *
 *  @param The entry's address
 *
 *  @return void
 */
void set_entry_invalid(unsigned int *entry_addr) {
  *entry_addr &= ~PRESENT_BIT_MASK; 
}

/** @brief Create a new page table (and a new entry in the page directory)
 *
 *  @param page_directory_entry_addr The address in the page directory where the
 *    entry should be created
 *  @param flags The flags that should be assigned to the new entry
 *
 *  @return The address of the newly created page table
 */
unsigned int *create_page_table(unsigned int *page_directory_entry_addr, 
                                uint32_t flags) {                          
  unsigned int *page_table_entry_addr =
      (unsigned int *)smemalign(PAGE_SIZE, PAGE_SIZE);
  memset(page_table_entry_addr, 0, PAGE_SIZE);
  *page_directory_entry_addr =
      ((unsigned int)page_table_entry_addr & PAGE_ADDR_MASK);
  *page_directory_entry_addr |= flags;
  return page_table_entry_addr;
}

/** @brief Create a page table entry
 *
 *  @param page_table_entry_addr The address where the page entry should be 
 *    created
 *  @param flags The set of flags that should be assigned to the new entry
 *
 *  @return The address of the newly allocated frame
 */
unsigned int *create_page_table_entry(unsigned int *page_table_entry_addr,
                                      uint32_t flags) {
  unsigned int *physical_frame_addr = allocate_frame();
  *page_table_entry_addr = ((unsigned int)physical_frame_addr & PAGE_ADDR_MASK);
  *page_table_entry_addr |= flags;
  return physical_frame_addr;
}

/** @brief Get the address of the page table stored in a page directory entry
 *  
 *  @param page_directory_entry_addr The page directory entry's address
 *  
 *  @return The page table's address
 */
unsigned int *get_page_table_addr(unsigned int *page_directory_entry_addr) {
  return (unsigned int *)(*page_directory_entry_addr & PAGE_ADDR_MASK);
}

/** @brief Get the address of the frame stored in a page table entry
 *  
 *  @param page_table_entry_addr The page table entry's address
 *  
 *  @return The frame's address
 */
unsigned int *get_frame_addr(unsigned int *page_table_entry_addr) {
  return (unsigned int *)(*page_table_entry_addr & PAGE_ADDR_MASK);
}

/** @brief Get the page directory entry for a particular virtual address
 *   with the given cr3 value
 *
 *  @param address A virtual address
 *  @param base_addr The base address to the page directory fot this process
 *
 *  @return The page directory entry which the virtual address maps to
 */
unsigned int *get_page_directory_addr(unsigned int *address, unsigned int *base_addr) {
  unsigned int offset = (((unsigned int)address & PAGE_TABLE_DIRECTORY_MASK) >>
                         PAGE_DIR_RIGHT_SHIFT);

  return base_addr + offset;
}

/** @brief Get the page directory entry for a particular virtual address
 *
 *  @param address A virtual address
 *
 *  @return The page directory entry which the virtual address maps to
 */
unsigned int *get_page_directory_addr_with_offset(unsigned int address) {
  // get base register in base_addr(unsigned int *)
  unsigned int *base_addr = (unsigned int *)get_cr3();

  unsigned int offset = ((address & PAGE_TABLE_DIRECTORY_MASK) >>
                         PAGE_DIR_RIGHT_SHIFT);
  return base_addr + offset;
}
/** @brief Get the entry related to a particular virtual address in a page table
 *
 *  @param page_directory_entry_addr  The page directory entry pointing to the
 *    page table
 *  @param address                    A virtual address
 *
 *  @return The page table entry related to the virtual address
 */
unsigned int *get_page_table_addr_with_offset(
             unsigned int *page_directory_entry_addr, unsigned int address) {
  // Access the page table
  unsigned int *page_table_base_addr = get_page_table_addr(
                                       page_directory_entry_addr);
  unsigned int offset =
      ((address & PAGE_TABLE_MASK) >> PAGE_TABLE_RIGHT_SHIFT);

  return page_table_base_addr + offset; 
}


/** @brief Get the flags of an entry in a page table/directory
 *  
 *  @param enry_addr The entry's address
 *  
 *  @return The entry's flags
 */
uint32_t get_entry_flags(unsigned int *entry_addr) {
  return *entry_addr & PAGE_TABLE_FLAGS;
}

/** @brief Get the virtual address associated with a physical frame, given the
 *  entries in the page directory/table that map to this frame
 *
 *  @param page_directory_entry_addr The address of the page directory entry
 *  @param page_table_entry_addr The address of the page table entry
 *  
 *  @return The virtual address associated with the given entries
 */
unsigned int get_virtual_address(unsigned int *page_directory_entry_addr,
                                  unsigned int *page_table_entry_addr) {

  unsigned int virtual_address = 0;

  unsigned int page_dir_index =
      ((unsigned int)page_directory_entry_addr & FRAME_OFFSET_MASK) >>
      ENTRY_SIZE_LOG2;
  unsigned int page_tab_index =
      ((unsigned int)page_table_entry_addr & FRAME_OFFSET_MASK) >>
      ENTRY_SIZE_LOG2;

  virtual_address |= page_dir_index << PAGE_DIR_RIGHT_SHIFT;
  virtual_address |= page_tab_index << PAGE_SIZE_LOG2;

  return virtual_address;
}


/** @brief Find and allocate a free frame in memory
 *
 *  @return The frame's address if one free frame was found, NULL otherwise
 */
unsigned int *allocate_frame() {
  int i;
  for (i = 0; i < num_user_frames; i++) {
    if (get_bit(&free_map, i) == BITMAP_UNALLOCATED) {
      set_bit(&free_map, i);
      return (void *)(USER_MEM_START + (i * PAGE_SIZE));
    }
  }
  return NULL;
}

/** @brief Free an allocated frame from memory
 *
 *  @param addr The frame's address
 *
 *  @return 0 on success, a negative number if the frame wasn't previously
 *    allocated
 */
int free_frame(unsigned int* addr) {
  int frame_index = ((unsigned int)(addr) - USER_MEM_START) / PAGE_SIZE;
  if (!get_bit(&free_map, frame_index)) {
    lprintf("free_frame(): Trying to deallocate unallocated frame");
    return -1;
  }
  unset_bit(&free_map, frame_index);
  return 0;
}
