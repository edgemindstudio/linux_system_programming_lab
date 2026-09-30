# Chapter 4 Observations

Keep deliberately retained evidence from Chapter 4 investigations here.
Generated data and ordinary test output belong under
`build/chapters/04-advanced-file-io/` and are not committed.

The tracing helper writes ignored traces to `observations/strace/`:

~~~bash
make chapter CHAPTER=04
chapters/04-advanced-file-io/scripts/trace.sh 01_writev_record
~~~

Useful observations include:

- the single `writev()` behind a multi-buffer record;
- the ready set returned from `epoll_wait()`;
- mapping, protection, advice, and synchronization system calls;
- the implementation behavior behind POSIX AIO;
- scheduler names exposed in `/sys/block/*/queue/scheduler`.

The lab deliberately does not automate `FIBMAP`. That ioctl commonly needs
`CAP_SYS_RAWIO`, depends on filesystem support, and exposes physical-layout
details. Chapter concepts can be learned without granting a study program
elevated privileges.
