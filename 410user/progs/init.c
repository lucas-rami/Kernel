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

// #define TEST "actual_wait"
// #define TEST "fork_exit_bomb"
#define TEST "make_crash"
// #define TEST "cho"
// #define TEST "remove_pages_test2"

int main()
{
  int pid, exitstatus;
  char shell[] = TEST;
  char * args[] = {TEST, 0};

  while(1) {
    pid = fork();
    if (!pid)
      exec(shell, args);
    
    while (pid != wait(&exitstatus));
    // while(1);
    // MAGIC_BREAK;
    // printf("Shell exited with status %d; starting it back up...", exitstatus);
  }
}
