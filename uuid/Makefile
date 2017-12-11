# expects uuid-dev package (Ubuntu) or similar

all: get_uuid

get_uuid: get_uuid.c
	$(CC) -o $@ $< -luuid

.PHONY: clean

clean: 
	rm -f get_uuid
