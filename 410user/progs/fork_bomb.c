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
#include <simics.h>

DEF_TEST_NAME("fork_bomb:");

/* Main */
int main() {
    report_start(START_4EVER);
    TEST_PROG_ENGAGE(200);

    int ret = 0;
    int stop = 0;

    while (ret == 0) {
        ret = fork();
        if (ret < 0) {
            lprintf("fork_bomb(): fork() returned an error");
            while(1);
        }
        if (ret == 0) lprintf("Hello world, I am thread %d", gettid());
    }

    deschedule(&stop);

    lprintf("NOPE NOPE NOPE NOPE");
    while(1);

}
