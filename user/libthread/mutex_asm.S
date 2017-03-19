.global atomic_add_and_update

atomic_add_and_update:
	movl 0x4(%esp), %ecx	# Move the first argument to ecx
	movl 0x8(%esp), %eax	# Move the second argument to eax
	lock xadd %eax, (%ecx)	# Atomically add the value in eax to 
				# the memory address pointed to by ecx
				# and retrieve the old value of it in eax
	ret 			# return 
