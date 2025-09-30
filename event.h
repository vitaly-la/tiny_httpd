#ifndef EVENT_H
#define EVENT_H

#include "sys.h"

int init_queue(void);

void queue_descriptor(int kq, int fd);

void wait_queue(int kq, event_t *pevent);

int get_descriptor(event_t *pevent);

void queue_timer(int kq, int fd, int64_t timeout);

void delete_timer(int kq, int fd, int64_t timeout);

int is_read_event(event_t *pevent);

#endif /* EVENT_H */
