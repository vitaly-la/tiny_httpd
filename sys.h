#ifndef SYS_H
#  define SYS_H

#  include <errno.h>
#  include <fcntl.h>
#  include <netinet/in.h>
#  include <stddef.h>
#  include <sys/mman.h>
#  include <sys/socket.h>
#  include <sys/stat.h>

#  ifdef LINUX

#    ifndef MAP_ANON
#      define MAP_ANON 0x20
#    endif

struct rusage;

#  else /* FreeBSD */

#    include <sys/event.h>

int sys_kevent(int kq, const struct kevent *changelist, int nchanges,
               struct kevent *eventlist, int nevents,
               const struct timespec *timeout);

int sys_kqueue(void);

#  endif /* LINUX */

int sys_accept(int s, struct sockaddr *addr, socklen_t *addrlen);

int sys_bind(int s, const struct sockaddr *addr, socklen_t addrlen);

int sys_close(int fd);

int sys_fstat(int fd, struct stat *sb);

int sys_listen(int s, int backlog);

void *sys_mmap(void *addr, size_t len, int prot,
               int flags, int fd, off_t offset);

int sys_open(const char *path, int flags, ...);

ssize_t sys_read(int fd, void *buf, size_t nbytes);

int sys_socket(int domain, int type, int protocol);

ssize_t sys_write(int fd, const void *buf, size_t nbytes);

void sys_exit(int status);

#endif /* SYS_H */
