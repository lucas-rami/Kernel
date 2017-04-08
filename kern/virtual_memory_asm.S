.global invalidate_tlb

invalidate_tlb:
    invlpg 4(%esp)      // Invalidate the TLB entry
    ret                 // Return from procedure