#ifndef PARSER_H
#define PARSER_H

const char *parse_request(const char *buffer, const char *end,
                          const char **responses);

#endif /* PARSER_H */
