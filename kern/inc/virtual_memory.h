#ifndef _VIRTUAL_MEMORY_H_
#define _VIRTUAL_MEMORY_H_

#include <elf_410.h>

int vm_init();
int setup_vm(const simple_elf_t *elf);
int load_every_segment(const simple_elf_t *elf);
int load_segment(const char *fname, unsigned long offset, unsigned long size,
  unsigned long start_addr, int type);
void *load_frame(unsigned int address, unsigned int type);
void *allocate_frame();

#endif
