/** @file keyboard.c
 *  @brief Implementation of the readchar API provided to the user to interact
 *  with the keyboard and some internal functions like the interrupt handler
 *  and the initializing function
 *
 *  This file contains the implementation of the reachar API and other
 *  internal functions needed by those APIs.
 *
 *  These functions help the user to convey data through the keyboard.
 *  The keyboard interrupt calls the keyboard_interrupt_handler function
 *  defined in keyboard_asm.h which in turn calls the keyboard_c_handler
 *  function defined here.
 *
 *  The keyboard_init function initializes the keyboard interrupt by
 *  registering the address of its handler in the IDT
 *
 *  The readchar function is the one available to the user to read from the
 *  buffer storing all the characters typed from the keyboard
 *
 *  @author Anirudh Kanjani
 */

#include <interrupts.h>
#include <keyhelp.h>
#include <asm.h>
#include <keyboard.h>
#include <queue.h>
#include "keyboard_asm.h"
#include <seg.h>
#include <interrupt_defines.h>
#include <stdio.h>

/** @brief keyboard initialization function
 *
 *   Installs the keyboard interrupt handler by registering its handler's
 *   address at the correct offset in the IDT
 *
 *   @param void
 *
 *   @return A negative error code on error, or 0 on success
 **/
int keyboard_init( void )
{
	return register_handler( (uintptr_t) keyboard_interrupt_handler, TRAP_GATE, 
		KEY_IDT_ENTRY, KERNEL_PRIVILEGE_LEVEL, SEGSEL_KERNEL_CS );
}

/** @brief The keyboard's C interrupt handler function
 *
 *   This is the function that is called via the assembly wrapper
 *   keyboard_interrupt_handler. It reads a byte from the keyboard
 *   port and enqueues it in the static keyboard buffer maintained by the
 *   kernel. It then acknowledges the PIC that the handler for keyboard
 *   has run and can service another keyboard_interrupt now.
 *
 *   @param void
 *
 *   @return void
 **/
void keyboard_c_handler( void )
{
	// Read the byte from the port
	uint8_t character = ( int )inb( KEYBOARD_PORT );

	// Store it in the static buf
	enqueue( character );

	// Ack the PIC
	outb( INT_CTL_PORT, INT_ACK_CURRENT );
}

/** @brief The API provided to the user to read the characters that were
 *   typed in on the keyboard.
 *
 *   This function looks at the static buffer for keyboard presses
 *   maintained by the kernel and dequeues a character and then feeds it to
 *   process_scancode API which is a state machine used to extract the
 *   actual character from the scan code that we get via the keryboard
 *   interrupts. If the queue is empty, it returns a negative value denoting
 *   error. If the scan_code represents a printable character and the
 *   release of the key is encountered, we return the character.
 *
 *   @param void
 *
 *   @return A negative error code on error, or the character read on success
 **/
int readchar( void )
{
	while( 1 ) {
		int ch = dequeue();
		if ( ch < 0 ) {
			// Dequeue error. The queue is empty
			break;
		}
		kh_type aug_char = process_scancode( ch );
		if ( KH_HASDATA( aug_char ) ) {
			// has a valid character
			if ( !KH_ISMAKE( aug_char ) ) {
				// key is released
				return KH_GETCHAR( aug_char );
			}
		}
	}
	return -1;
}
