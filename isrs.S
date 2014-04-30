/* isrs.S */

	.code32

	.text

	.global isrs

	.align 64
isrs:
.rept 256
//.rept 1
	.align 64,0x90,64
	cli
1:
	pushw	$0
	pusha
	popa
	iret

.endr
