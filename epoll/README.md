This is a standard template for an epoll-based, signal handling,
event driven C program. It is minimal- it only waits for a signal.
Every few seconds it does background work. A real application
would put some application logic in and handling of additional
file descriptors.

The second example keys.c shows using epoll with the same set of
descriptors as well as reading from the keyboard, with key echo off.
