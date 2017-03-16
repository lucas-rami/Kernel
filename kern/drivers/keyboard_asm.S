/** @file keyboard_asm.S
 *  @brief The file which contains the handler of the keyboard interrupt
 *
 *  @author Anirudh Kanjani
 */

# Ensure that the symbol keyboard_interrupt_handler is accessible via C code
.global keyboard_interrupt_handler


keyboard_interrupt_handler:
        pusha				// Save the current state on the stack
        call keyboard_c_handler		// Call the C handler
        popa				// Restore the state from the stack
        iret				// Return from the handler

