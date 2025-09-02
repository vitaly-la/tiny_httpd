	.globl	accept
	.globl	bind
	.globl	close
	.globl	fstat
	.globl	listen
	.globl	mmap
	.globl	open
	.globl	read
	.globl	socket
	.globl	write
	.globl	_exit

	.text
accept:
	mov	$43, %eax
	syscall
	ret

bind:
	mov	$49, %eax
	syscall
	ret

close:
	mov	$3, %eax
	syscall
	ret

fstat:
	mov	$5, %eax
	syscall
	ret

listen:
	mov	$50, %eax
	syscall
	ret

mmap:
	mov	$9, %eax
	mov	%rcx, %r10
	syscall
	ret

open:
	mov	$2, %eax
	syscall
	ret

read:
	mov	$0, %eax
	syscall
	ret

socket:
	mov	$41, %eax
	syscall
	ret

write:
	mov	$1, %eax
	syscall
	ret

_exit:
	mov	$60, %eax
	syscall
