/* boot.S */

#define BOOT_ADDRESS		0x7c00
#define BOOT_SEGMENT		0x7c0
#define STACK_ADDRESS		0x7a00
#define SECTOR_SIZE		512

	.text
	.code16

	.globl _start

	.org 0
_start:
	jmp	start0
	nop

	.org	8
sector_count:
	.long (512*1024-0x7c00)/512

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
	pushl	$(BOOT_ADDRESS+SECTOR_SIZE)
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

#define cylinder_number		-16(%bp)
#define head_number		-14(%bp)
#define sector_number		-12(%bp)
#define sectors_per_track	-10(%bp)
#define cylinder_limit		-8(%bp)
#define head_limit		-6(%bp)
#define backup_es		-4(%bp)
#define destination_offset	4(%bp)
#define destination_segment	6(%bp)
#define logical_sector		8(%bp)
#define sector_count		12(%bp)
#define drive_number		14(%bp)

	movw	%es, %cx
	movw	%cx, backup_es

	xorw	%cx, %cx
	movw	%cx, %es
	movw	%cx, %di

	/* Read drive parameters. */
	movb	$0x08, %ah
	movb	drive_number, %dl
	int	$0x13
	jc	read_error

	mov	%cx, %ax

	/* ax:sectors per track */
	and	$0x3f, %ax
	movw	%ax, sectors_per_track

	/* cx: cylinder limit
	rorw	$8, %cx
	shr	$2, %ch
	and	$0x3ff, %cx
	movw	%cx, cylinder_limit(%bp) */

	/* dh,dl: number of hard drives and head limit */
	addb	$1, %dh
	movb	%dh, head_limit

	/* sector: lba % sectors_per_track + 1
	   head: lba / sectors_per_track % heads_per_cylinder
	   cylinder: lba / sectors_per_track / heads_per_cylinder
	*/

	/* Divide the LBA by the number of sectors per track. */
	xorl	%ebx, %ebx
	movb	sectors_per_track, %bl
	movl	logical_sector, %eax
	xorl	%edx, %edx

	divl	%ebx

	/* (dx,ax): (lba / sectors_per_track, lba % sectors_per_track) */

	/* sector number */
	add	$1, %dx
	movw	%dx, sector_number

	/* Divide the track number by the number of heads per cylinder. */
	xorb	%bh, %bh
	xorw	%dx, %dx
	movb	head_limit, %bl
	divw	%bx
	
	// head number
	movw	%dx, head_number
	// cylinder number
	movw	%ax, cylinder_number


	/* ah: 2
	   al: number of sectors
	   ch: cylinder
	   cl: sector
	   dh: head
	   dl: drive
	   es:bx: buffer address
	*/
	/* Construct the cylinder,head value in %cx */
	movw	cylinder_number, %cx
	rorw	$8, %cx
	shlb	$6, %cl
	movb	sector_number, %al
	andb	$0x3f, %al
	or	%al, %cl

	/* drive number */
	movb	drive_number, %dl
	/* head number */
	movb	head_number, %dh
	/* number of sectors to read */
	movb	sector_count, %al
	/* destination address */
	movw	6(%bp), %bx
	movw	%bx, %es
	movw	4(%bp), %bx
	

	/* Do the read. */
	movb	$0x02, %ah
	int	$0x13

	jc	read_error

	jmp	1f
read_error:

1:
	movw	backup_es, %cx
	movw	%cx, %es

	add	$32, %sp
	popa
	pop	%bp
	ret

	/* Partition table. */
	.org 0x1be
	.quad 0,0,0,0

	/*.org 510
	.byte 0x55, 0xaa*/
