#include <virtual_memory.h>
#include <syscalls.h>
#include <stddef.h>
#include <string.h>
#include <kernel_state.h>
#include <cr.h>
#include <task_create.h>
#include <malloc.h>
#include <simics.h>
#include <scheduler.h>
#include <asm.h>

#define ERR_INVALID_ARGS -1
// TODO: What should this value be? How about max no of args
#define ARGS_MAX_SIZE 128
#define STACK_TOP 0xFFFFFFFF
#define STACK_START_ADDR 0xfffff000
#define TRUE 1
#define FALSE 0

int kern_exec(char *execname, char **argvec) {
  // Validate all arguments
  //lprintf("Kern exec. ");
  //MAGIC_BREAK;
  // lprintf("and the string is %s", esi_reg, *esi_reg);
  // TODO: Should be done after validation
  if (is_valid_string(execname) == FALSE) {
    lprintf("Execname not valid");
    return ERR_INVALID_ARGS;
  }
  lprintf("Execname has been validated");
  int i = 0;
  while (argvec[i] != NULL) {
    if (is_valid_string(argvec[i]) == FALSE || strlen(argvec[i]) > ARGS_MAX_SIZE) {
      lprintf("Invalid args");
      return ERR_INVALID_ARGS;
    }
    i++;
  }

  // unsigned int *prev_page_table_dir = (unsigned int *)get_cr3();
  disable_interrupts();
  create_task_from_executable(execname, TRUE, argvec, i);
  lprintf("Exec should work now");
  // setup_vm sets cr3 to the new page table dir
  // unsigned int *new_page_table_dir = get_cr3();
  // TODO: Do we really need to set the cr3 value in setup vm?
  // set_cr3((uint32_t)prev_page_table_dir);
  // char *new_stack_addr = load_args_for_new_program(argvec, new_page_table_dir, i);
  kernel.current_thread->cr3 = (uint32_t)get_cr3();
  // run_next_thread();
  lprintf("EXEC. The current tcb is %p and the esp is %p", kernel.current_thread, (char*)kernel.current_thread->esp);
  // make_runnable_and_switch();
  switch_esp(kernel.current_thread->esp);
  lprintf("This shouldnt be printed");
  return 0;
  // Copy the arguments to the new memory space
  // Make the stack for the main thread with proper args
  // First argument should be the count of the strings in argvec
  // Second should be an array of strings(the pointers should be valid in this new
  // address space
  // Third and fourth arguments should be the stack high and low for this program
  // TODO: Make sure that argvec[0] is the same as execname
  // TODO: Think of a reasonable limit on the number of args in argvec
  // On success, switch to the second thread after deallocating space for the 
  // first task 
  // On failure, clean up whatever space we have allocated for this new task
  // and return a negative value
  // No software exception handler should be registered. Do something to make
  // that happen
}

char *load_args_for_new_program(char **argvec, unsigned int *old_ptd, int count/*, char *execname*/) {
  unsigned int *new_ptd = (unsigned int *)get_cr3();
  char *stack_addr = (char *)STACK_TOP;
  char *buf = malloc(sizeof(char) * (ARGS_MAX_SIZE + 1));
  char **args_addr = malloc(sizeof(char*) * (count/* + 1*/));
  int i = 0, len;
  
  /*len = strlen(execname);
  memcpy(buf, execname, len + 1);
  stack_addr -= (len + 1);
  args_addr[0] = stack_addr;
  set_cr3(new_ptd);
  memcpy(stack_addr, buf, len + 1);
  set_cr3(old_ptd);
  */
  set_cr3((uint32_t)old_ptd);
  while (argvec[i] != NULL) {
    // TODO: Is this a TOCTOU problem?
    len = strlen(argvec[i]);
    memcpy(buf, argvec[i], len + 1);
    stack_addr -= (len + 1);
    args_addr[i/* + 1*/] = stack_addr;
    set_cr3((uint32_t)new_ptd);
    memcpy(stack_addr, buf, len + 1);
    set_cr3((uint32_t)old_ptd);
    i++;
  }
  // args_addr[i] = NULL;
  stack_addr -= (sizeof(char*) * (count/* + 1*/));
  char *start_of_argv = stack_addr;
  set_cr3((uint32_t)new_ptd);
  memcpy(stack_addr, args_addr, (sizeof(char*) * (count/* + 1*/)));
  unsigned int ptr_size = sizeof(void*);
  stack_addr -= ptr_size;
  *(char **)stack_addr = (char*)STACK_START_ADDR;
  stack_addr -= ptr_size;
  *(char **)stack_addr = (stack_addr - (ptr_size + sizeof(int)));
  stack_addr -= ptr_size;
  *(char **)stack_addr = start_of_argv;
  stack_addr -= sizeof(int);
  *(int *)stack_addr = (count/* + 1*/);
  lprintf("The count of args is %d", *(int *)stack_addr);
  // char **argv = *((int *)stack_addr + i);
  char **printstr = *(char ***)(stack_addr + sizeof(int));
  for (i = 0; i < count; i++) {
    lprintf("Arg num %d, val %s", i, *(printstr + i));
  }
  // set_cr3(old_ptd);

  return stack_addr;
}
  
