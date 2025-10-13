#include "event.h"

#ifdef LINUX

#define EPOLLTIMER (1 << 31)

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
    if (pevent->data.fd & EPOLLTIMER) {
        return pevent->data.fd ^ EPOLLTIMER;
    } else {
        return pevent->data.fd;
    }
}

int queue_timer(int kq, int fd, int64_t timeout)
{
    int timer_fd = sys_timerfd_create(CLOCK_REALTIME, 0);
    struct itimerspec time;
    event_t event;

    time.it_value.tv_sec = timeout;
    time.it_value.tv_nsec = 0;
    time.it_interval.tv_sec = 0;
    time.it_interval.tv_nsec = 0;
    sys_timerfd_settime(timer_fd, 0, &time, NULL);

    event.events = EPOLLIN | EPOLLONESHOT;
    event.data.fd = fd | EPOLLTIMER;
    sys_epoll_ctl(kq, EPOLL_CTL_ADD, timer_fd, &event);

    return timer_fd;
}

enum event_type get_event_type(event_t *pevent)
{
    if (pevent->data.fd & EPOLLTIMER) {
        return timer_event;
    }
    return read_event;
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

enum event_type get_event_type(event_t *pevent)
{
    if (pevent->filter == EVFILT_READ) {
        return read_event;
    }
    return timer_event;
}

#endif
