#include "event.h"

#ifdef LINUX

int init_queue(void)
{
    return sys_epoll_create1(0);
}

void queue_descriptor(int kq, int fd)
{
    event_t event;

    event.events = EPOLLIN;
    event.data.fd = fd;
    sys_epoll_ctl(kq, EPOLL_CTL_ADD, fd, &event);
}

void wait_queue(int kq, event_t *pevent)
{
    sys_epoll_wait(kq, pevent, 1, -1);
}

int get_descriptor(event_t *pevent)
{
    return pevent->data.fd;
}

void queue_timer(int kq, int fd, int64_t timeout);
    (void)kq;
    (void)fd;
    (void)timeout;
}

void delete_timer(int kq, int fd, int64_t timeout)
    (void)kq;
    (void)fd;
    (void)timeout;
}

int is_read_event(event_t *pevent)
{
    (void)pevent;
    return 1;
}

#else /* FreeBSD */

int init_queue(void)
{
    return sys_kqueue();
}

void queue_descriptor(int kq, int fd)
{
    event_t event;

    EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
    sys_kevent(kq, &event, 1, NULL, 0, NULL);
}

void wait_queue(int kq, event_t *pevent)
{
    sys_kevent(kq, NULL, 0, pevent, 1, NULL);
}

int get_descriptor(event_t *pevent)
{
    return pevent->ident;
}

void queue_timer(int kq, int fd, int64_t timeout)
{
    event_t event;

    EV_SET(&event, fd, EVFILT_TIMER, EV_ADD | EV_ONESHOT,
           NOTE_SECONDS, timeout, NULL);
    sys_kevent(kq, &event, 1, NULL, 0, NULL);
}

void delete_timer(int kq, int fd, int64_t timeout)
{
    event_t event;

    EV_SET(&event, fd, EVFILT_TIMER, EV_DELETE,
           NOTE_SECONDS, timeout, NULL);
    sys_kevent(kq, &event, 1, NULL, 0, NULL);
}

int is_read_event(event_t *pevent)
{
    return pevent->filter == EVFILT_READ;
}

#endif
