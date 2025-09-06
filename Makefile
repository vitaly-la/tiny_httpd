SYS := $(shell uname | grep -iq linux && echo sys_linux || echo sys_bsd)

ifeq ($(SYS), sys_linux)
	CFLAGS := -std=gnu99
else
	CFLAGS := -std=c99
endif

CFLAGS += -O2 -Wall -Wextra -static -nostdlib -s

all:
	$(CC) $(CFLAGS) main.c $(SYS).s -o tiny_httpd
	objcopy --remove-section .comment      tiny_httpd
	objcopy --remove-section .eh_frame     tiny_httpd
	objcopy --remove-section .eh_frame_hdr tiny_httpd

clean:
	rm -f *.o tiny_httpd
