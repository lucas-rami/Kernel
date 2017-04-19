#include <kernel_state.h>
#include <stddef.h>
#include <simics.h>
#include <ureg.h>
#include <cr.h>
#include <context_switch_asm.h>
#include <eflags.h>
#include <assert.h>
#include <string.h>

int create_stack_sw_exception(unsigned int cause, char *stack_start) {
  if (kernel.current_thread->swexn_values.esp3 == NULL ||
      kernel.current_thread->swexn_values.eip == NULL) {
    lprintf("No exception handler installed");
    return 0;
  }

  const unsigned int unsigned_int_size = sizeof(unsigned int);
  const unsigned int ureg_size = sizeof(ureg_t);
  const unsigned int pointer_size = sizeof(void*);
  char *stack_ptr = kernel.current_thread->swexn_values.esp3;

  char *ureg_start = stack_ptr - ureg_size;

  *(unsigned int *)(ureg_start) = cause;
  *(unsigned int *)(ureg_start + unsigned_int_size) = (unsigned int)get_cr2();
  memcpy(ureg_start + (2 * unsigned_int_size), stack_start, ureg_size - (8 * unsigned_int_size));
  stack_start += ureg_size - (8 * unsigned_int_size);
  stack_start += pointer_size;
  memcpy(ureg_start + (14 * unsigned_int_size), stack_start, 6 * unsigned_int_size);
  // lprintf("The value at stack_start is %p", (uint32_t*)*(unsigned int*)(stack_start));
  // lprintf("The stack address during page fault handler is %p", (void *)(*(unsigned int *)(ureg_start + (18 * unsigned_int_size))));
  // MAGIC_BREAK;
  stack_ptr = ureg_start - pointer_size;
  *(unsigned int **)stack_ptr = (unsigned int *)ureg_start;
  stack_ptr -= pointer_size;
  *(unsigned int **)stack_ptr = kernel.current_thread->swexn_values.arg;
  stack_ptr -= pointer_size;
  
  // lprintf("ureg_start %p, stack start %p", ureg_start, kernel.current_thread->swexn_values.esp3);
  unsigned int *sw_eip = (unsigned int *)kernel.current_thread->swexn_values.eip;
  kernel.current_thread->swexn_values.esp3 = NULL;
  kernel.current_thread->swexn_values.eip = NULL;
  kernel.current_thread->swexn_values.arg = NULL;

  // lprintf("Running first thread with the correct values");
  // lprintf("eip %p stack %p", sw_eip, stack_ptr);
  // Make the sw exception handler run now by creating a trap frame
  run_first_thread((uint32_t)sw_eip, (uint32_t)stack_ptr, (uint32_t)get_eflags());

  // lprintf("create_stack_sw_exception(): ERROR! Should never reach here");
  assert(0);
  return 0;
}
