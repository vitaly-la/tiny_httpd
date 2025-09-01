SYS != [ "$(uname)" = Linux ] && echo sys_linux || echo sys_bsd

all:
	nasm -f elf64 $(SYS).asm
	$(CC) -std=c99 -O0 -Wall -Wextra -static -nostdlib -s \
		main.c $(SYS).o -o tiny_httpd

clean:
	rm -f *.o tiny_httpd
