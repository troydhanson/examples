# POSIX message queues. 

Message queues allow processes to exchange messages by opening the same queue.
See `mq_overview(7)`. On Linux, message queues:

 * are identified using a flat, absolute namespace e.g. /somename
 * persist until a process uses `mq_unlink` or the system reboots
 * descriptors work with epoll; on Linux, they're file descriptors
 * min/max message sizes and queue lengths in /proc/sys/fs/mqueue
 * require applications link with `-lrt`
 * are listed in /dev/mqueue

See `mq-read.c` and `mq-write.c`. Try them out:

    % make
    % ./mq-read /foo 

Now in another terminal,

    % ./mq-write /foo hello

You can play around with read/write to see how the queue will accept up to
10 messages before the writer blocks. Run the reader repeatedly to drain it.

Inspect the queue via the /proc filesystem:

    % cat /dev/mqueue/foo
    QSIZE:0          NOTIFY:0     SIGNO:0     NOTIFY_PID:0     

QSIZE shows the number of bytes queued across all messages. The other fields
relate to `mq_notify(3)`.

If /dev/mqueue is not already mounted, `sem_overview(7)` says to mount it using:

    % mkdir /dev/mqueue
    % mount -t mqueue none /dev/mqueue

When you are done, unlink the message queue. A reboot would also unlink it.

    % ./mq-unlink /foo

You could also use the /dev/mqueue filesystem to remove a queue:

    % rm /dev/mqueue/foo

