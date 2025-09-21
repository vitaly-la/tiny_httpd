	.globl	sys_accept
	.globl	sys_bind
	.globl	sys_close
	.globl	sys_fork
	.globl	sys_fstat
	.globl	sys_listen
	.globl	sys_mmap
	.globl	sys_open
	.globl	sys_read
	.globl	sys_setitimer
	.globl	sys_socket
	.globl	sys_wait4
	.globl	sys_write
	.globl	sys_exit

	.text
sys_accept:
	mov	$43, %eax
	syscall
	ret

sys_bind:
	mov	$49, %eax
	syscall
	ret

sys_close:
	mov	$3, %eax
	syscall
	ret

sys_fork:
	mov	$57, %eax
	syscall
	ret

sys_fstat:
	mov	$5, %eax
	syscall
	ret

sys_listen:
	mov	$50, %eax
	syscall
	ret

sys_mmap:
	mov	$9, %eax
	mov	%rcx, %r10
	syscall
	ret

sys_open:
	mov	$2, %eax
	syscall
	ret

sys_read:
	mov	$0, %eax
	syscall
	ret

sys_setitimer:
	mov	$38, %eax
	syscall
	ret

sys_socket:
	mov	$41, %eax
	syscall
	ret

sys_wait4:
	mov	$61, %eax
	mov	%rcx, %r10
	syscall
	ret

sys_write:
	mov	$1, %eax
	syscall
	ret

sys_exit:
	mov	$60, %eax
	syscall
