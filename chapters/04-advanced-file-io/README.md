# Chapter 4 — Advanced File I/O

**Status:** Lab prepared; study in progress

## Purpose

Move beyond one-buffer, one-descriptor I/O. This chapter studies operations
that combine buffers, monitor many descriptors, map files into virtual memory,
give access-pattern advice, submit asynchronous work, and reason about storage
performance.

## Book Scope

Robert Love, *Linux System Programming*, second edition, Chapter 4, printed
pages 91–135.

## Study Material

- `notes/study-guide-pages-91-135.md`
- `mental-models/advanced-file-io.md`

## Exercises

1. Build one logical record with `writev()`
2. Split one input operation across buffers with `readv()`
3. Manage the `epoll` add, wait, modify, and delete lifecycle
4. Distinguish an `epoll_wait()` timeout from an error
5. Observe level-triggered readiness
6. Drain an edge-triggered, nonblocking descriptor to `EAGAIN`
7. Read a file through `mmap()` after closing its descriptor
8. Make a shared mapping visible to the underlying file
9. Observe copy-on-write with a private mapping
10. Respect page size and mapping-offset alignment
11. Resize a mapping with Linux `mremap()`
12. Change page protection with `mprotect()`
13. Give the kernel mapping advice with `madvise()`
14. Give file-access advice with `posix_fadvise()`
15. Complete a POSIX asynchronous read
16. Sort files by inode as a locality heuristic

## Experiments

- compare linear and vectored writes;
- monitor many descriptors while returning only the ready set;
- observe descriptor and mapping coherence around `msync()`;
- count page faults while touching a mapping;
- probe Linux `readahead()` support;
- keep several POSIX AIO reads in flight;
- inspect the I/O schedulers exposed by the current Linux system.

## Commands

~~~bash
make chapter CHAPTER=04
make test CHAPTER=04
make tidy-chapter CHAPTER=04
chapters/04-advanced-file-io/scripts/trace.sh 01_writev_record
chapters/04-advanced-file-io/scripts/trace.sh linear_vs_vectored vectored
~~~

Generated files are placed under:

~~~text
build/chapters/04-advanced-file-io/
~~~

## Completion Rule

Building the lab is not the same as mastering it. Before moving to Chapter 5,
predict which descriptors become ready, explain the lifetime of a mapping,
distinguish visibility from durability, identify which interfaces merely give
hints, and explain why historical disk-scheduling guidance depends on the
actual storage stack.
