Here's a link back to my [main GitHub page](http://troydhanson.github.io/).

# Three ways to read a whole file in C

The word "slurp" is sometimes used for reading a file all at once, in contrast
to reading it line-by-line or in records. In C it is easy to slurp a file.

* slurp.c: get file size, allocate buffer, read file into buffer
* mmap.c: get file size, map file into memory
* special.c: use realloc/read to read file 

## Memory mapped files

It is convenient to map a file into memory. This avoids the need to allocate
a buffer and copy the file contents into the buffer. When a file is mapped,
its descriptor can be closed and the mapped region remains intact. The code
should eventually call `munmap()` to release the mapping.

### hexdump

As an example, `hexdump.c` maps a file into memory and dumps it in hex.

## Special files

For files such as those in /proc on Linux, stat reports `st_size` as 0 bytes.
These files get generated programmatically only when read.  For special files,
we pick a buffer size, then try to read the whole file into it.  If needed,
realloc is used to grow the buffer until the whole file fits.

