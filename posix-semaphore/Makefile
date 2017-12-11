PROGS=psem-create psem-wait psem-post psem-unlink psem-getvalue

OBJS=$(patsubst %,%.o,$(PROGS))
all: $(OBJS) $(PROGS) 

CFLAGS=-pthread 
LDFLAGS=

# static pattern rule: multiple targets 

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< 

$(PROGS): %: %.o
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)


.PHONY: clean

clean:
	rm -f $(OBJS) $(PROGS)
