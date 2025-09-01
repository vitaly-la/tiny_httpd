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
	mov	eax, 30
	syscall
	ret

bind:
	mov	eax, 104
	syscall
	ret

close:
	mov	eax, 6
	syscall
	ret

fstat:
	mov	eax, 551
	syscall
	ret

listen:
	mov	eax, 106
	syscall
	ret

mmap:
	mov	eax, 477
	mov	r10, rcx
	syscall
	ret

open:
	mov	eax, 5
	syscall
	ret

read:
	mov	eax, 3
	syscall
	ret

socket:
	mov	eax, 97
	syscall
	ret

write:
	mov	eax, 4
	syscall
	ret

_exit:
	mov	eax, 1
	syscall
