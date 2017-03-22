/** @file virtual_memory.h
 *  @brief This file contains the declarations for functions manipulating
 *  virtual memory
 *  @author akanjani, lramire1
 */

#ifndef _VIRTUAL_MEMORY_H_
#define _VIRTUAL_MEMORY_H_

#include <elf_410.h>

int vm_init();
unsigned int *setup_vm(const simple_elf_t *elf);
int load_every_segment(const simple_elf_t *elf);
int load_segment(const char *fname, unsigned long offset, unsigned long size,
                 unsigned long start_addr, int type);
void *load_frame(unsigned int address, unsigned int type);
void *allocate_frame();
void vm_enable();

int is_entry_present(unsigned int *entry_addr);
unsigned int *create_page_table(unsigned int *page_directory_entry_addr);
unsigned int *create_page_table_entry(unsigned int *page_table_entry_addr,
                                      uint32_t flags);

unsigned int *get_page_table_addr(unsigned int *page_directory_entry_addr);
unsigned int *get_frame_addr(unsigned int *page_table_entry_addr);
uint32_t get_entry_flags(unsigned int *entry_addr);

unsigned int *get_virtual_address(unsigned int *page_directory_entry_addr,
                                  unsigned int *page_table_entry_addr);

#endif
