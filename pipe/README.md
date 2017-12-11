Example of pipe

See pipe(2).

A call to pipe() results in a pair of file descriptors, the "write end" and the 
"read end". A typical usage is that the pipe is created, then the process
forks, then the writer closes the read end, and vice versa. Then the writer
writes and the reader reads. A pipe is uni-directonal from writer to reader.

After a fork, it is important to have the reader close the write end, to detect
EOF properly, since EOF only occurs when all the write descriptors are closed.

Examples

* pipe1: simple use of pipe between processes
