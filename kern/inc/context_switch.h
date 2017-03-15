/** @file context_switch.h
 *  @brief This file contains the declarations for functions related to context
 *  switching
 *  @author akanjani, lramire1
 */

#include <tcb.h>
#include <stdint.h>

void context_switch(tcb_t* from, tcb_t* to);
