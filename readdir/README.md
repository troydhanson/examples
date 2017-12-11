Show usage of readdir(3) to iterate over a directory.

An example of reading a directory, and doing a optional 
suffix match on the filenames, is in readdir-suffix.c

    % ./readdir-suffix -d /etc -s .conf
    suffix match: logrotate.conf
    suffix match: debconf.conf
    suffix match: ltrace.conf

The directory entry has a field called `d_type` which
seems useful in distinguishing a regular file from a
directory or block device, fifo or other kind of file.
The readdir program prints a string representation of 
this field.

    % /readdir /etc
     1: inode 655913 type DT_DIR update-motd.d
     2: inode 655635 type DT_DIR iproute2
     3: inode 655706 type DT_REG networks

However, on some filesystems `d_type` is not populated.
We can call lstat(2) on each entry to get a more robust
set of mode bits. This is shown in readdir-lstat.c.

    % ./readdir-lstat /etc
     1: inode 655913 type DT_DIR mode directory name update-motd.d
     2: inode 655635 type DT_DIR mode directory name iproute2
     3: inode 655706 type DT_REG mode file name networks

