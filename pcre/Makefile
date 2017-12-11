LDFLAGS=-lpcre

all: multi simple captures

multi: multi.o
simple: simple.o
captures: captures.o

.PHONY: clean

clean:
	rm -f *.o multi simple captures
