#ifndef EVENT_H
#  define EVENT_H

#  include "sys.h"

enum event_type
{
    read_event,
    timer_event
};

int init_queue(void);

void queue_descriptor(int kq, int fd);

void wait_queue(int kq, event_t *pevent);

int get_descriptor(event_t *pevent);

#  ifdef LINUX
int  queue_timer(int kq, int fd, int64_t timeout);
#  else /* FreeBSD */
void queue_timer(int kq, int fd, int64_t timeout);
#  endif

void delete_timer(int kq, int fd, int64_t timeout);

enum event_type get_event_type(event_t *pevent);

#endif /* EVENT_H */
