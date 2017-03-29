/** @file virtual_memory.c
 *  @brief This file contains the definitions for functions manipulating
 *  virtual memory
 *  @author akanjani, lramire1
 */

#include <bitmap.h>
#include <common_kern.h>
#include <cr.h>
#include <elf_410.h>
#include <free_map.h>
#include <loader.h>
#include <page.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <virtual_memory.h>

#include <simics.h>

#define NUM_KERNEL_FRAMES 4096

#define PAGE_TABLE_DIRECTORY_MASK 0xffc00000
#define PAGE_DIR_RIGHT_SHIFT 22
#define PRESENT_BIT_MASK 1
#define PAGE_ADDR_MASK 0xfffff000
// TODO: Check if this should be user accessible and should the r/w bit be set?
#define PAGE_TABLE_DIRECTORY_FLAGS 0x7
#define PAGE_TABLE_MASK 0x3ff000
#define PAGE_TABLE_RIGHT_SHIFT 12
#define PAGE_TABLE_FLAGS 0x7
#define FRAME_OFFSET_MASK 0xfff

#define SECTION_KERNEL 0
#define SECTION_TXT 1
#define SECTION_DATA 2
#define SECTION_RODATA 3
#define SECTION_BSS 4
#define SECTION_STACK 5

#define STACK_SIZE 4096
#define STACK_START_ADDR 0xfffff000

#define PAGING_ENABLE_MASK 0x80000000
#define PAGE_GLOBAL_ENABLE_MASK 0x80

#define ENTRY_SIZE_LOG2 2
#define PAGE_SIZE_LOG2 12

static unsigned int num_user_frames;

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
    // TODO: free
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

/** @brief Set up a page table entry and allocate a new frame for this entry
 *
 *  @param address A virtual address
 *  @param type The address's type (e.g. code, data...)
 *
 *  @return 0 The allocated frame's physical address
 */
void *load_frame(unsigned int address, unsigned int type) {

  // get base register in base_addr(unsigned int *)
  unsigned int *base_addr = (unsigned int *)get_cr3();

  unsigned int offset = (((unsigned int)address & PAGE_TABLE_DIRECTORY_MASK) >>
                         PAGE_DIR_RIGHT_SHIFT);

  unsigned int *page_directory_entry_addr = base_addr + offset;

  // If there is no page table associated with this entry, create it
  if (!is_entry_present(page_directory_entry_addr)) {
    create_page_table(page_directory_entry_addr);
  }

  // Access the page table
  unsigned int *page_table_base_addr =
      get_page_table_addr(page_directory_entry_addr);

  offset =
      (((unsigned int)address & PAGE_TABLE_MASK) >> PAGE_TABLE_RIGHT_SHIFT);
  unsigned int *page_table_entry = page_table_base_addr + offset;

  // If there is no physical frame associated with this entry, create it
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
  offset = ((unsigned int)address & FRAME_OFFSET_MASK);

  // TODO: This should be uint8_t *  and not unsinged int *, right?
  return (frame_base_addr + offset);
}

/** @brief Find and allocate a free frame in memory
 *
 *  @return The frame's address if one free frame was found, NULL otherwise
 */
void *allocate_frame() {
  int i;
  for (i = 0; i < num_user_frames; i++) {
    if (get_bit(&free_map, i) == BITMAP_UNALLOCATED) {
      set_bit(&free_map, i);
      return (void *)(USER_MEM_START + (i * PAGE_SIZE));
    }
  }
  return NULL;
}

/** @brief Check if the given entry is valid (maps to something meaningful)
 *
 *  @param The entry's address
 *
 *  @return A positive integer if the entry is valid, 0 otherwise
 */
int is_entry_present(unsigned int *entry_addr) {
  return *entry_addr & PRESENT_BIT_MASK;
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
 *  @param page_directory_entry_addr The address of the page directory's entry
 *  @param page_table_entry_addr The address of the page table's entry
 *  
 *  @return The virtual address associated with the given entries
 */
unsigned int *get_virtual_address(unsigned int *page_directory_entry_addr,
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

  return (unsigned int *) virtual_address;
}

/** @brief Enable paging and the "Page Global Enable" bit in %cr4
 *
 *  @return void
 */
void vm_enable() {
  set_cr0(get_cr0() | PAGING_ENABLE_MASK);
  set_cr4(get_cr4() | PAGE_GLOBAL_ENABLE_MASK);
}
