SYS := $(shell uname | grep -iq linux && echo sys_linux || echo sys_bsd)

all:
	$(CC) -std=c99 -Oz -Wall -Wextra -static -nostdlib -s \
		main.c $(SYS).s -o tiny_httpd
	objcopy --remove-section .comment      tiny_httpd
	objcopy --remove-section .eh_frame     tiny_httpd
	objcopy --remove-section .eh_frame_hdr tiny_httpd

clean:
	rm -f *.o tiny_httpd
