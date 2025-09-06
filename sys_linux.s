	.globl	accept
	.globl	bind
	.globl	close
	.globl	fork
	.globl	fstat
	.globl	listen
	.globl	mmap
	.globl	open
	.globl	read
	.globl	setitimer
	.globl	socket
	.globl	wait4
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

fork:
	mov	$57, %eax
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

setitimer:
	mov	$38, %eax
	syscall
	ret

socket:
	mov	$41, %eax
	syscall
	ret

wait4:
	mov	$61, %eax
	mov	%rcx, %r10
	syscall
	ret

write:
	mov	$1, %eax
	syscall
	ret

_exit:
	mov	$60, %eax
	syscall
