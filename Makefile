SYS = $(shell uname | grep -iq linux && echo sys_linux || echo sys_bsd)
SRC = main.c date.c event.c parser.c
OBJ = $(SRC:.c=.o)
CFLAGS = -Wall -Wextra -ansi -pedantic -O2 -ffreestanding
LDFLAGS = -static -nostdlib -s

ifeq ($(SYS), sys_linux)
	CFLAGS += -DLINUX
endif

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

tiny_httpd: $(OBJ) $(SYS).s
	$(CC) $(LDFLAGS) $^ -o $@
	objcopy --remove-section .comment      		$@
	objcopy --remove-section .eh_frame     		$@
	objcopy --remove-section .eh_frame_hdr 		$@
	objcopy --remove-section .note.gnu.build-id	$@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRC)
	$(CC) -MM $^ >$@

clean:
	rm -f *.o tiny_httpd deps.mk
