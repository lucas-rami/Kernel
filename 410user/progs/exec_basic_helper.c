/** @file 410user/progs/exec_basic_helper.c
 *  @author mpa
 *  @brief Run by exec_basic to test exec.
 *  @public yes
 *  @for p3
 *  @covers nothing
 *  @status done
 *  @note Helper.
 */

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <simics.h>
#include "410_tests.h"

DEF_TEST_NAME("exec_basic:");
#define DELAY (16*1024)

int gcc_please_do_not_optimize_too_much = 0;

void foo() {
  ++gcc_please_do_not_optimize_too_much;
}

/* this function eats up a bunch of cpu cycles */
void slow() {
  int i;

  for (i = 0; i < DELAY; i++) foo();
}

int main()
{
  // REPORT_MISC("exec_basic_helper main() starting...");
  // REPORT_END_SUCCESS;
  lprintf("Exec basic helper");
  // MAGIC_BREAK;
  while(1) {
  slow();
  lprintf("Exec basic helper");
  }
  // exit(1);
}
