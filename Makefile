SYS = $(shell uname | grep -iq linux && echo sys_linux || echo sys_bsd)
CFLAGS = -Wall -Wextra -ansi -pedantic -O2 -ffreestanding
LDFLAGS = -static -nostdlib -s

ifeq ($(SYS), sys_linux)
	CFLAGS += -DLINUX
endif

tiny_httpd: main.o parser.o $(SYS).s
	$(CC) $(LDFLAGS) $^ -o $@
	objcopy --remove-section .comment      		$@
	objcopy --remove-section .eh_frame     		$@
	objcopy --remove-section .eh_frame_hdr 		$@
	objcopy --remove-section .note.gnu.build-id	$@

main.o: main.c config.h sys.h
	$(CC) $(CFLAGS) -c $< -o $@

parser.o: parser.c parser.h config.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o tiny_httpd
