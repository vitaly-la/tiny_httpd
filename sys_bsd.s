	.globl	sys_accept
	.globl	sys_bind
	.globl	sys_close
	.globl	sys_fstat
	.globl	sys_kevent
	.globl	sys_kqueue
	.globl	sys_listen
	.globl	sys_mmap
	.globl	sys_open
	.globl	sys_read
	.globl	sys_socket
	.globl	sys_write
	.globl	sys_exit

	.text
sys_accept:
	mov	$30, %eax
	syscall
	ret

sys_bind:
	mov	$104, %eax
	syscall
	ret

sys_close:
	mov	$6, %eax
	syscall
	ret

sys_fstat:
	mov	$551, %eax
	syscall
	ret

sys_kevent:
	mov	$560, %eax
	mov	%rcx, %r10
	syscall
	ret

sys_kqueue:
	mov	$362, %eax
	syscall
	ret

sys_listen:
	mov	$106, %eax
	syscall
	ret

sys_mmap:
	mov	$477, %eax
	mov	%rcx, %r10
	syscall
	ret

sys_open:
	mov	$5, %eax
	syscall
	ret

sys_read:
	mov	$3, %eax
	syscall
	ret

sys_socket:
	mov	$97, %eax
	syscall
	ret

sys_write:
	mov	$4, %eax
	syscall
	ret

sys_exit:
	mov	$1, %eax
	syscall
