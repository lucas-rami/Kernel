/** @file virtual_memory_defines.h
 *  @brief This file contains constant definitions for manipulating virtual
 *      memory
 *  @author akanjani, lramire1
 */

#ifndef _VIRTUAL_MEMORY_DEFINES_H_
#define _VIRTUAL_MEMORY_DEFINES_H_

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
#define PAGE_TABLE_KERNEL_FLAGS 0x3
#define PAGE_TABLE_RESERVED_BITMASK 0x100
#define FRAME_OFFSET_MASK 0x00000fff

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
#define SIZE_ENTRY_BYTES 4
#define NB_ENTRY_PER_PAGE PAGE_SIZE / SIZE_ENTRY_BYTES

/* Constants for free_address_space()/free_page_table() functions */
#define KERNEL_AND_USER_SPACE 0
#define USER_SPACE_ONLY 1

#endif /* _VIRTUAL_MEMORY_DEFINES_H_ */
