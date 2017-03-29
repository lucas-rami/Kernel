/** @file 410user/progs/idle.c
 *  @author ?
 *  @brief Idle program.
 *  @public yes
 *  @for p2 p3
 *  @covers
 *  @status done
 */

int main() {
  int cnt = 0;
  int pid = fork();
  lprintf("Running thread with ID %d\n", gettid());
  while (1) {
    if (cnt % 3000000 == 0) {
      if (pid == 0) {
        lprintf("Child process\n");
      } else {
        lprintf("Parent process\n");
      }
      lprintf("tid : %d", gettid());
      cnt = 0;
    }
    ++cnt;
  }
  return 0;
}
