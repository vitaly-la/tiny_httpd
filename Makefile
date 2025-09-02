SYS := $(shell uname | grep -iq linux && echo sys_linux || echo sys_bsd)

all:
	$(CC) -std=c99 -O0 -Wall -Wextra -static -nostdlib -s \
		main.c $(SYS).s -o tiny_httpd

clean:
	rm -f *.o tiny_httpd
