	.section	__TEXT,__text,regular,pure_instructions
	.globl	_main
	.align	4, 0x90
_main:                                  ## @main
	.cfi_startproc
## BB#0:                                ## %entry
	pushq	%rbp
Ltmp2:
	.cfi_def_cfa_offset 16
Ltmp3:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp4:
	.cfi_def_cfa_register %rbp
	movl	$12, -4(%rbp)
	movl	$34, -8(%rbp)
	movl	$67, -12(%rbp)
	xorl	%eax, %eax
	popq	%rbp
	retq
	.cfi_endproc


.subsections_via_symbols
