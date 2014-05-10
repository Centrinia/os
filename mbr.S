/* mbr.S */



	.text
	.code16

	.globl _start


	.org 0
_start:
	/* Set up the stack. It will start at 0x7fffc (07000:fffc); that is right below the extended BIOS data area. */
	movw	$0x7000, %cx
	movw	%cx, %ss
	movw	$0xfffc, %sp

	/* Relocate the boot sector to 0060:0000. */
	/* Set up the source address. */
	movw	$0x7c0, %cx
	movw	%cx, %ds
	xorw	%si, %si

	/* Set up the destination address. */
	movw	$0x60, %cx
	movw	%cx, %es
	xorw	%di, %di

	/* Do the move. */
	movw	$256, %cx
	rep	movsw


	/* Jump. */
	ljmp	$0x60,$1f


	.align 4
1:
	/* Set up the source and destination segments for the move. */
	movw	$0x60, %cx
	movw	%cx, %ds


	/* Find and boot the first active partition. */
	movw	$partition_table, %si
	/* There are four partition table entries to search. */
	movw	$4, %cx
1:
	/* Test bit 7 to determine if the partition is active. */
	testb	$0x80, (%si)
	jnz	load_pbr

	/* Advance to the next partition table entry. */
	addl	$16, %si
	loop	1b


	/* Halt the system because we couldn't find an active partition. */
halt:
	hlt
	jmp	1b


load_pbr:
	/* The destination is 07c0:0000 */
	movw	$0x7c0, %cx
	movw	%cx, %es
	xorw	%bx, %bx
	/* Call ah=02 and read two sectors. */
	movw	$0x0202, %ax
	/* Get the cylinder and sector. */
	movw	2(%si), %cx
	/* Get the head. */
	movb	1(%si), %dh
	/* Do the read. */
	int	$0x13

	/* Do the jump. */
	ljmp	$0x7c0,$0


	/* disk timestamp */
	.org 0x0dc
	/* zero */
	.word 0
	/* original physical drive */
	.byte 0x80
	/* seconds, minutes, hours (starting at zero) */
	.byte 0,0,0

	.org 0x1b8
	.long 0xdeadbeef
	.word 0

	/* partition table */
	.org 0x1be
partition_table:
	.quad 0
	.quad 0
	.quad 0
	.quad 0

	/* boot sector signature */
	.org 0x1fe
	.byte 0x55,0xaa

