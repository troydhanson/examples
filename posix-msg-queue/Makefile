PROGS=mq-read mq-write mq-unlink
OBJS=$(patsubst %,%.o,$(PROGS))

CFLAGS=-g -Wall -Wextra
LDFLAGS=-lrt

all: $(OBJS) $(PROGS)

# static pattern rule: multiple targets 

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< 

$(PROGS): %: %.o 
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJS) $(PROGS)
