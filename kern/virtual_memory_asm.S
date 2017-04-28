/** @file virtual_memory_asm.S
 *  @brief  This file contains the definition for the function used to 
 *          invalidate an entry in the TLB
 *  @author akanjani, lramire1
 */

.global invalidate_tlb

invalidate_tlb:
    movl 4(%esp), %eax
    invlpg (%eax)         // Invalidate the TLB entry
    ret                   // Return from procedure