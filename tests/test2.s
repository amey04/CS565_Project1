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
	movl	$5, -8(%rbp)
	imull	$3, -8(%rbp), %eax
	movl	%eax, -8(%rbp)
	shll	-8(%rbp)
	imull	$5, -8(%rbp), %eax
	movl	%eax, -8(%rbp)
	leaq	L_.str2(%rip), %rdi
	xorl	%eax, %eax
	callq	_printf
	movl	-8(%rbp), %esi
	leaq	L_.str3(%rip), %rdi
	xorl	%eax, %eax
	callq	_printf
	movl	-4(%rbp), %eax
	addq	$16, %rsp
	popq	%rbp
	retq
	.cfi_endproc

	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"This code is unreachable\n"

L_.str1:                                ## @.str1
	.asciz	"This code is reachable from label2, but not from entry.\n"

L_.str2:                                ## @.str2
	.asciz	"Expected result: 5*3*2*5=150\n"

L_.str3:                                ## @.str3
	.asciz	"%d"


.subsections_via_symbols
