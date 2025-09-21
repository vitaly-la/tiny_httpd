#define MAIN_C

#include "config.h"
#include "parser.h"
#include "sys.h"

static const char *headers[] = { "HTTP/1.1 200 OK\r\n",
                                 "Content-Type: text/html\r\n",
                                 "Connection: close\r\n" };

static char buffer[1024];

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

static char *itoa(unsigned val)
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

static void *mempcpy(void *dst, const void *src, size_t len)
{
    size_t i;
    for (i = 0; i < len; ++i) {
        ((char*)dst)[i] = ((char*)src)[i];
    }
    return (char*)dst + len;
}

static char *create_response(const char *path)
{
    struct stat sb;
    size_t len, response_len = 0, i;
    char *response, *ptr;

    int fd = sys_open(path, O_RDONLY);
    if (fd < 0 || fd == ENOENT) {
        char msg[] = "open failed\n";
        sys_write(2, msg, sizeof(msg) - 1);
        sys_exit(1);
    }

    sys_fstat(fd, &sb);
    len = sb.st_size;

    for (i = 0; i < sizeof(headers) / sizeof(*headers); ++i) {
        response_len += strlen(headers[i]);
    }
    response_len += sizeof("Content-Length: ") - 1 +
                    strlen(itoa(len)) +
                    sizeof("\r\n") - 1 +
                    sizeof("\r\n") - 1 +
                    len + 1;

    response = sys_mmap(NULL, response_len, PROT_READ | PROT_WRITE,
                        MAP_ANON | MAP_PRIVATE, -1, 0);

    ptr = response;
    for (i = 0; i < sizeof(headers) / sizeof(*headers); ++i) {
        ptr = mempcpy(ptr, headers[i], strlen(headers[i]));
    }
    ptr = mempcpy(ptr, "Content-Length: ",
                  sizeof("Content-Length: ") - 1);
    ptr = mempcpy(ptr, itoa(len), strlen(itoa(len)));
    ptr = mempcpy(ptr, "\r\n", sizeof("\r\n") - 1);
    ptr = mempcpy(ptr, "\r\n", sizeof("\r\n") - 1);
    ptr += sys_read(fd, ptr, len);
    *ptr = '\0';

    sys_close(fd);
    return response;
}

static void create_responses(const char **responses)
{
    size_t i;
    for (i = 0; i < endpoints_count; ++i) {
        responses[i] = create_response(pages[i]);
    }
}

static void serve(int client_fd, const char **responses)
{
    /* zero-initialization of itimerval causes SIGSEGV
       on linux for some reason */
    volatile struct itimerval timer;
    size_t cnt;
    const char *response = NULL;

    timer.it_value.tv_sec     = 10;
    timer.it_value.tv_usec    =  0;
    timer.it_interval.tv_sec  =  0;
    timer.it_interval.tv_usec =  0;
    sys_setitimer(ITIMER_REAL, (const struct itimerval*)&timer, NULL);

    for (cnt = 0;;) {
        char *ptr = buffer + cnt;
        cnt += sys_read(client_fd, ptr, sizeof(buffer) - cnt);
        response = parse_request(buffer, buffer + cnt, responses);
        if (response) {
            break;
        }
    }

    sys_write(client_fd, response, strlen(response));
    sys_exit(0);
}

void _start(void)
{
    const char *responses[endpoints_count];
    int sock;
    struct sockaddr_in sa;
    socklen_t b;

    create_responses(responses);
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

    sys_listen(sock, 16);

    for (;;) {
        int client_fd = sys_accept(sock, (struct sockaddr*)&sa, &b);
        int pid = sys_fork();
        if (pid == 0) {
            sys_close(sock);
            serve(client_fd, responses);
        }
        sys_close(client_fd);
        do {
            pid = sys_wait4(-1, NULL, WNOHANG, NULL);
        } while (pid > 0 && pid != ECHILD);
    }
}
