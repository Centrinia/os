
#define BOOT_ADDRESS		0x7c00
#define BOOT_SEGMENT		0x7c0
#define STACK_ADDRESS		0x7a00



	.text
	.code16

	.globl _start

	.org 0
_start:
	jmp	start0
	nop


	.org	8
sector_count:
	// 6144+4
	//.long	(640*480/512+2+2)
	.long	(640*480/512+2+2)
	//.long	36

	.org	12
start0:
	xorw	%cx, %cx
	movw	%cx, %ss
	movw	%cx, %ds
	movw	%cx, %es


/* Set up the stack. */
	movw	$STACK_ADDRESS, %sp

	movl	sector_count,%eax
	pushw	%dx
	pushl	$0x7e00
	pushl	$1
	pushl	%eax

	call	read_sectors
	sub	$12, %sp

	jmp	kernel
	nop

/*
   16(%bp): drive number
   12(%bp): 32 bit address of the destination buffer
   8(%bp): Starting LBA
   4(%bp): Number of sectors to read
 */
read_sectors:
	push	%bp
	mov	%sp, %bp
	pusha
	sub	$8, %sp

	xorl	%ecx, %ecx
1:
	movl	%ecx, -4(%bp)

	xorw	%dx, %dx
	movb	16(%bp), %dl
	pushw	%dx
	pushw	$1

	// 32 bit LBA 
	add	8(%bp), %ecx
	pushl	%ecx

	//pushw	$0
	//pushw	$(BOOT_ADDRESS+512)
	movl	12(%bp), %ecx
	movl	-4(%bp), %edx
	shll	$9, %edx
	addl	%edx, %ecx

	movl	%ecx, %edx
	andl	$0x1fff, %edx
	xorl	%edx, %ecx
	shrl	$4, %ecx

	pushw	%cx
	pushw	%dx

	call	read_sector
	add	$12, %sp

	movl	-4(%bp), %ecx

	addl	$1, %ecx
	cmpl	4(%bp), %ecx
	jne	1b

	add	$8, %sp
	popa
	pop	%bp
	ret



	/*	
		
		14(%bp):	drive number
		12(%bp):	number of sectors to read
		8(%bp):	logical sector number
		6(%bp):	destination segment
		4(%bp):	destination offset
		-4(%bp): %es
		
	*/
read_sector:
	push	%bp
	mov	%sp, %bp
	pusha
	sub	$32, %sp

	movw	%es, %cx
	movw	%cx, -4(%bp)

	xorw	%cx, %cx
	movw	%cx, %es
	movw	%cx, %di


	movb	$0x08, %ah
	movb	14(%bp), %dl
	int	$0x13
	jc	read_error

	mov	%cx, %ax

	shr	$6, %cx
	and	$0x3ff, %cx
	and	$0x3f, %ax

	// ax:sectors per track
	movw	%ax, -10(%bp)

	// cx: cylinder limit
	movw	%cx, -8(%bp)

	// dh,dl: number of hard drives and head limit
	addb	$1, %dh
	movb	%dh, -6(%bp)
	

	/* sector: lba % sectors_per_track + 1
	   head: lba / sectors_per_track % heads_per_cylinder
	   cylinder: lba / sectors_per_track / heads_per_cylinder
	*/



	// Divide the LBA by the number of sectors per track.
	xorb	%bh, %bh
	movb	-10(%bp), %bl
	movw	8(%bp), %ax
	movw	10(%bp), %dx

	divw	%bx
	// (dx,ax): (lba / sectors_per_track, lba % sectors_per_track)

	// sector number
	add	$1, %dx
	movw	%dx, -12(%bp)

	// Divide the track number by the number of heads per cylinder
	xorb	%bh, %bh
	xorw	%dx, %dx
	movb	-6(%bp), %bl
	divw	%bx
	
	// head number
	movw	%dx, -14(%bp)
	// Cylinder number
	movw	%ax, -16(%bp)
	/* ah: 2
	   al: number of sectors
	   ch: cylinder
	   cl: sector
	   dh: head
	   dl: drive
	   es:bx: buffer address
	*/
	// Construct the cyllinder,head value in %cx
	movb	-16(%bp), %ch
	movb	-17(%bp), %cl
	shlb	$6, %cl
	movb	-12(%bp), %al
	andb	$0x3f, %al
	or	%al, %cl

	movb	14(%bp), %dl
	movb	-14(%bp), %dh
	movb	12(%bp), %al
	movb	$0x02, %ah

	// Do the read.
	movw	6(%bp), %bx
	movw	%bx, %es
	movw	4(%bp), %bx
	
	int	$0x13

	jc	read_error

