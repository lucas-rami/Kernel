/** @file timer.c
 *  @brief Implementation of the functions required for the timer interrupt
 *
 *   This file contains the implementation of the c interrupt handler and
 *   initializing function for the time interrupt.
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
#include <interrupts.h>
#include "prechecks.h"
#include <scheduler.h>

#define REQUIRED_FREQUENCY 0.01
#define ONE_LSB_MASK 0xFF
#define SECOND_LSB_MASK 0xFF00
#define BITS_IN_ONE_BYTE 8
#define TICK_COUNT_START_VALUE 0

typedef struct timer_state {
	void ( *global_callback ) ( unsigned int );
	unsigned int global_counter;
} timer_state_t;

timer_state_t timer_state_;

/** @brief The function called by the interrupt handler for the timer interrupt
 *
 *   The handler for timer interrupts, timer_interrupt_handler defined in
 *   timer_asm.h calls this function after storing the current context on
 *   stack.
 *
 *  @param void
 *
 *  @return void
 **/
void timer_c_handler()
{
	// update the tick count
	timer_state_.global_counter++;

	// call the callback function
	timer_state_.global_callback( timer_state_.global_counter );

	// acknowledge the most recent interrupt to the PIC
	outb( INT_CTL_PORT, INT_ACK_CURRENT );

	make_runnable_and_switch();
}

/** @brief Initializes the timer and registers its handler with the IDT
 *
 *   This function configures the timer interrupt with the correct
 *   waveform and period for the timer. It also registers the handler for the
 *   interrupt at the particular IDT offset. Lastly, it initializes the
 *   state maintained by the timer to their default values and the callback
 *   function is also maintained as part of the state.
 *
 *  @param tickback A function pointer to the callback function for the timer
 *
 *  @return int Returns a negative error code on error, 0 on success
 **/
int timer_init( void ( *tickback )( unsigned int ) )
{
	if ( !is_valid_pointer( tickback ) ) {
		// Invalid function pointer to the callback function
		printf( "Pointer validation failed\n" );
		return -1;
	}
	if ( register_handler( (uintptr_t) timer_interrupt_handler, TRAP_GATE, TIMER_IDT_ENTRY,
		KERNEL_PRIVILEGE_LEVEL, SEGSEL_KERNEL_CS ) == -1 ) {
		// Some error trying to register the handler
		printf( "Registering timer handler failed\n" );
		return -1;
	}

	// Configure the timer with the correct waveform to use
	outb( TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE );

	// Calculate the period for the timer
	uint16_t number_of_cycles = REQUIRED_FREQUENCY / ( 1.0 / TIMER_RATE );

	uint8_t lsb = number_of_cycles & ONE_LSB_MASK;
	uint8_t msb = (number_of_cycles & SECOND_LSB_MASK ) >> BITS_IN_ONE_BYTE;

	// Configure the calculated period for the timer
	outb( TIMER_PERIOD_IO_PORT, lsb );
	outb( TIMER_PERIOD_IO_PORT, msb );

	// Initialize the timer state
	timer_state_.global_counter = TICK_COUNT_START_VALUE;
	timer_state_.global_callback = tickback;

	return 0;
}
