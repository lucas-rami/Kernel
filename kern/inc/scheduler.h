/** @file scheduler.h
 *  @brief This file contains the declaration for the functions related to
 *  thread scheduling
 *  @author akanjani, lramire1
 */

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <tcb.h>

tcb_t *next_thread();
void make_runnable_and_switch();
void block_and_switch(int is_descheduled);

void add_runnable_thread(tcb_t *tcb);
void force_next_thread(tcb_t *force_next_tcb);

#endif /* _SCHEDULER_H_ */
