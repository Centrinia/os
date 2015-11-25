/* ext2_pbr.S
   The partition boot record for ext2 filesystems. */



	.code16
	.text
	.org	0
_start:
	movw	$0x7000, %cx
	movw	%cx, %ss

	movw	%cx, %es
	movw	%cx, %fs
	movw	$0xfff8, %sp
	xorw	%di, %di

	drive_number = 0
	heads_per_cylinder = 4
	sectors_per_track = 8
	block_size = 12
	block_sector = 16
	iblock_index = 20
	block = 24

	movb	%dl, %fs:drive_number

	movw	$0x7c0, %cx
	movw	%cx, %ds
	
	/* Get drive parameters. */

	/* The drive number is already in %dl */
	/* movw	%fs:DRIVE_NUMBER, %dx */
	movb	$0x08, %ah
	int	$0x13
	jc	handle_error

	xorl	%eax, %eax
	movb	%cl, %al
	andb	$0x3f, %al
	movl	%eax, %fs:sectors_per_track

	movb	%dh, %al
	incw	%ax
	movl	%eax, %fs:heads_per_cylinder


	/* Read the superblock. */
	movw	$block, %bx
	movl	$2, %eax
	call	read_lba

	/* Get the log of the block size. */
	movl	%es:24(%bx), %ecx
	addl	$1, %ecx
	movl	%ecx, %fs:block_sector
	addl	$9, %ecx
	movl	%ecx, %fs:block_size

	/* Get the index of the first group descriptor. */
	movl	%es:20(%bx), %eax
	incl	%eax
	movb	%fs:block_sector,%cl
	shll	%cl, %eax
	/* Read the group descriptor. */
	call	read_lba

	/* Get the inode table. */
	movl	%es:8(%bx), %eax
	movb	%fs:block_sector,%cl
	shll	%cl, %eax
	
	/* Get the second inode. */
	call	read_lba

	xorw	%ax, %ax
	movw	%ax, iblock_index

	movw	$0x6c00, %cx
	movw	%cx, %es

search_directory:
	movw	iblock_index, %di
	/* Get the i_block array. */
	movl	%fs:(block+40+128)(%di), %eax
	movb	%fs:block_sector,%cl
	shll	%cl, %eax

	/* Start loading directory data. */
	xorw	%bx, %bx
	call	read_lba

	
1:
	/* Bail if the record length is zero. Put the record length in %bp */
	movw	%es:4(%bx), %bp
	test	%bp, %bp
	jz	handle_error

	/* Compare the file name with the kernel name. */
	xorl	%ecx, %ecx
	movb	%es:6(%bx), %cl

	/* Do the string comparison. */
	movw	$(requested_file-_start), %si
	movw	$8, %di
	addw	%bx, %di

	repe cmpsb

	//movw	$10, %eax
	/* If the zero flag is not set then there is a mismatch. */
	jnz	2f

	//movw	$20, %eax
	/* If the counter is not zero then there are remaining bytes in the directory entry. */
	testl	%ecx, %ecx
	jnz	2f

	//movw	$30, %eax
	/* If the remaining byte in the requested file string is not the null terminator then there are remaining bytes in that. */
	movb	%ds:(%si), %al
	testb	%al, %al
	jnz	2f

	jmp	found_entry


2:
	/* Go to the next record. */
	addw	%bp, %bx
	cmpw	%bx, %fs:block_size
	jb	1b

	movw	iblock_index, %cx
	addw	$4, %cx
	movw	%cx, iblock_index
	cmpw	$12, %cx
	jbe	search_directory



handle_error:
	hlt
	jmp	handle_error


found_entry:
	/* %es:(%bx) is the directory entry of the kernel file. */
	/* Get the inode index. */
	movb	%es:(%bx), %eax
	/* TODO: Load the kernel. */
	nop
	jmp	found_entry


	/* Read a sector.
	 * %es:(%bx) : The destination buffer.
	 * %eax    : The LBA
         */
read_lba:
	xorl	%edx, %edx
	divl	%fs:sectors_per_track

	/* The sector number is in %cl. */
	movb	%dl, %cl
	incb	%cl

	xorl	%edx, %edx
	divl	%fs:heads_per_cylinder

	movb	%dl, %dh
	movb	%fs:drive_number, %dl

	/* The cylinder number is in %ax. */
	rolb	$6, %ah
	orb	%ah, %cl
	movb	%al, %ch
	
	movw	$0x0201, %ax
1:
	int	$0x13
	jc	handle_error
	//jc	1b

	ret

requested_file:
	//.asciz	"kernel.bin"
	.asciz	"foo_0.txt"

	.org	510
	.byte	0x55,0xaa

