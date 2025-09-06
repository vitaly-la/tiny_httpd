SYS := $(shell uname | grep -iq linux && echo sys_linux || echo sys_bsd)

CFLAGS := -std=c89 -O2 -Wall -Wextra -static -nostdlib -s

ifeq ($(SYS), sys_linux)
	CFLAGS += -DLINUX
endif

all:
	$(CC) $(CFLAGS) main.c $(SYS).s -o tiny_httpd
	objcopy --remove-section .comment      		tiny_httpd
	objcopy --remove-section .eh_frame     		tiny_httpd
	objcopy --remove-section .eh_frame_hdr 		tiny_httpd
	objcopy --remove-section .note.gnu.build-id	tiny_httpd

clean:
	rm -f *.o tiny_httpd
