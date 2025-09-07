#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

const char *parse_request(char *buffer, size_t cnt, char **responses);

#endif
