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

int main()
{
  int pid, exitstatus;
  char shell[] = "actual_wait";
  char * args[] = {"actual_wait", 0};

  while(1) {
    pid = fork();
    if (!pid)
      exec(shell, args);
    
    // while (pid != wait(&exitstatus));
    while(1);
  
    printf("Shell exited with status %d; starting it back up...", exitstatus);
  }
}
