/** @file scheduler.h
 *  @brief This file contains the declaration for the functions related to
 *  thread scheduling
 *  @author akanjani, lramire1
 */

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <tcb.h>

void run_next_thread(tcb_t *current_tcb);
void make_runnable_and_switch(tcb_t *tcb);
void block_and_switch(tcb_t *tcb);

void add_runnable_thread(tcb_t *current_tcb);
void force_next_thread(tcb_t *current_tcb, tcb_t *force_next_tcb);

#endif /* _SCHEDULER_H_ */
