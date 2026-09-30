# Advanced File I/O Mental Model

Chapter 4 adds several different tools. They solve different bottlenecks and
should not be treated as interchangeable optimizations.

## One Purpose per Interface

| Interface | Main question it answers |
|---|---|
| `readv()`, `writev()` | Can one I/O operation use several memory regions? |
| `epoll` | Which watched descriptors can make progress now? |
| `mmap()` | Can file bytes be accessed through virtual-memory addresses? |
| `mprotect()` | Which accesses may this page range perform? |
| `msync()` | How should dirty shared pages be synchronized? |
| `madvise()`, `posix_fadvise()` | What access pattern should the kernel expect? |
| POSIX AIO | Can work be submitted before its result is collected? |
| I/O scheduler | In what order should block requests reach storage? |

## Vectored I/O

~~~text
iovec[0] ─┐
iovec[1] ─┼─ one readv/writev request ─ kernel ─ file or descriptor
iovec[2] ─┘
~~~

The vector describes memory, not separate files. The kernel processes entries
in array order. The return value is one total byte count, so short operations
must be mapped back across the vector by the caller.

## Readiness with epoll

~~~text
interest set ── epoll_ctl() ──> epoll instance
                                      │
ready descriptors <── epoll_wait() ───┘
~~~

`epoll` reports readiness; it does not transfer application data. The program
still calls `read()`, `write()`, `accept()`, or another operation afterward.

- Level-triggered: report the descriptor while the condition remains true.
- Edge-triggered: report a transition; use nonblocking I/O and drain until
  `EAGAIN`.

## File-backed Mapping

~~~text
process virtual addresses
        │ page-table translation
        ▼
page-cache pages  <──>  file  <──>  storage
~~~

`mmap()` returns a virtual address range. Pages are normally populated on
demand. Closing the original descriptor does not remove a successful mapping.
`munmap()` ends the mapping. A mapping is page-granular even when the requested
length is not.

- `MAP_SHARED`: writes can change the file and be seen by other mappings.
- `MAP_PRIVATE`: writes use copy-on-write and do not update the file.
- `msync()`: controls synchronization; it is not required merely to make a
  process's own mapped writes readable through the page cache.

## Advice and Asynchrony

Advice calls describe expected behavior. They do not promise that data is
already resident or that a future operation cannot block.

POSIX AIO separates submission from collection:

~~~text
stable aiocb + stable buffer → submit → pending → complete → aio_return()
~~~

The control block and buffer must remain alive and unchanged until completion
is collected.

## Performance Perspective

Measure before optimizing. Device type, filesystem, page cache, virtualized
storage, request merging, and the active multi-queue scheduler can all change
the result. A rule developed for a rotating disk may be irrelevant—or
counterproductive—on SSD, NVMe, or memory-backed storage.
