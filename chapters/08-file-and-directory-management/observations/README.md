# Chapter 8 Observations

Store deliberate Chapter 8 evidence here. Generated traces are ignored by Git
unless explicitly selected for long-term study.

Suggested observations:

- compare `stat()` and `lstat()` on the same symbolic-link pathname;
- inspect inode and link counts with `ls -li` after hard-link operations;
- watch `newfstatat`, `fchmod`, `mkdir`, `link`, `symlink`, `unlink`, and
  `rename` in `strace` output;
- prove that an open descriptor remains usable after unlink or rename;
- compare directory-stream order across filesystems without depending on it;
- inspect extended attributes with `getfattr` when that utility is installed;
- correlate paired inotify move events with their cookie;
- measure pending inotify bytes with `FIONREAD`;
- explain why a directory watch is not recursive;
- use `/proc/self/fd` to connect descriptors to currently resolved pathnames.

Do not commit enormous raw traces. Preserve short, explained evidence that
answers a specific prediction.
