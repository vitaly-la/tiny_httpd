#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t wait4(pid_t wpid, int *status, int options, struct rusage *rusage);

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

static inline uint16_t my_htons(uint16_t hostshort)
{
    return ((hostshort & 255) << 8) | ((hostshort & ~255) >> 8);
}

static char *itoa(unsigned val)
{
    static char buf[32];
    buf[31] = '\0';
    int i = 30;
    do {
        buf[i] = "0123456789"[val % 10];
        --i;
        val /= 10;
    } while (val && i);
    return &buf[i + 1];
}

static int memcmp(const void *b1, const void *b2, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        if (((char*)b1)[i] != ((char*)b2)[i]) {
            return ((char*)b2)[i] - ((char*)b1)[i];
        }
    }
    return 0;
}

static void *mempcpy(void *dst, const void *src, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
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
    for (long i = 0; i < cnt - 3; ++i) {
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
    for (long i = 0; i < cnt - 1; ++i) {
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
    int fd = open("index.html", O_RDONLY);
    if (fd == -1) {
        char msg[] = "open failed\n";
        write(2, msg, sizeof(msg) - 1);
        _exit(1);
    }

    struct stat sb;
    fstat(fd, &sb);
    size_t len = sb.st_size;
    size_t response_len = len;

    for (size_t i = 0; i < sizeof(headers) / sizeof(*headers); ++i) {
        response_len += strlen(headers[i]);
    }
    response_len += sizeof("Content-Length: ") - 1;
    response_len += strlen(itoa(len));
    response_len += sizeof("\r\n") - 1;
    response_len += sizeof("\r\n") - 1;

    char *response = mmap(NULL, response_len, PROT_READ | PROT_WRITE,
                          MAP_ANON | MAP_PRIVATE, -1, 0);

    char *ptr = response;
    for (size_t i = 0; i < sizeof(headers) / sizeof(*headers); ++i) {
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

void _start(void)
{
    char *response;
    size_t response_len = create_response(&response);

    int s = socket(PF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_port = my_htons(port);
    socklen_t b = sizeof(sa);

    if (bind(s, (const struct sockaddr*)&sa, b)) {
        char msg[] = "bind failed\n";
        write(2, msg, sizeof(msg) - 1);
        _exit(1);
    }

    listen(s, 16);

    for (;;) {
        int c = accept(s, (struct sockaddr*)&sa, &b);
        int pid = fork();
        if (pid == 0) {
            close(s);

            char buffer[1024];
            int status = 0;
            for (long cnt = 0;;) {
                char *ptr = buffer + cnt;
                cnt += read(c, ptr, sizeof(buffer) - cnt);
                if (parse_request(buffer, cnt, &status) == 0) {
                    break;
                }
            }

            if (status == 0) {
                write(c, response, response_len);
            } else {
                write(c, bad_request, sizeof(bad_request) - 1);
            }

            close(c);
            _exit(0);
        }
        close(c);
        do {
            pid = wait4(-1, NULL, WNOHANG, NULL);
        } while (pid > 0 && pid != ECHILD);
    }
}
