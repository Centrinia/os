/* isrs.S */

	.code32

	.text

	.global isrs

	.align 64
isrs:
.rept 256
	.align 64,0x90,64
1:
	pusha
	movl	3f, %eax
	movl	2f, %edx
	pushl	%edx
	call	*%eax

	addl	$4, %esp
	popa
	iret
	. = 1b+56
2:
	.long 0xdeadbeef
3:
	.long dummy

.endr

	nop
	nop

	.align 8
dummy:
	nop
	ret