read_error:
	movw	-4(%bp), %cx
	movw	%cx, %es

	add	$32, %sp
	popa
	pop	%bp
	ret



	.org 510
	.byte 0x55, 0xaa


	/* The rest of the kernel. */
kernel:
	//movw	$0x0013, %ax
	//int	$0x10
#define WIDTH	640
#define HEIGHT	480
#define VESA_MODE		0x111

	movw	$0x4f02, %ax
	movw	$VESA_MODE, %bx
	int	$0x10




	xorw	%ax, %ax
	movw	%ax, %es
	movw	$vesa_structure, %di
	movw	$0x4f01, %ax
	movw	$VESA_MODE, %cx
	int	$0x10

	xorl	%eax, %eax
	movw	wingran, %ax
	shll	$10, %eax

	movl	%eax, bank_size

	movl	$WIDTH, %eax
	movl	$HEIGHT, %ebx
	shll	$2, %eax
	mull	%ebx

	movl	bank_size, %ebx
	divl	%ebx
	xorl	%ebx, %ebx
	subl	%edx, %ebx
	sbbl	%edx, %edx
	negl	%edx
	addl	%edx, %eax
	movl	%eax, bank_count

	/*movw	$0x4f08, %ax
	movb	$0, %bl
	movb	$8, %bh
	int	$0x10*/


	movw	$0x4f08, %ax
	movb	$1, %bl
	//movb	$32, %bh
	int	$0x10

	// Set palette.
	movw	$0x4f09, %ax
	movb	$0, %bl
	movw	$0, %dx
	movw	%ds, %cx
	movw	%cx, %es
	movw	$palette, %di
	movw	$256, %cx
	int	$0x10

	xorl	%edi, %edi
	xorl	%esi, %esi

	movw	$0xa000, %bx
	movw	%bx, %es

	xorl	%ecx, %ecx
1:
	movl	$img, %ebx
	addl	%ecx, %ebx
	movl	%ebx, %edx
	andl	$0x1fff, %edx
	xorl	%edx, %ebx
	shr	$4, %ebx
	movw	%bx, %ds

	xorl	%eax, %eax
	movb	(%edx), %al
	shl	$1, %ax

	movl	$palette, %ebx
	addl	%eax, %ebx

	movl	%ebx, %edx
	andl	$0x1fff, %edx
	xorl	%edx, %ebx
	shr	$4, %ebx
	movw	%bx, %ds


	movw	(%edx), %ax


	xorw	%bx, %bx
	movw	%bx, %ds

	movw	%ax, %es:(%di)

	addl	$1,%ecx
	cmpl	imgsize, %ecx
	jnc	5f

	addl	$2, %edi
	cmp	bank_size, %edi
	jnc	3f

	jmp	1b

3:
	subl	bank_size, %edi
	addl	$1, %esi

	cmp	bank_count, %esi
	jnc	2f
4:

	movw	$0x4f05, %ax
	xorw	%bx, %bx
	movw	%si, %dx
	int	$0x10

	jmp	1b
	
2:
	xorl	%esi, %esi

	jmp	4b


5:
	jmp	5b

bank_size:
	.long 0x10000
bank_count:
	.long 3


imgsize:
	//.long 3145728
	.long	307200

vesa_structure:
	.short 0
	.byte 0,0
	//.short 0
wingran:
	.short 0

	. = vesa_structure + 128

	. = _start + 512*8
palette:

	. = palette + 512
img:
	.long 0

