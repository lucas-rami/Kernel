/** @file 410user/progs/init.c
 *  @author ?
 *  @brief Initial program.
 *  @public yes
 *  @for p2 p3
 *  @covers fork exec wait print
 *  @status done
 */

#include <syscall.h>
#include <stdio.h>
#include <simics.h>

//#define TEST "actual_wait"
// #define TEST "fork_wait_bomb"
// #define TEST "make_crash"
// #define TEST "cho"
// #define TEST "slaughter"
// #define SLAUGHTER_ARG "cho"
// #define TEST "remove_pages_test1"
#define TEST "shell"

int main()
{
  int pid, exitstatus;
  char shell[] = TEST;
  char * args[] = {TEST/*, "2", "2", "42", SLAUGHTER_ARG*/, 0};

  while(1) {
    pid = fork();
    if (!pid)
      exec(shell, args);
    
    while (pid != wait(&exitstatus));
    // while(1);
    // lprintf("Test EXITED");
    // MAGIC_BREAK;
    // printf("Shell exited with status %d; starting it back up...", exitstatus);
  }
}
