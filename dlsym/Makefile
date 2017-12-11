LDFLAGS=-ldl
all: subject.so dlsym

subject.so: subject.c
	$(CC) -shared -o $@ $<

dlsym: dlsym.o

.PHONY: clean

clean:
	rm -f subject.so dlsym *.o
