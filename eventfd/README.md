Example of eventfd

See eventfd(2).

An eventfd file descriptor has an internal 8-byte counter whose value is set
at creation and added to by write(). A read() blocks if the counter is zero.
If the counter is non-zero, the read() behaves as such:

* in regular mode, the read returns the counter value. Counter is reset to zero. 
* in semaphore mode, the read returns 1 (in 8 bytes). Internal counter decrements.

The current value of the internal counter is exposed in /proc/<pid>/fdinfo/<fd>.

    % cat /proc/6188/fdinfo/3
    pos:	0
    flags:	02
    mnt_id:	11
    eventfd-count:               64

Above, the internal counter's current value is 0x64 (decimal 100).

Examples

* event1: simple use of eventfd between processes
* event2: shows how multiple writes add together
* event3: shows looking at the counter value in /proc
* event4: semaphore mode, each read returns 1 til counter reaches 0
