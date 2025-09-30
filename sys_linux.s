	.globl	sys_accept
	.globl	sys_bind
	.globl	sys_close
	.globl	sys_epoll_create1
	.globl	sys_epoll_ctl
	.globl	sys_epoll_wait
	.globl	sys_fstat
	.globl	sys_listen
	.globl	sys_mmap
	.globl	sys_open
	.globl	sys_read
	.globl	sys_socket
	.globl	sys_timerfd_create
	.globl	sys_timerfd_settime
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

sys_epoll_create1:
	mov	$291, %eax
	syscall
	ret

sys_epoll_ctl:
	mov	$233, %eax
	mov	%rcx, %r10
	syscall
	ret

sys_epoll_wait:
	mov	$232, %eax
	mov	%rcx, %r10
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

sys_socket:
	mov	$41, %eax
	syscall
	ret

sys_timerfd_create:
	mov	$283, %eax
	syscall
	ret

sys_timerfd_settime:
	mov	$286, %eax
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
