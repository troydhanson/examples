all: slurp mmap special hexdump

CFLAGS=-Wall -Wextra

slurp: slurp.c
mmap: mmap.c
hexdump: hexdump.c
special: special.c

.PHONY: clean

clean:
	rm -f *.o slurp mmap special hexdump
