/** @file 410user/progs/fork_bomb.c
 *  @author zra
 *  @brief Tests fork() in low-memory conditions.
 *  @public yes
 *  @for p3
 *  @covers fork oom
 *  @status done
 */

/* Includes */
#include <syscall.h>    /* for fork */
#include <stdlib.h>
#include "410_tests.h"
#include <report.h>
#include <syscall.h>

DEF_TEST_NAME("fork_bomb:");

/* Main */
int main() {
    report_start(START_4EVER);
    TEST_PROG_ENGAGE(200);

    int ret = 1;

    while (ret != 0) {
        ret = fork();
        if (ret == 0) lprintf("New thread's tid is %d", gettid());
    }

    while(1) {
        continue;
    }

}
