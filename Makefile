SYS = $(shell uname | grep -iq linux && echo sys_linux || echo sys_bsd)
CFLAGS = -Wall -Wextra -ansi -pedantic -O2 -static -nostdlib -s

ifeq ($(SYS), sys_linux)
	CFLAGS += -DLINUX
endif

tiny_httpd: main.c parser.c $(SYS).s
	$(CC) $(CFLAGS) $^ -o $@
	objcopy --remove-section .comment      		$@
	objcopy --remove-section .eh_frame     		$@
	objcopy --remove-section .eh_frame_hdr 		$@
	objcopy --remove-section .note.gnu.build-id	$@

all: tiny_httpd

clean:
	rm -f *.o tiny_httpd
