POSIX semaphors provide an atomic post/wait mechanism. They come in named
and unnamed versions.  The examples here are based on The Linx Programming
Interface by Michael Kerrisk. They show usage of named semaphores. The Linux
implementation of named semaphores, available since kernel 2.6, internally
creates the semaphore in /dev/shm/sem.name.

As with the usual semaphore behavior- each post increments its value, and each
wait decrements it- immediately if it's already positive, or blocking until it
becomes positive first. Thus a producer can use it to notify a consumer of how
many units are available to consume, by posting that many times.

    % ./psem-create -c /demo
    % ./psem-wait /demo &
    % ./psem-getvalue /demo
    % ./psem-post /demo
    % ./psem-unlink /demo
