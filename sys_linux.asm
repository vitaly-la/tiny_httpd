global	accept
global	bind
global	close
global	fstat
global	listen
global	mmap
global	open
global	read
global	socket
global	write
global	_exit

section	.text

accept:
	mov	eax, 43
	syscall
	ret

bind:
	mov	eax, 49
	syscall
	ret

close:
	mov	eax, 3
	syscall
	ret

fstat:
	mov	eax, 5
	syscall
	ret

listen:
	mov	eax, 50
	syscall
	ret

mmap:
	mov	eax, 9
	mov	r10, rcx
	syscall
	ret

open:
	mov	eax, 2
	syscall
	ret

read:
	mov	eax, 0
	syscall
	ret

socket:
	mov	eax, 41
	syscall
	ret

write:
	mov	eax, 1
	syscall
	ret

_exit:
	mov	eax, 60
	syscall
