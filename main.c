#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"
#include "parser.h"
#include "string.h"

#ifdef LINUX
#define MAP_ANON 0x20
struct rusage;
pid_t wait4(pid_t wpid, int *status, int options, struct rusage *rusage);
#endif

static const char *headers[] = { "HTTP/1.1 200 OK\r\n",
                                 "Content-Type: text/html\r\n",
                                 "Connection: close\r\n" };

#ifndef htons
static uint16_t htons(uint16_t hostshort)
{
    return ((hostshort & 255) << 8) | ((hostshort & ~255) >> 8);
}
#endif

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

size_t strlen(const char *s)
{
    size_t len = 0;
    for (; *s != '\0'; ++s, ++len) {}
    return len;
}

static char *create_response(const char *path)
{
    struct stat sb;
    size_t len, response_len = 0, i;
    char *response, *ptr;

    int fd = open(path, O_RDONLY);
    if (fd < 0 || fd == ENOENT) {
        char msg[] = "open failed\n";
        write(2, msg, sizeof(msg) - 1);
        _exit(1);
    }

    fstat(fd, &sb);
    len = sb.st_size;

    for (i = 0; i < sizeof(headers) / sizeof(*headers); ++i) {
        response_len += strlen(headers[i]);
    }
    response_len += sizeof("Content-Length: ") - 1 +
                    strlen(itoa(len)) +
                    sizeof("\r\n") - 1 +
                    sizeof("\r\n") - 1 +
                    len + 1;

    response = mmap(NULL, response_len, PROT_READ | PROT_WRITE,
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
    ptr += read(fd, ptr, len);
    *ptr = '\0';

    close(fd);
    return response;
}

static void create_responses(char **responses)
{
    size_t i;
    for (i = 0; i < endpoints_count; ++i) {
        responses[i] = create_response(pages[i]);
    }
}

static void serve(int client_fd, char **responses)
{
    /* zero-initialization of itimerval causes SIGBUS
       on Linux clang -O2 */
    volatile struct itimerval timer;
    size_t cnt;
    char buffer[1024];
    const char *response = NULL;

    timer.it_value.tv_sec     = 10;
    timer.it_value.tv_usec    = 0;
    timer.it_interval.tv_sec  = 0;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, (const struct itimerval*)&timer, NULL);

    for (cnt = 0;;) {
        char *ptr = buffer + cnt;
        cnt += read(client_fd, ptr, sizeof(buffer) - cnt);
        response = parse_request(buffer, cnt, responses);
        if (response) {
            break;
        }
    }

    write(client_fd, response, strlen(response));
    _exit(0);
}

void _start(void)
{
    char *responses[endpoints_count];
    int sock;
    struct sockaddr_in sa = { 0 };
    socklen_t b;

    create_responses(responses);
    sock = socket(PF_INET, SOCK_STREAM, 0);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    b = sizeof(sa);
    if (bind(sock, (const struct sockaddr*)&sa, b)) {
        char msg[] = "bind failed\n";
        write(2, msg, sizeof(msg) - 1);
        _exit(1);
    }

    listen(sock, 16);

    for (;;) {
        int client_fd = accept(sock, (struct sockaddr*)&sa, &b);
        int pid = fork();
        if (pid == 0) {
            close(sock);
            serve(client_fd, responses);
        }
        close(client_fd);
        do {
            pid = wait4(-1, NULL, WNOHANG, NULL);
        } while (pid > 0 && pid != ECHILD);
    }
}
