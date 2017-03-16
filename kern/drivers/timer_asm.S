/** @file timer_asm.S
 *  @brief The file which contains the handler of the timer interrupt
 *
 *  @author Anirudh Kanjani
 */

# Ensure that the symbol timer_interrupt_handler is accessible via C code
.global timer_interrupt_handler

timer_interrupt_handler:
	pusha			// Save the current state on the stack
	call timer_c_handler	// Call the C handler
	popa			// Restore the state on the stack
	iret			// Return from the handler

