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
	subq	$16, %rsp
	movl	$0, -4(%rbp)
	movl	$0, -12(%rbp)
	movl	$0, -8(%rbp)
	jmp	LBB0_1
	.align	4, 0x90
LBB0_2:                                 ## %for.body
                                        ##   in Loop: Header=BB0_1 Depth=1
	movl	-8(%rbp), %eax
	imull	%eax, %eax
	addl	%eax, -12(%rbp)
	incl	-8(%rbp)
LBB0_1:                                 ## %for.cond
                                        ## =>This Inner Loop Header: Depth=1
	cmpl	$9, -8(%rbp)
	jle	LBB0_2
## BB#3:                                ## %for.end
	movl	-12(%rbp), %esi
	leaq	L_.str(%rip), %rdi
	xorl	%eax, %eax
	callq	_printf
	movl	-4(%rbp), %eax
	addq	$16, %rsp
	popq	%rbp
	retq
	.cfi_endproc

	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"sum: %d\n"


.subsections_via_symbols
