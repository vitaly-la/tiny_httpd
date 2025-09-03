#include <fcntl.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

enum { port = 8000 };

static const char *headers[] = {"HTTP/1.1 200 OK\r\n",
                                "Content-Type: text/html\r\n",
                                "Connection: close\r\n"};

static const char crlf[] = "\r\n";

static inline uint16_t htons2(uint16_t hostshort)
{
    return ((hostshort & 255) << 8) | ((hostshort & ~255) >> 8);
}

static char *itoa(unsigned val)
{
    static char buf[32] = {0};
    int i = 30;
    do {
        buf[i] = "0123456789"[val % 10];
        --i;
        val /= 10;
    } while (val && i);
    return &buf[i + 1];
}

static void *mempcpy(void *dst, const void *src, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        ((char*)dst)[i] = ((char*)src)[i];
    }
    return (char*)dst + len;
}

void *memset(void *dest, int c, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        ((char*)dest)[i] = c;
    }
    return dest;
}

static size_t strlen(const char *s)
{
    size_t len = 0;
    for (; *s != '\0'; ++s, ++len) {}
    return len;
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
    response_len += sizeof(crlf) - 1;
    response_len += sizeof(crlf) - 1;

    char *response = mmap(NULL, response_len, PROT_READ | PROT_WRITE,
                          MAP_ANON | MAP_PRIVATE, -1, 0);

    char *ptr = response;
    for (size_t i = 0; i < sizeof(headers) / sizeof(*headers); ++i) {
        ptr = mempcpy(ptr, headers[i], strlen(headers[i]));
    }
    ptr = mempcpy(ptr, "Content-Length: ",
                  sizeof("Content-Length: ") - 1);
    ptr = mempcpy(ptr, itoa(len), strlen(itoa(len)));
    ptr = mempcpy(ptr, crlf, sizeof(crlf) - 1);
    ptr = mempcpy(ptr, crlf, sizeof(crlf) - 1);

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
    sa.sin_port = htons2(port);
    socklen_t b = sizeof(sa);

    if (bind(s, (const struct sockaddr*)&sa, b)) {
        char msg[] = "bind failed\n";
        write(2, msg, sizeof(msg) - 1);
        _exit(1);
    }

    listen(s, 1024);

    for (;;) {
        int c = accept(s, (struct sockaddr*)&sa, &b);
        write(c, response, response_len);
        if (close(c) == -1) {
            char msg[] = "descriptor failure\n";
            write(2, msg, sizeof(msg) - 1);
            _exit(1);
        }
    }
}
