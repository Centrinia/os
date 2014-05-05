

	.code16
#define E820_ENTRY_SIZE		24
#define SMAP			0x534d4150

	.global probe_e820
	/* Populate the address map.
	 * 2(%ebp): flat destination address. */
probe_e820:
	movl	%ebp, %eax
	movl	%esp, %ebp
	pushl	%eax
	pushl	%edi
	pushl	%ebx
	movw	%es, %cx
	pushl	%ecx


	movl	2(%ebp), %eax
	addl	$4, %eax
	movl	%eax, -8(%ebp)

	xorl	%ebx, %ebx
1:
	/* Generate the destination address. */
	movl	-8(%ebp), %eax
	movl	%eax, %ecx
	shrl	$4, %ecx
	andl	$0x0f, %eax
	movw	%cx, %es
	movw	%ax, %di

	/* Make the call. */
	movl	$0xe820, %eax
	movl	$SMAP, %edx
	movl	$E820_ENTRY_SIZE, %ecx
	int	$0x15

	jc	2f

	cmpl	$SMAP, %eax
	jnz	2f

	addl	$E820_ENTRY_SIZE, -8(%ebp)
	addl	$1, -4(%ebp)

	testl	%ebx, %ebx
	jnz	1b

	movl	-4(%ebp), %eax
	jmp	3f

2:
	xorl	%eax, %eax

3:
	/* Set the length field. */
	movl	2(%ebp), %edx
	movl	%edx, %ecx
	shrl	$4, %ecx
	andl	$0x0f, %edx
	movw	%cx, %es
	movw	%dx, %di

	movl	%eax, %es:(%di)

	popl	%ecx
	movw	%cx, %es
	popl	%ebx
	popl	%edi
	popl	%ebp

	ret

//smap:
//	.ascii "SMAP"
	//.ascii "PAMS"
