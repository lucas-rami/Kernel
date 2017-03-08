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

void get_tid_and_print() {

  lprintf("Entered get_tid_and_print()");

  int tid = gettid();
  lprintf("TID is %d", tid);

}

int main() {

  lprintf("Idle task running !");
  get_tid_and_print();
  while (1) {
    continue;
  }
}
