
	.text
	.code16

	.globl _start

	.org 0
_start:
	//jmp	0x7c00+start
	jmp	start0
	nop
start0:
	/*xorw	%cx, %cx
	movw	%cx, %ss
	movw	%cx, %ds
	movw	%cx, %es*/



	movw	$0xb800, %ax
	movw	%ax, %ds

	mov	$message, %si
	xor	%di, %di
	xor	%dx, %dx
	mov	$0x1e, %dh

1:
	mov	%cs:(%si), %dl
	mov	%dx, (%di)
	add	$2, %di


	inc	%si
	test	%dl, %dl
	jnz	1b

1:
	jmp	1b

message:
	.asciz "Hello World!"

	.org 510
	.byte 0x55, 0xaa
