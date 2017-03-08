/** @file interrupts.c
 *  @brief Implementation of APIs provided to the user to install the 
 *  keyboard, timer interrupts and initialize the console
 *
 *  This file contains the implementation of the handler_install API and
 *  register_handler function which help the user to setup their timer,
 *  keyboard interrupts and the console.
 *
 *  @author Anirudh Kanjani
 */

#include <stdio.h>
#include <timer_defines.h>
#include <interrupt_defines.h>
#include <asm.h>
#include <ctype.h>
#include "timer_asm.h"
#include <seg.h>
#include <simics.h>

#include <timer.h>
#include <console.h>
#include <interrupts.h>
#include <keyboard.h>

#define NUM_32BIT_INT_PER_IDT_ENTRY 2

/** @brief The driver-library initialization function
 *
 *   Installs the timer and keyboard interrupt handler.
 *   NOTE: The console has to be initialized or cleared in case the user 
 *   decides not to call handler_install and not use the timer or the keyboard.
 *
 *   @param tickback Pointer to clock-tick callback function
 *   
 *   @return A negative error code on error, or 0 on success
 **/
int handler_install( void ( *tickback )( unsigned int ) )
{
        // Initialize the console
        console_init();
                
        // Initializes the timer
        if ( timer_init( tickback ) == -1 ) { 
		printf( "Timer init failed\n" );
		return -1;
	}

        // Initializes the keyboard
        if ( keyboard_init() == -1 ) {
		printf( "Keyboard init failed\n" );
		return -1;
	}

        return 0;
}

/** @brief The driver-library registering function
 *
 *   Makes an entry in the IDT for the timer and keyboard interrupt handlers.
 *
 *   @param handler_function Pointer to the interrupt handler function
 *   @param gate_type A uint8_t type specifying the type of the gate i.e. 
 *   interrupt/trap etc.
 *   @param idt_offset Offset to the starting address of the IDT for this 
 *   interrupt
 *   @param privilege_level The uint8_t type specifying the privilege_level 
 *   of the handler. Anything other than KERNEL_PRIVILEGE_LEVEL throws an 
 *   error as of now.
 *   @param segment A uint16_t specifying the segment to be selected for the
 *   target code segment. Anything other than SEGSEL_KERNEL_CS throws an
 *   error as of now.
 *   
 *   @return A negative error code on error, or 0 on success
 **/
int register_handler( void ( *handler_function ) ( void ), uint8_t gate_type, 
	uint32_t idt_offset, uint8_t privilege_level, uint16_t segment )
{
	// Sanity precheck
	if (segment != SEGSEL_KERNEL_CS || gate_type != TRAP_GATE ) {
		printf( "Either privilege level is not KERNEL_PRIVILEGE_LEVEL" 
			"or segment is not SEGSEL_KERNEL_CS\n" );
		return -1;
	}

	// Extract the IDT base and create the IDT entry
	uint32_t *idt = ( uint32_t * )idt_base();
	uintptr_t offset = ( uintptr_t )handler_function;
	uint32_t offset_lower = offset & TWO_LSB_MASK;
	uint32_t offset_upper = ( offset & TWO_MSB_MASK ) >> BITS_IN_TWO_BYTES;
	uint32_t interrupt_gate_descriptor_upper = 
		( ( offset_upper << BITS_IN_TWO_BYTES ) | WORKING_GATE | 
		SIZE_GATE_32 | ((int)privilege_level << 13));
	interrupt_gate_descriptor_upper |= TRAP_GATE_IDENTIFIER;

	// Set the higher 32 bits for the IDT entry for this handler
	*( idt + NUM_32BIT_INT_PER_IDT_ENTRY * idt_offset + 1 ) = 
		interrupt_gate_descriptor_upper;

	// Make the lower 32 bits of the entry
	uint32_t shifted = ( uint32_t )segment << BITS_IN_TWO_BYTES;
	uint32_t interrupt_gate_descriptor_lower = ( shifted | offset_lower );

	// Set the lower 32 bits of the entry
	*( idt + NUM_32BIT_INT_PER_IDT_ENTRY * idt_offset ) = 
		interrupt_gate_descriptor_lower;
	return 0;
}

