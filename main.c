#include <fcntl.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

static const char header[] =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 1256\r\n"
    "Connection: close\r\n"
    "\r\n";

static inline uint16_t htons2(uint16_t hostshort)
{
    return ((hostshort & 255) << 8) | ((hostshort & ~255) >> 8);
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
    size_t response_len = sizeof(header) - 1 + len;

    char *response = mmap(NULL, response_len, PROT_READ | PROT_WRITE,
                          MAP_ANON | MAP_PRIVATE, -1, 0);
    for (size_t i = 0; i < sizeof(header) - 1; ++i) {
        response[i] = header[i];
    }

    read(fd, response + sizeof(header) - 1, len);

    close(fd);

    *presponse = response;
    return response_len;
}

void _start(void)
{
    char *response;
    size_t response_len = create_response(&response);

    int s = socket(PF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in sa;
    for (size_t i = 0; i < sizeof(sa); ++i) {
        ((char*)&sa)[i] = 0;
    }
    sa.sin_family = AF_INET;
    sa.sin_port = htons2(8000);
    socklen_t b = sizeof(sa);

    if (bind(s, (const struct sockaddr*)&sa, b)) {
        char msg[] = "bind failed\n";
        write(2, msg, sizeof(msg) - 1);
        _exit(1);
    }

    listen(s, 1024);

    for (;;) {
        int c = accept(s, (struct sockaddr*)&sa, &b);
        if (c == -1) {
            char msg[] = "accept failed\n";
            write(2, msg, sizeof(msg) - 1);
            _exit(1);
        }

        if (write(c, response, response_len) == -1) {
            char msg[] = "write failed\n";
            write(2, msg, sizeof(msg) - 1);
            _exit(1);
        }

        close(c);
    }
}
