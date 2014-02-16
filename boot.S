
#define BOOT_ADDRESS		0x7c00
#define STACK_ADDRESS		0x7a00


#define KERNEL_SECTOR_COUNT	256

	.text
	.code16

	.globl _start

	.org 0
_start:
	jmp	start0
	nop


start0:
	xorw	%cx, %cx
	movw	%cx, %ss
	movw	%cx, %ds
	movw	%cx, %es


/* Set up the stack. */
	movw	$STACK_ADDRESS, %sp


	// Drive number is in %dl
	pushw	%dx
	// Read one sector
	pushw	$1

	//pushw	$(KERNEL_SECTOR_COUNT-1)

	// 32 bit LBA of 1
	pushl	$1

	//pushw	$0
	//pushw	$(BOOT_ADDRESS+512)
	pushw	$0x7e0
	pushw	$0

	call	read_sector

	jmp	kernel
	nop

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

	// ax: maximum sector number
	movw	%ax, -10(%bp)

	// cx: cylinder limit
	movw	%cx, -8(%bp)

	// dh,dl: number of hard drives and head limit
	movw	%dx, -6(%bp)
	

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
	movw	%ax, -14(%bp)
	// Cylinder number
	movw	%dx, -16(%bp)
	/* ah: 2
	   al: number of sectors
	   ch: cylinder
	   cl: sector
	   dh: head
	   dl: drive
	   es:bx: buffer address
	*/
	// Construct the cyllinder,head value in %cx
	movw	-16(%bp), %cx
	shlw	$6, %cx
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

