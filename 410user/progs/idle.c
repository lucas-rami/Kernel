/** @file 410user/progs/idle.c
 *  @author ?
 *  @brief Idle program.
 *  @public yes
 *  @for p2 p3
 *  @covers
 *  @status done
 */
#include <simics.h>
#include <syscall.h>

int main()
{
    lprintf("Running thread with ID %d\n", gettid());
    /*int pid = fork();
    if (pid == 0) {
      lprintf("Child process\n");
      while(1) {
      }
    }
    lprintf("Parent process\n");
    */while (1) {
    }
}
