PROGS=simple select insert create replace scan_files scan_lines
OBJS=$(patsubst %,%.o,$(PROGS))

CFLAGS=-g -Wall -Wextra
LDFLAGS=-lsqlite3

all: $(OBJS) $(PROGS)

# static pattern rule: multiple targets 

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< 

$(PROGS): %: %.o 
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJS) $(PROGS)
