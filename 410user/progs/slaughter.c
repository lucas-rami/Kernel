/** @file 410user/progs/slaughter.c
 *  @author mtelgars
 *  @author nwf
 *  @brief Runs a lot of copies of a given test.
 *  @public yes
 *  @for p3
 *  @covers fork exec wait set_status vanish
 *  @status done
 */

#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "410_tests.h"
#include <report.h>

DEF_TEST_NAME("slaughter:");

unsigned int width;
int initial_tid;
int exit_status;

void __attribute__((noreturn)) slaughter(char **cmd, unsigned int depth)
{
  int curtid = gettid();

  report_start(START_CMPLT);

  if ( ! depth )
  {
    lprintf("Depth is 0");
    if (exec(cmd[0], cmd))
    {
      lprintf("=====> exec failed from %d!", curtid);
      exit(-373);
    }
  }
  else
  {
    unsigned int i, j, matched;
    int threads[2*width]; /* not doing anything smart here yet.. */
    int status, tid;

    for (i = 0; i != 2*width; ++i)
    {
      // MAGIC_BREAK;
      /* lprintf("tid %d cmd[0] %p cmd[1] %p cmd[2] %p", gettid(), cmd[0], cmd[1], cmd[2]);
      int k = 0;
      while(cmd[k] != NULL) {
        lprintf("Arg %d is %s", k, cmd[k]);
        k++;
      }*/
      if ( ! (threads[i] = fork()) )
      {
        /* alternate immediate exec and slaughter */
        /* lprintf("tid %d cmd[0] %p cmd[1] %p cmd[2] %p", gettid(), cmd[0], cmd[1], cmd[2]);
        lprintf("Child calling slaughter again");
        int j = 0;
        while(cmd[j] != NULL) {
          lprintf("Arg %d is %s", j, cmd[j]);
          j++;
        }*/
        slaughter(cmd, (i % 2) * (depth - 1));
      }
      else if (threads[i] < 0) {
        lprintf("=====> XXX tid %d could not fork (err=%d)!!",
                curtid,threads[i]);
        report_end(END_FAIL);
        exit(exit_status+1);
      }
      else
      {
        lprintf("=====> Slaugher spawn created TID %d from TID %d",
                threads[i], curtid);
      }
    }

    for (i = 0; i != 2*width; ++i)
    {
      tid = wait(&status);
      if ( tid < 0 )
      {
        lprintf("=====> XXX Wait failed in TID %d with %d; bombing out",
                curtid, tid );
        report_end(END_FAIL);
        exit(exit_status+3);
      }
      else
      {
        matched = 0;
        for ( j = 0; j != 2*width; ++j )
        {
          if ( threads[j] == tid )
          {
            matched = 1;
            threads[j] = -1;
            break;
          }
        }
        if ( matched == 0 )
        {
          lprintf("=====> XXX Wait returned bad TID %d to TID %d; bombing out",
                   tid, curtid );
          report_end(END_FAIL);
          exit(exit_status+5);
        }
        lprintf("=====> %stid %d gave %svalid status code of %d to tid %d",
          (status == exit_status) ? "" : "XXX ",
          tid, (status == exit_status) ? "" : "IN", status, curtid);
        if ( status != exit_status )
        {
          report_end(END_FAIL);
          exit(exit_status+7);
        }
      }
    }
  }

  lprintf("=====> Slaughterer %d completed", curtid);
  report_end(END_SUCCESS);
  exit(exit_status);
}

int main(int argc, char *argv[])
{
  unsigned int initial_depth;
  char *default_cmd[] = { "print_basic", NULL };
  char **cmd;

  if (argc < 5)
  {
    printf("slaughter_spawn <recursive depth> "
            "<recursive width> <exit status> <file> [args]\n");
    printf(" DEFAULTING TO: 2 2 0 print_basic\n");
    cmd = default_cmd;
    width = 2;
    initial_depth = 2;
    exit_status = 0;
  }
  else if ( ! (width = strtoul(argv[1], NULL, 10)) )
  {
    printf(" bad recursive width\n");
    return -2;
  }
  else if ( ! (initial_depth = strtoul(argv[2], NULL, 10)) )
  {
    printf(" bad recursive depth\n");
    return -3;
  }
  else
  {
    exit_status = strtol(argv[3], NULL, 10);
    cmd = &argv[4];
  }
  /* lprintf("Arg %p argv[0] %s argv[1] %s argv[2] %s argv[3] %s argv[4] %s argv[5] %d", argv, argv[0], argv[1], argv[2], argv[3], argv[4], (int)argv[5]);

  MAGIC_BREAK;
  lprintf("cmd %p, value at cmd %s.", cmd, *cmd); 
  lprintf("cmd+1 %p", cmd+1);
  MAGIC_BREAK;
  *(cmd+1) = 0;
  lprintf("Value at cmd+1 %s", *(cmd + 1));
  // cmd[1] = 0;
  int i = 0;
  while (cmd[i] != NULL) {
    lprintf("i %d cmd[i] %s", i, cmd[i]);
    i++;
  }*/
  slaughter(cmd, initial_depth);
  return 0;
}
