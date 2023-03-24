# syscallasm.S - Create system linkage between user / kernel
# vim:ts=4 noexpandtab


# NUM_SYS_CALLS: 10

# before calling:
#	 EAX: Call Number
#	 EBX: Arg 1 - left
#	 ECX: Arg 2
#	 EDX: Arg 3 - right
.text
 .globl call_handler
call_handler:
	
	pushl %esp
	pushl %ebp
	pushl %edi
	
	
	cmpl $10, %eax		# valid number of arguments
	jg badcode
	cmpl $0, %eax
	jle badcode
	pushfl
	pushl %edx
	pushl %ecx
	pushl %ebx
	call *jump_table(,%eax,4)
	popl %ebx
	popl %ecx
	popl %edx
	popfl

	
	popl %edi
	popl %ebp
	popl %esp

	jmp done

jump_table:
	.long 0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn


done:
	
	iret
	
badcode:
	movl $-1, %eax	# return error
	addl $12, %esp	# pop the 3 values off the register

	iret
