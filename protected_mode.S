/* protected_mode.S */

	.code32
	.align	16
	.global protected_mode

protected_mode:
	movl	$0x10, %eax
	movl	%eax, %ds
	movl	%eax, %es
	movl	%eax, %fs
	movl	%eax, %gs
	movl	%eax, %ss


	/*movw	$0x4f02, %ax
	movw	$0x101, %bx
	int	$0x10*/

	movw	$0x18, %di
	ltr	%di


	//sti

	call	debug

1:
	int	$0x0d
	jmp	1b

	.end

