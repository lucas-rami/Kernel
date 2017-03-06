
.global page_fault_handler


page_fault_handler:
        pusha				// Save the current state on the stack
        call page_fault_c_handler	// Call the C handler
        popa				// Restore the state from the stack
        iret				// Return from the handler

