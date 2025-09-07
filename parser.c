#include "parser.h"

#include "config.h"
#include "string.h"

struct line
{
    char *ptr;
    size_t len;
};

struct token
{
    char *ptr;
    size_t len;
};

static const char method[] = "GET";

static const char version[] = "HTTP/1.1";

static const char response400[] = "HTTP/1.1 400 Bad Request\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Content-Length: 16\r\n"
                                  "Connection: close\r\n"
                                  "\r\n"
                                  "400 Bad Request\n";

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

static struct line next_line(char *buffer, char *end_ptr, int crlf)
{
    struct line line = { NULL, 0 };
    char *ptr;

    if (buffer < end_ptr) {
        line.ptr = buffer;
        line.len = end_ptr - buffer;
    }

    if (crlf) {
        for (ptr = buffer; ptr < end_ptr - 1; ++ptr) {
            if (memcmp(ptr, "\r\n", 2) == 0) {
                line.len = ptr - buffer;
                break;
            }
        }
    } else {
        for (ptr = buffer; ptr < end_ptr; ++ptr) {
            if (*ptr == '\n') {
                line.len = ptr - buffer;
                break;
            }
        }
    }

    return line;
}

static struct token next_token(char *buffer, char *end_ptr)
{
    struct token token = { NULL, 0 };
    char *ptr;

    for (ptr = buffer; ptr < end_ptr; ++ptr) {
        if (*ptr == ' ' || *ptr == '\r' ||
            *ptr == '\n' || *ptr == '\0')
        {
            if (token.ptr) {
                token.len = ptr - token.ptr;
                break;
            }
        } else if (!token.ptr) {
            token.ptr = ptr;
        }
    }

    if (token.ptr && !token.len) {
        token.len = end_ptr - token.ptr;
    }

    return token;
}

const char *parse_request(char *buffer, size_t cnt, char **responses)
{
    struct line line;
    struct token token;
    char *line_ptr, *token_ptr;
    int crlf = 0;
    int parsed_method = 0;
    int endpoint_idx = -1;
    int parsed_version = 0;
    int bad_request = 0;
    size_t i;

    for (i = 0; i < cnt - 1; ++i) {
        if (memcmp(buffer + i, "\r\n", 2) == 0) {
            crlf = 1;
            break;
        }
    }

    line_ptr = buffer;
    for (;;) {
        line = next_line(line_ptr, buffer + cnt, crlf);
        if (!line.ptr) {
            return NULL;
        }
        if (line.ptr && !line.len) {
            if (bad_request) {
                return response400;
            } else if (parsed_version) {
                return responses[endpoint_idx];
            }
        }
        line_ptr = line.ptr + line.len + (crlf ? 2 : 1);

        token_ptr = line.ptr;
        while (!bad_request) {
            token = next_token(token_ptr, line_ptr);
            if (!token.ptr) {
                break;
            }
            token_ptr = token.ptr + token.len;
            if (!parsed_method) {
                if (token.len == sizeof(method) - 1 &&
                    memcmp(token.ptr, method, token.len) == 0)
                {
                    parsed_method = 1;
                } else {
                    bad_request = 1;
                }
                continue;
            }
            if (endpoint_idx == -1) {
                for (i = 0; i < endpoints_count; ++i) {
                    if (token.len == strlen(endpoints[i]) &&
                        memcmp(token.ptr, endpoints[i], token.len) == 0)
                    {
                        endpoint_idx = i;
                        break;
                    }
                }
                if (endpoint_idx == -1) {
                    bad_request = 1;
                }
                continue;
            }
            if (!parsed_version) {
                if (token.len == sizeof(version) - 1 &&
                    memcmp(token.ptr, version, token.len) == 0)
                {
                    parsed_version = 1;
                } else {
                    bad_request = 1;
                }
                continue;
            }
        }
    }
}
