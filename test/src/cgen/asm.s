	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 10, 15	sdk_version 10, 15
	.section	__TEXT,__literal8,8byte_literals
	.p2align	3               ## -- Begin function f2
LCPI0_0:
	.quad	4608083138725491507     ## double 1.2
LCPI0_1:
	.quad	4607767886751575572     ## double 1.1299999999999999
LCPI0_2:
	.quad	4607722850755301868     ## double 1.1200000000000001
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_f2
	.p2align	4, 0x90
_f2:                                    ## @f2
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movsd	LCPI0_0(%rip), %xmm0    ## xmm0 = mem[0],zero
	movsd	LCPI0_1(%rip), %xmm1    ## xmm1 = mem[0],zero
	movsd	LCPI0_2(%rip), %xmm2    ## xmm2 = mem[0],zero
	movsd	%xmm2, -8(%rbp)
	movsd	%xmm1, -16(%rbp)
	movsd	-8(%rbp), %xmm1         ## xmm1 = mem[0],zero
	addsd	-16(%rbp), %xmm1
	movsd	%xmm1, -8(%rbp)
	movsd	-8(%rbp), %xmm1         ## xmm1 = mem[0],zero
	subsd	-16(%rbp), %xmm1
	movsd	%xmm1, -8(%rbp)
	movsd	-8(%rbp), %xmm1         ## xmm1 = mem[0],zero
	mulsd	-16(%rbp), %xmm1
	movsd	%xmm1, -8(%rbp)
	movsd	-8(%rbp), %xmm1         ## xmm1 = mem[0],zero
	divsd	-16(%rbp), %xmm1
	movsd	%xmm1, -8(%rbp)
	popq	%rbp
	retq
	.cfi_endproc
                                        ## -- End function
	.section	__TEXT,__literal4,4byte_literals
	.p2align	2               ## -- Begin function f3
LCPI1_0:
	.long	1067030938              ## float 1.20000005
LCPI1_1:
	.long	1066443735              ## float 1.13
LCPI1_2:
	.long	1066359849              ## float 1.12
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_f3
	.p2align	4, 0x90
_f3:                                    ## @f3
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movss	LCPI1_0(%rip), %xmm0    ## xmm0 = mem[0],zero,zero,zero
	movss	LCPI1_1(%rip), %xmm1    ## xmm1 = mem[0],zero,zero,zero
	movss	LCPI1_2(%rip), %xmm2    ## xmm2 = mem[0],zero,zero,zero
	movss	%xmm2, -4(%rbp)
	movss	%xmm1, -8(%rbp)
	movss	-4(%rbp), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	addss	-8(%rbp), %xmm1
	movss	%xmm1, -4(%rbp)
	movss	-4(%rbp), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	subss	-8(%rbp), %xmm1
	movss	%xmm1, -4(%rbp)
	movss	-4(%rbp), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	mulss	-8(%rbp), %xmm1
	movss	%xmm1, -4(%rbp)
	movss	-4(%rbp), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	divss	-8(%rbp), %xmm1
	movss	%xmm1, -4(%rbp)
	popq	%rbp
	retq
	.cfi_endproc
                                        ## -- End function
	.section	__TEXT,__literal4,4byte_literals
	.p2align	2               ## -- Begin function f
LCPI2_0:
	.long	1067030938              ## float 1.20000005
LCPI2_1:
	.long	1066443735              ## float 1.13
LCPI2_2:
	.long	1066359849              ## float 1.12
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_f
	.p2align	4, 0x90
_f:                                     ## @f
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movss	LCPI2_0(%rip), %xmm0    ## xmm0 = mem[0],zero,zero,zero
	movss	LCPI2_1(%rip), %xmm1    ## xmm1 = mem[0],zero,zero,zero
	movss	LCPI2_2(%rip), %xmm2    ## xmm2 = mem[0],zero,zero,zero
	movss	%xmm2, -4(%rbp)
	movss	%xmm1, -8(%rbp)
	movss	-4(%rbp), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	addss	-8(%rbp), %xmm1
	movss	%xmm1, -4(%rbp)
	movss	-4(%rbp), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	subss	-8(%rbp), %xmm1
	movss	%xmm1, -4(%rbp)
	movss	-4(%rbp), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	mulss	-8(%rbp), %xmm1
	movss	%xmm1, -4(%rbp)
	movss	-4(%rbp), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	divss	-8(%rbp), %xmm1
	movss	%xmm1, -4(%rbp)
	popq	%rbp
	retq
	.cfi_endproc
                                        ## -- End function
	.globl	_main                   ## -- Begin function main
	.p2align	4, 0x90
_main:                                  ## @main
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	subq	$16, %rsp
	movl	$0, -4(%rbp)
	callq	_f
	xorl	%eax, %eax
	movss	%xmm0, -8(%rbp)         ## 4-byte Spill
	addq	$16, %rsp
	popq	%rbp
	retq
	.cfi_endproc
                                        ## -- End function

.subsections_via_symbols
