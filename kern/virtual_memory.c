#include <bitmap.h>
#include <elf_410.h>
#include <free_map.h>
#include <stdint.h>
#include <common_kern.h>
#include <stdlib.h>
#include <page.h>
#include <virtual_memory.h>
#include <loader.h>
#include <string.h>
#include <cr.h>

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

unsigned int num_user_frames;

int vm_init() {

  // Figure out the number of user frames
  num_user_frames = machine_phys_frames() - NUM_KERNEL_FRAMES;

  int size = (num_user_frames / BITS_IN_UINT8_T) + 1;

  bitmap_init(&free_map, size);

  return 0;
}

int setup_vm(const simple_elf_t *elf_info) {

  unsigned int *page_table_directory = (unsigned int *)smemalign( PAGE_SIZE,
                                        PAGE_SIZE);
  // set cr3 to this value;
  set_cr3((uint32_t)page_table_directory);

  int i;
  for (i = 0; i < USER_MEM_START; i += PAGE_SIZE) {
    load_frame(i, SECTION_KERNEL);
  }

  // Load kernel section as well
  // Add stack area as well.
  if (load_every_segment(elf_info) < 0) {
    // free?
    return -1;
  }

  return 0;
}

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

int load_segment(const char *fname, unsigned long offset, unsigned long size,
  unsigned long start_addr, int type) {

  // get page table base register
  // assume it is base_addr;

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
    unsigned int size_allocated = ((PAGE_SIZE - temp_offset) < max_size) ?
      (PAGE_SIZE - temp_offset) : max_size;

    if (type != SECTION_STACK && type != SECTION_BSS) {
      memcpy((frame_addr), buf + curr_offset, size_allocated);
    }
    if ( type == SECTION_BSS) {
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

void *load_frame(unsigned int address, unsigned int type) {
  // get base register in base_addr(unsigned int *)

  unsigned int *base_addr = (unsigned int *)get_cr3();

  unsigned int offset = (((unsigned int)address & PAGE_TABLE_DIRECTORY_MASK)
    >> PAGE_DIR_RIGHT_SHIFT);

  unsigned int *page_table_directory_entry_addr = base_addr + offset;

  if (!(*page_table_directory_entry_addr & PRESENT_BIT_MASK)) {
    unsigned int *page_table_entry_addr = (unsigned int *)smemalign(PAGE_SIZE,
      PAGE_SIZE);
    memset(page_table_entry_addr, 0, PAGE_SIZE);
    *page_table_directory_entry_addr = ((unsigned int)page_table_entry_addr &
      PAGE_ADDR_MASK);
    *page_table_directory_entry_addr |= PAGE_TABLE_DIRECTORY_FLAGS;
  }

  unsigned int *page_table_base_addr = (unsigned int *)(*page_table_directory_entry_addr &
    PAGE_ADDR_MASK);
  offset = (((unsigned int)address & PAGE_TABLE_MASK) >>
    PAGE_TABLE_RIGHT_SHIFT);
  unsigned int *page_table_entry = page_table_base_addr + offset;

  if (!(*page_table_entry & PRESENT_BIT_MASK)) {
    if (type != SECTION_KERNEL) {
      unsigned int *physical_frame_addr = allocate_frame();
      *page_table_entry = ((unsigned int)physical_frame_addr & PAGE_ADDR_MASK);
      // TODO: Set appropriate flag based on the type passed
      *page_table_entry |=  PAGE_TABLE_FLAGS;
    } else {
      *page_table_entry = address & PAGE_ADDR_MASK;
      // TODO: Make it kernel accessible only
      *page_table_entry |=  PAGE_TABLE_FLAGS;
    }
  }

  uint8_t *frame_base_addr = (uint8_t*)(*page_table_entry & PAGE_ADDR_MASK);
  offset = ((unsigned int)address & FRAME_OFFSET_MASK);

  // TODO: This should be uint8_t *  and not unsinged int *, right?
  return (frame_base_addr + offset);
}

void *allocate_frame() {
  int i;
  for (i = 0; i < num_user_frames; i++) {
    if (get_bit(&free_map, i) == BITMAP_UNALLOCATED) {
      set_bit(&free_map, i);
      return (void*)(USER_MEM_START + (i * PAGE_SIZE));
    }
  }
  return NULL;
}

void vm_enable() {
  uint32_t curr = get_cr0();
  set_cr0(curr | PAGING_ENABLE_MASK);
}
