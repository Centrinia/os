/* boot.S */

#define BOOT_ADDRESS		0x7c00
#define BOOT_SEGMENT		0x7c0
#define STACK_ADDRESS		0x7a00


#define WIDTH	640
#define HEIGHT	480
#define VESA_MODE		0x111

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
	//.long	(WIDTH*HEIGHT/512+2+8)
	//.long	36

	.long 80

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

	.local drive_number, dst, lba, count
	drive_number = 16
	dst = 12
	lba = 8
	count = 4
	.local counter
	counter = -4

	xorl	%ecx, %ecx
1:
	movl	%ecx, counter(%bp)

	xorw	%dx, %dx
	movb	drive_number(%bp), %dl
	pushw	%dx
	pushw	$1

	// 32 bit LBA 
	add	lba(%bp), %ecx
	pushl	%ecx

	//pushw	$0
	//pushw	$(BOOT_ADDRESS+512)
	movl	dst(%bp), %ecx
	movl	counter(%bp), %edx
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

	movl	counter(%bp), %ecx

	addl	$1, %ecx
	cmpl	count(%bp), %ecx
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



	/*.org 510
	.byte 0x55, 0xaa*/
