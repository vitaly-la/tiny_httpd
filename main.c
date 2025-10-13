#define MAIN_C

#include "config.h"
#include "date.h"
#include "event.h"
#include "parser.h"
#include "sys.h"

struct connection
{
    int fd;
    int offset;
#ifdef LINUX
    int timer_fd;
#endif
};

static const char *headers[] = { "HTTP/1.1 200 OK\r\n",
                                 "Content-Type: text/html\r\n",
                                 "Connection: close\r\n" };

char *itoa(unsigned val)
{
    static char buf[32];
    int i;

    buf[31] = '\0';
    i = 30;
    do {
        buf[i] = "0123456789"[val % 10];
        --i;
        val /= 10;
    } while (val && i);
    return &buf[i + 1];
}

char *stpcpy(char *dst, const char *src)
{
    size_t i;
    for (i = 0; src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }
    dst[i] = src[i];
    return dst + i;
}

size_t strlen(const char *s)
{
    size_t len = 0;
    for (; *s != '\0'; ++s, ++len) {}
    return len;
}

static uint16_t my_htons(uint16_t hostshort)
{
    return ((hostshort & 255) << 8) | ((hostshort & ~255) >> 8);
}

static uint32_t my_htonl(uint32_t hostlong)
{
    return ((hostlong & (255 << 24)) >> 24) |
           ((hostlong & (255 << 16)) >>  8) |
           ((hostlong & (255 <<  8)) <<  8) |
           ((hostlong & (255 <<  0)) << 24);
}

static void create_response(const char *path, char **presponse)
{
    struct stat sb;
    size_t len, response_len = 0, i;
    char *ptr;
    const char *date;

    int fd = sys_open(path, O_RDONLY);
    if (fd < 0 || fd == ENOENT) {
        char msg[] = "open failed\n";
        sys_write(2, msg, sizeof(msg) - 1);
        sys_exit(1);
    }

    sys_fstat(fd, &sb);
    len = sb.st_size;

    date = get_rfc_now();

    for (i = 0; i < sizeof(headers) / sizeof(*headers); ++i) {
        response_len += strlen(headers[i]);
    }
    response_len += sizeof("Content-Length: ") - 1 +
                    strlen(itoa(len)) +
                    sizeof("\r\n") - 1 +
                    sizeof("Date: ") - 1 +
                    strlen(date) +
                    sizeof("\r\n") - 1 +
                    sizeof("\r\n") - 1 +
                    len + 1;

    if (!*presponse) {
        *presponse = sys_mmap(NULL, response_len, PROT_READ | PROT_WRITE,
                              MAP_ANON | MAP_PRIVATE, -1, 0);
    }

    ptr = *presponse;
    for (i = 0; i < sizeof(headers) / sizeof(*headers); ++i) {
        ptr = stpcpy(ptr, headers[i]);
    }
    ptr = stpcpy(ptr, "Content-Length: ");
    ptr = stpcpy(ptr, itoa(len));
    ptr = stpcpy(ptr, "\r\n");
    ptr = stpcpy(ptr, "Date: ");
    ptr = stpcpy(ptr, date);
    ptr = stpcpy(ptr, "\r\n");
    ptr = stpcpy(ptr, "\r\n");
    ptr += sys_read(fd, ptr, len);
    *ptr = '\0';

    sys_close(fd);
}

static void create_responses(char **responses)
{
    size_t i;
    for (i = 0; i < endpoints_count; ++i) {
        create_response(pages[i], responses + i);
    }
}

void _start(void)
{
    static char *responses[endpoints_count] = {0};
    static struct connection connections[max_connections];
    static char buffers[max_connections * buffer_size];
    int sock, kq;
    struct sockaddr_in sa;
    socklen_t b;

    sock = sys_socket(PF_INET, SOCK_STREAM, 0);

    sa.sin_family = AF_INET;
    sa.sin_port = my_htons(port);
    sa.sin_addr.s_addr = my_htonl(INADDR_ANY);
    b = sizeof(sa);
    if (sys_bind(sock, (const struct sockaddr*)&sa, b)) {
        char msg[] = "bind failed\n";
        sys_write(2, msg, sizeof(msg) - 1);
        sys_exit(1);
    }

    kq = init_queue();
    queue_descriptor(kq, sock);

    sys_listen(sock, 16);

    for (;;) {
        event_t event;

        wait_queue(kq, &event);
        if (get_descriptor(&event) == sock) {
            int fd = sys_accept(sock, (struct sockaddr*)&sa, &b);
            int i;

            for (i = 0; i < max_connections; ++i) {
                if (connections[i].fd == 0) {
                    connections[i].fd = fd;
                    connections[i].offset = 0;
                    queue_descriptor(kq, fd);
#ifdef LINUX
                    connections[i].timer_fd = queue_timer(kq, fd, timeout);
#else /* FreeBSD */
                    queue_timer(kq, fd, timeout);
#endif
                    break;
                }
            }

            if (i == max_connections) {
                sys_close(fd);
            }

        } else if (get_event_type(&event) == read_event) {
            int fd = get_descriptor(&event);
            int i, offset;
            char *buffer, *ptr;
            const char *response = NULL;

            for (i = 0; connections[i].fd != fd; ++i) {}

            buffer = buffers + i * buffer_size;
            offset = connections[i].offset;
            ptr = buffer + offset;

            offset += sys_read(fd, ptr, buffer_size - offset);
            connections[i].offset = offset;

            create_responses(responses);
            response = parse_request(buffer, buffer + offset,
                                     (const char**)responses);

            if (response) {
                sys_write(fd, response, strlen(response));
                sys_close(fd);
                connections[i].fd = 0;
#ifdef LINUX
                sys_close(connections[i].timer_fd);
#else /* FreeBSD */
                delete_timer(kq, fd, timeout);
#endif
            }

        } else {
            int fd = get_descriptor(&event);
            int i;

            for (i = 0; connections[i].fd != fd; ++i) {}

            sys_close(fd);
            connections[i].fd = 0;
#ifdef LINUX
            sys_close(connections[i].timer_fd);
#endif
        }
    }
}
