.global invalidate_tlb

invalidate_tlb:
    movl 4(%esp), %eax
    invlpg (%eax)         // Invalidate the TLB entry
    ret                   // Return from procedure