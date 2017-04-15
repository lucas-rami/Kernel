/** @file 410user/progs/coolness.c
 *  @author de0u
 *  @brief Tests fork, exec, and context-switching.
 *  @public yes
 *  @for p3
 *  @covers fork gettid exec
 *  @status done
 */

#include <syscall.h>
#include <simics.h>


int main() {

  char *program = "fork_wait_bomb";
  char *args[2];
  int pid = fork();
  args[0] = program;
  args[1] = 0;
  // fork();
  if (pid == 0) 
    exec(program, args);

  while(1);
      lprintf("ULTIMATE BADNESS");
}
