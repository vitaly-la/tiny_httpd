#ifndef CONFIG_H
#  define CONFIG_H

enum
{
    port = 8000,
    endpoints_count = 1,
    max_connections = 100,
    buffer_size = 1024,
    timeout = 10
};

#  ifdef MAIN_C

const char *endpoints[endpoints_count] = {
    "/",
};
const char *pages[endpoints_count] = {
    "index.html",
};

#  else
extern const char *endpoints[endpoints_count];
extern const char *pages[endpoints_count];
#  endif
#endif
