/** @file context_switch.h
 *  @brief This file contains the declarations for functions related to context
 *  switching
 *  @author akanjani, lramire1
 */

#ifndef _CONTEXT_SWITCH_H_
#define _CONTEXT_SWITCH_H_

#include <tcb.h>
#include <stdint.h>

void context_switch(tcb_t* to);
void init_thread(tcb_t* to);

#endif /* _CONTEXT_SWITCH_H_ */
