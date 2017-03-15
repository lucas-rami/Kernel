/** @file context_switch.h
 *  @brief This file contains the definitions for functions related to context
 *  switching
 *  @author akanjani, lramire1
 */

#include <cr.h>
#include <tcb.h>
#include <stdlib.h>
#include <context_switch_asm.h>

void context_switch(tcb_t* from, tcb_t* to) {

  if (from == NULL || to == NULL) {
    //TODO: panic
  }

  uint32_t cr3 = get_cr3();

  context_switch_asm(cr3, &from->esp, to->esp);

}
