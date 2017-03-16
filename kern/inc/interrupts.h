/** @file interrupts.h
 *  @brief The file which contains the defintions of the helper functions
 *   needed by all the interrupts to register their handlers with the IDT.
 *
 *  @author Anirudh Kanjani
 */

#ifndef __INTERRUPTS_H_
#define __INTERRUPTS_H_

#include <stdint.h>

#define WORKING_GATE 0x8000
#define DPL_HARDWARE_INTERRUPTS 0x0
#define TRAP_GATE_IDENTIFIER 0x700
#define INTERRUPT_GATE_IDENTIFIER 0x600
#define SIZE_GATE_32 0x800
#define TWO_LSB_MASK 0xFFFF
#define TWO_MSB_MASK 0xFFFF0000
#define BITS_IN_TWO_BYTES 16
#define TRAP_GATE 0
#define INTERRUPT_GATE 1
#define TASK_GATE 2
#define USER_PRIVILEGE_LEVEL 3
#define KERNEL_PRIVILEGE_LEVEL 0
#define USER_PRIVILEGE_MASK 0x6000

int register_handler(uintptr_t handler_function, uint8_t gate_type,
                     uint32_t idt_offset, uint8_t privilege_level,
                     uint16_t segment);

/*********************************************************************/
/*                                                                   */
/* Interface for device-driver initialization and timer callback     */
/*                                                                   */
/*********************************************************************/

/** @brief The driver-library initialization function
 *
 *   Installs the timer and keyboard interrupt handler.
 *   NOTE: handler_install should ONLY install and activate the
 *   handlers; any application-specific initialization should
 *   take place elsewhere.
 *
 *   @param tickback Pointer to clock-tick callback function
 *
 *   @return A negative error code on error, or 0 on success
 **/
int handler_install(void (*tickback)(unsigned int));

#endif
