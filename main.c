#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef LINUX
#define MAP_ANON 0x20
struct rusage;
pid_t wait4(pid_t wpid, int *status, int options, struct rusage *rusage);
#endif

enum { port = 8000 };

static const char *headers[] = {"HTTP/1.1 200 OK\r\n",
                                "Content-Type: text/html\r\n",
                                "Connection: close\r\n"};

static const char bad_request[] = "HTTP/1.1 400 Bad Request\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Content-Length: 16\r\n"
                                  "Connection: close\r\n"
                                  "\r\n"
                                  "400 Bad Request\n";

static const char get_request[] = "GET / HTTP/1.1";

static uint16_t my_htons(uint16_t hostshort)
{
    return ((hostshort & 255) << 8) | ((hostshort & ~255) >> 8);
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

static int memcmp(const void *b1, const void *b2, size_t len)
{
    size_t i;
    for (i = 0; i < len; ++i) {
        if (((char*)b1)[i] != ((char*)b2)[i]) {
            return ((char*)b2)[i] - ((char*)b1)[i];
        }
    }
    return 0;
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

static int parse_request(char *buffer, long cnt, int *status)
{
    long i;
    for (i = 0; i < cnt - 3; ++i) {
        if (memcmp(buffer + i, "\r\n\r\n", 4) == 0) {
            if (cnt < (long)sizeof(get_request) - 1 ||
                memcmp(buffer, get_request,
                       sizeof(get_request) - 1) != 0)
            {
                *status = 1;
            }
            return 0;
        }
    }
    for (i = 0; i < cnt - 1; ++i) {
        if (memcmp(buffer + i, "\n\n", 2) == 0) {
            if (cnt < (long)sizeof(get_request) - 1 ||
                memcmp(buffer, get_request,
                       sizeof(get_request) - 1) != 0)
            {
                *status = 1;
            }
            return 0;
        }
    }
    return 1;
}

static size_t create_response(char **presponse)
{
    struct stat sb;
    size_t len, response_len, i;
    char *response, *ptr;

    int fd = open("index.html", O_RDONLY);
    if (fd < 0 || fd == ENOENT) {
        char msg[] = "open failed\n";
        write(2, msg, sizeof(msg) - 1);
        _exit(1);
    }

    fstat(fd, &sb);
    len = sb.st_size;
    response_len = len;

    for (i = 0; i < sizeof(headers) / sizeof(*headers); ++i) {
        response_len += strlen(headers[i]);
    }
    response_len += sizeof("Content-Length: ") - 1;
    response_len += strlen(itoa(len));
    response_len += sizeof("\r\n") - 1;
    response_len += sizeof("\r\n") - 1;

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

    read(fd, ptr, len);

    close(fd);

    *presponse = response;
    return response_len;
}

static void serve(int client_fd, char *response, size_t response_len)
{
    /* zero-initialization of itimerval causes SIGBUS
       on Linux clang -O2 */
    volatile struct itimerval timer;
    int status;
    long cnt;
    char buffer[1024];

    timer.it_value.tv_sec     = 10;
    timer.it_value.tv_usec    = 0;
    timer.it_interval.tv_sec  = 10;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, (const struct itimerval*)&timer, NULL);

    status = 0;
    for (cnt = 0;;) {
        char *ptr = buffer + cnt;
        cnt += read(client_fd, ptr, sizeof(buffer) - cnt);
        if (parse_request(buffer, cnt, &status) == 0) {
            break;
        }
    }

    if (status == 0) {
        write(client_fd, response, response_len);
    } else {
        write(client_fd, bad_request, sizeof(bad_request) - 1);
    }

    _exit(0);
}

void _start(void)
{
    char *response;
    size_t response_len = create_response(&response);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in sa = {0};
    socklen_t b;

    sa.sin_family = AF_INET;
    sa.sin_port = my_htons(port);
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
            serve(client_fd, response, response_len);
        }
        close(client_fd);
        do {
            pid = wait4(-1, NULL, WNOHANG, NULL);
        } while (pid > 0 && pid != ECHILD);
    }
}
