#include <interrupts.h>
#include "page_fault_asm.h"
#include <seg.h>
#include <simics.h>
#include <virtual_memory_helper.h>
#include <cr.h>
#include <syscalls.h>

#define PAGE_FAULT_IDT 0xE

int page_fault_init(void)
{
  return register_handler((uintptr_t)page_fault_handler, TRAP_GATE, 
      PAGE_FAULT_IDT, USER_PRIVILEGE_LEVEL, SEGSEL_KERNEL_CS);
}

void page_fault_c_handler(char *stack_ptr)
{

  lprintf("\tpage_fault_c_handler(): Page fault @ %p", (void*) get_cr2());
  MAGIC_BREAK;

  if (allocate_frame_if_address_requested(get_cr2()) < 0) {
    create_stack_sw_exception(SWEXN_CAUSE_PAGEFAULT, stack_ptr);
    kern_set_status(-2);
    kern_vanish();
  }
  
  lprintf("\tpage_fault_c_handler(): The frame allocation was successfull");
  // Re run the instruction

  // Get the page table base register from the register cr3
  //
  // Figure out the correct offset and check if the entry is valid
  // if not, call malloc to create a new page table entry and set the
  // address of the new entry in the page directory and set the valid
  // bit and continue
  //
  // Go at the correct offset in the page table and check whether the
  // entry is valid or not
  // If it is valid, why was there a page fault?
  //
  // Allocate a new frame from the bit map and set it to be allocated
  // copy its address to the page table entry and set the page table entry
  // as valid now
  //
  return;
}
