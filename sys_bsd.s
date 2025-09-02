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
	mov	$30, %eax
	syscall
	ret

bind:
	mov	$104, %eax
	syscall
	ret

close:
	mov	$6, %eax
	syscall
	ret

fstat:
	mov	$551, %eax
	syscall
	ret

listen:
	mov	$106, %eax
	syscall
	ret

mmap:
	mov	$477, %eax
	mov	%rcx, %r10
	syscall
	ret

open:
	mov	$5, %eax
	syscall
	ret

read:
	mov	$3, %eax
	syscall
	ret

socket:
	mov	$97, %eax
	syscall
	ret

write:
	mov	$4, %eax
	syscall
	ret

_exit:
	mov	$1, %eax
	syscall
