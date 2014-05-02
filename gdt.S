/* gdt.S */

#define BOOT_ADDRESS		0x7c00
#define STACK_ADDRESS		0x7a00

	.text
	.code16
	.globl kernel

	/*.extern _start
	. = _start + 512*/
kernel:
	jmp mode_switch
	nop


	.align 64
	.globl gdt
gdt:
.long 0x00000000, 0x00000000
.long 0x0000ffff, 0x00cf9a00
.long 0x0000ffff, 0x00cf9200
.long 0x10000067, 0x00408900
gdt_end:



	.globl idt
	.align 64
	.short 0
idt:
	.skip 256*8
idt_end:


	.align 16
	.short 0
gdtr:
	.short 0
gdtr_base:
	.long 0


	.align 16
	.short 0
idtr:
	.short 1024
idtr_base:
	.long 0


	.align 16
default_interrupt:
	pusha
	popa
	iret

	.align 16
mode_switch:
	cli
// Enable the A20 line
	in	$0x92, %al
	test	$2, %al
	jz	1f
	or	$2, %al
	and	$0xfe, %al
	out	%al, $0x92



// Copy over the real mode IDT
	movl	$idt, %edi

	xorl	%ecx, %ecx
	movw	%cx, %es
1:
	// Get the IVT entry
	//movl	%es:(%ecx), %ebx
	/*movl	%ebx, %eax
	shrl	$12, %eax
	andl	$0xffff0, %eax
	xorl	%edx, %edx
	movw	%bx, %dx
	addl	%edx, %eax
	movl	%eax, %ebx*/
	movl	$default_interrupt, %ebx
	
	// Copy over the entire offset.
	movl	%ebx, (%edi)
	movl	%ebx, 4(%edi)
	// Set the code segment field.
	movw	$0x08, 2(%edi)
	movb	$0x0, 4(%edi)
	// Set the type to be a 286 interrupt gate.
	//movb	$0x06, 5(%edi)
	movb	$0x8e, 5(%edi)

	addl	$8, %edi
	addl	$1, %ecx
	cmpl	$0x100, %ecx
	jb	1b

// Load the IDT
	xorl	%eax, %eax
	movw	%ds, %ax
	shll	$4, %eax
	addl	$idt, %eax
	movl	%eax, idtr_base
	movl	$idt_end, %eax
	sub	$idt, %eax
	movw	%ax, idtr
	lidt	idtr



// Load the GDT
	xorl	%eax, %eax
	movw	%ds, %ax
	shll	$4, %eax
	addl	$gdt, %eax
	movl	%eax, gdtr_base
	movl	$gdt_end, %eax
	sub	$gdt, %eax
	movw	%ax, gdtr
	lgdt	gdtr

// Enable protected mode.
	movl	%cr0, %eax
	orl	$1, %eax
	movl	%eax, %cr0


	ljmp	$0x08,$protected_mode
	nop

	.end
