#include "parser.h"

#include <stddef.h>

#include "config.h"

size_t strlen(const char *s);

struct line
{
    char *begin;
    char *end;
};

struct token
{
    char *begin;
    char *end;
};

static const char method[] = "GET";

static const char version[] = "HTTP/1.1";

static const char response400[] = "HTTP/1.1 400 Bad Request\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Content-Length: 16\r\n"
                                  "Connection: close\r\n"
                                  "\r\n"
                                  "400 Bad Request\n";

static size_t len(struct token token)
{
    return token.end - token.begin;
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

static struct line next_line(char *buffer, char *buffer_end, int crlf)
{
    struct line line = { 0 };
    char *ptr;

    if (buffer < buffer_end) {
        line.begin = buffer;
        line.end = buffer_end;
    }

    if (crlf) {
        for (ptr = buffer; ptr < buffer_end - 1; ++ptr) {
            if (memcmp(ptr, "\r\n", 2) == 0) {
                line.end = ptr;
                break;
            }
        }
    } else {
        for (ptr = buffer; ptr < buffer_end; ++ptr) {
            if (*ptr == '\n') {
                line.end = ptr;
                break;
            }
        }
    }

    return line;
}

static struct token next_token(char *line, char *line_end)
{
    struct token token = { 0 };
    char *ptr;

    for (ptr = line; ptr < line_end; ++ptr) {
        if (*ptr == ' ' || *ptr == '\r' ||
            *ptr == '\n' || *ptr == '\0')
        {
            if (token.begin) {
                token.end = ptr;
                break;
            }
        } else if (!token.begin) {
            token.begin = ptr;
        }
    }

    if (token.begin && !token.end) {
        token.end = line_end;
    }

    return token;
}

const char *parse_request(char *buffer, char *end, char **responses)
{
    struct line line;
    struct token token;
    char *ptr, *line_ptr, *token_ptr;
    int crlf = 0;
    int parsed_method = 0;
    int endpoint_idx = -1;
    int parsed_version = 0;
    int bad_request = 0;
    size_t i;

    for (ptr = buffer; ptr < end - 1; ++ptr) {
        if (memcmp(ptr, "\r\n", 2) == 0) {
            crlf = 1;
            break;
        }
    }

    line_ptr = buffer;
    for (;;) {
        line = next_line(line_ptr, end, crlf);
        if (!line.begin) {
            return NULL;
        }
        if (line.begin && line.begin == line.end) {
            if (bad_request) {
                return response400;
            } else if (parsed_version) {
                return responses[endpoint_idx];
            }
        }
        line_ptr = line.end + (crlf ? 2 : 1);

        token_ptr = line.begin;
        while (!bad_request) {
            token = next_token(token_ptr, line_ptr);
            if (!token.begin) {
                break;
            }
            token_ptr = token.end;
            if (!parsed_method) {
                if (len(token) == sizeof(method) - 1 &&
                    memcmp(token.begin, method, len(token)) == 0)
                {
                    parsed_method = 1;
                } else {
                    bad_request = 1;
                }
                continue;
            }
            if (endpoint_idx == -1) {
                for (i = 0; i < endpoints_count; ++i) {
                    if (len(token) == strlen(endpoints[i]) &&
                        memcmp(token.begin, endpoints[i], len(token)) == 0)
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
                if (len(token) == sizeof(version) - 1 &&
                    memcmp(token.begin, version, len(token)) == 0)
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
