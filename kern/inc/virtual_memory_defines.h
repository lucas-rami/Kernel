/** @file virtual_memory_defines.h
 *  @brief This file contains constant definitions for manipulating virtual
 *      memory
 *  @author akanjani, lramire1
 */

#ifndef _VIRTUAL_MEMORY_DEFINES_H_
#define _VIRTUAL_MEMORY_DEFINES_H_

/* --------  MASKS  -------- */
#define PAGE_TABLE_DIRECTORY_MASK 0xffc00000
#define PAGE_TABLE_MASK 0x3ff000
#define PAGE_ADDR_MASK 0xfffff000
#define PAGE_FLAGS_MASK 0xfff
#define FRAME_OFFSET_MASK 0x00000fff

/* --------  SHIFTS  -------- */
#define PAGE_DIR_RIGHT_SHIFT 22
#define PAGE_TABLE_RIGHT_SHIFT 12

/* --------  FLAGS  -------- */
#define PRESENT_BIT     0x001
#define PAGE_WRITABLE   0x002
#define USER_ACCESSIBLE 0x004 
#define WRITE_THROUGH   0x008 // Should be unset
#define DISABLE_CACHING 0x010 // Should be unset
#define ACCESSED        0x020 // Ignored
#define DIRTY           0x040 // Ignored
#define PAGE_SIZE_FLAG  0x080 // Should be unset
#define PAGE_GLOBAL     0x100
#define PAGE_TABLE_RESERVED_BIT 0x200

#define DIRECTORY_FLAGS PRESENT_BIT | PAGE_WRITABLE | USER_ACCESSIBLE
#define PAGE_KERN_FLAGS PRESENT_BIT | PAGE_WRITABLE | PAGE_GLOBAL
#define PAGE_USER_RO_FLAGS PRESENT_BIT | USER_ACCESSIBLE
#define PAGE_USER_FLAGS PAGE_USER_RO_FLAGS | PAGE_WRITABLE

/* --------  SEGMENTS  -------- */
#define SECTION_KERNEL 0
#define SECTION_TXT 1
#define SECTION_DATA 2
#define SECTION_RODATA 3
#define SECTION_BSS 4
#define SECTION_STACK 5

/* --------  CONTROL REGISTERS  -------- */
#define PAGING_ENABLE_MASK 0x80000000
#define PAGE_GLOBAL_ENABLE_MASK 0x80

/* --------  SIZES  -------- */
#define ENTRY_SIZE_LOG2 2
#define PAGE_SIZE_LOG2 12
#define SIZE_ENTRY_BYTES 4
#define NB_ENTRY_PER_PAGE PAGE_SIZE / SIZE_ENTRY_BYTES
#define STACK_SIZE 4096
#define STACK_START_ADDR 0xfffff000
#define NUM_KERNEL_FRAMES 4096

/* Constants for free_address_space()/free_page_table() functions */
#define KERNEL_AND_USER_SPACE 0
#define USER_SPACE_ONLY 1

/* Constants for setup_vm() function */
#define FIRST_TASK_TRUE 1
#define FIRST_TASK_FALSE 0

#endif /* _VIRTUAL_MEMORY_DEFINES_H_ */
