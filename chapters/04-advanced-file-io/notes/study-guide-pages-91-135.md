# Chapter 4 Study Guide — Advanced File I/O

Book: Robert Love, *Linux System Programming*, second edition

Scope: Chapter 4, printed pages 91–135

Repository: `linux_system_programming_lab`

## How to use this guide

This chapter is easiest to learn by asking what problem each interface solves.
Do not memorize a list of system calls. Before running a program, predict:

1. what state exists in the process;
2. what state exists in the kernel;
3. which call crosses the user/kernel boundary;
4. what the return value actually counts;
5. what remains valid after the call;
6. which behavior is guaranteed and which behavior is only a hint.

Build and test the whole chapter from the repository root:

~~~bash
make chapter CHAPTER=04
make test CHAPTER=04
~~~

Each file under `exercises/` is a complete program, not a fragment. Generated
files are isolated under `build/chapters/04-advanced-file-io/data/`.

## Chapter summary

Chapter 2 introduced descriptor-based operations such as `open()`, `read()`,
`write()`, and `lseek()`. Chapter 3 placed C library streams and user-space
buffers above those descriptors. Chapter 4 explores more specialized paths:

- scatter/gather I/O transfers between one descriptor and several buffers;
- `epoll` watches many descriptors and returns the subset that is ready;
- memory mapping exposes file-backed page-cache data through virtual addresses;
- protection and synchronization calls control mapped regions;
- advice interfaces tell the kernel about likely access patterns;
- asynchronous I/O separates submitting work from collecting its result;
- I/O scheduling and locality affect performance below the file API.

These are not replacements for ordinary I/O. They are tools for particular
shapes of work.

## A map from the earlier chapters

| Earlier model | Chapter 4 extension |
|---|---|
| One buffer per `read()` or `write()` | Several buffers with `readv()` or `writev()` |
| Wait on or poll a descriptor | Maintain a scalable interest set with `epoll` |
| Copy file bytes into a buffer | Access file-backed pages through `mmap()` |
| Let the kernel infer access patterns | Supply hints with advice calls |
| Call and wait for an I/O result | Submit AIO and collect completion later |
| Think in pathname order | Consider cache, inode locality, and storage queues |

---

## Part I — Scatter/gather I/O

### 1. The core idea

Ordinary `write()` accepts one continuous buffer. Real records are often
already split into a header, payload, and trailer. Copying those pieces into a
temporary combined buffer costs CPU time and memory bandwidth. `writev()` can
describe the pieces directly.

~~~c
struct iovec {
    void  *iov_base;
    size_t iov_len;
};
~~~

An array of these structures is called an I/O vector. For `writev()`, the
kernel reads each region in array order. For `readv()`, the kernel fills each
region in array order before continuing to the next.

The calls are conceptually:

~~~c
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
~~~

The return value is one total byte count across the entire vector. It is not a
count of completed vector elements.

### 2. `writev()` record exercise

Complete program:
`exercises/01_writev_record.c`

Run it:

~~~bash
./build/chapters/04-advanced-file-io/exercises/01_writev_record
~~~

Expected output:

~~~text
segments=3 bytes=30 record=type=event value=42 status=ok
~~~

The logical record begins in three separate arrays. The program's
`writev_all()` helper handles partial progress by advancing through fully
written entries and adjusting the base address and length of a partially
written entry.

That adjustment matters. A successful return smaller than the requested total
is not an error. Retrying the original vector would duplicate the bytes that
were already written.

### 3. `readv()` segment exercise

Complete program:
`exercises/02_readv_segments.c`

~~~bash
./build/chapters/04-advanced-file-io/exercises/02_readv_segments
~~~

Expected output:

~~~text
bytes=19 header=HDR1 payload=payload-data trailer=END
~~~

One call fills three buffers. Notice that the program reserves an extra byte
for each terminating null character. `readv()` transfers raw bytes; it does
not turn those bytes into C strings.

### 4. Limits and atomicity

The vector count cannot be arbitrarily large. Linux exposes a runtime limit:

~~~c
long maximum = sysconf(_SC_IOV_MAX);
~~~

`IOV_MAX` may also be available as a compile-time constant. The sum of all
lengths must fit in the supported return-value range.

Vectored I/O is one operation, but that does not mean every possible target
provides unlimited atomicity. A pipe, for example, guarantees noninterleaving
only up to `PIPE_BUF`. Regular-file concurrency also depends on offsets and
open-file-description sharing. “One system call” and “indivisible under every
circumstance” are different claims.

### 5. Linear versus vectored experiment

~~~bash
./build/chapters/04-advanced-file-io/experiments/linear_vs_vectored linear
./build/chapters/04-advanced-file-io/experiments/linear_vs_vectored vectored
cmp build/chapters/04-advanced-file-io/data/linear_output.txt \
    build/chapters/04-advanced-file-io/data/vectored_output.txt
~~~

Expected program output includes:

~~~text
mode=linear bytes=17
mode=vectored bytes=17
~~~

Both paths create the same bytes. Use the tracing helper to see the different
system-call shapes:

~~~bash
chapters/04-advanced-file-io/scripts/trace.sh \
    linear_vs_vectored vectored
~~~

### Scatter/gather mistakes

- Treating the return value as a number of vectors rather than bytes.
- Retrying an unchanged vector after partial progress.
- Forgetting that `iov_base` points to storage that must stay valid during the
  call.
- Assuming input bytes automatically receive string terminators.
- Passing more entries than the system supports.
- Assuming vectored I/O is faster without measuring copies and system calls.

---

## Part II — Scalable readiness with `epoll`

### 6. Readiness is not data transfer

`epoll` answers a narrow question: which watched file descriptors are ready
for an operation now? It does not read bytes, write bytes, or accept a socket
connection. The application performs that operation after receiving the
event.

The main objects are:

1. an epoll file descriptor, created with `epoll_create1()`;
2. an interest set, changed with `epoll_ctl()`;
3. a caller-provided event array, filled by `epoll_wait()`.

~~~c
int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
~~~

`EPOLL_CLOEXEC` prevents an unintended descriptor leak through a later
`exec()`.

### 7. Interest-set lifecycle

Complete program:
`exercises/03_epoll_readiness.c`

~~~bash
./build/chapters/04-advanced-file-io/exercises/03_epoll_readiness
~~~

Expected output:

~~~text
events=1 readable=yes message=ready lifecycle=add-mod-del
~~~

The exercise demonstrates all three `epoll_ctl()` operations:

| Operation | Meaning |
|---|---|
| `EPOLL_CTL_ADD` | Begin watching a descriptor |
| `EPOLL_CTL_MOD` | Change its event mask or attached data |
| `EPOLL_CTL_DEL` | Remove it from the interest set |

An `epoll_event` has an event mask and a data union. Store one meaningful form
in that union, such as `.data.fd` or `.data.ptr`, and interpret it consistently.

### 8. Waiting and timeouts

Complete program:
`exercises/04_epoll_timeout.c`

~~~bash
./build/chapters/04-advanced-file-io/exercises/04_epoll_timeout
~~~

Expected output:

~~~text
timeout_ms=25 ready_events=0 outcome=timeout
~~~

The important return cases from `epoll_wait()` are:

| Return | Meaning |
|---:|---|
| positive | Number of populated entries in the event array |
| zero | Timeout expired; this is not an error |
| `-1` | Failure; inspect `errno` |

A signal can interrupt the wait with `EINTR`. The example retries that case.
Whether a real application should retry immediately or handle a shutdown flag
depends on its event-loop design.

A timeout of zero polls without waiting. A timeout of `-1` waits indefinitely.

### 9. Level-triggered behavior

Complete program:
`exercises/05_epoll_level_triggered.c`

~~~bash
./build/chapters/04-advanced-file-io/exercises/05_epoll_level_triggered
~~~

Expected output:

~~~text
first_wait=1 second_wait=1 first=A remaining=BC
~~~

The pipe initially contains three bytes. The program reads only `A`. Because
`BC` remains, the descriptor is still readable and a zero-timeout second wait
reports it again. This is the default level-triggered model: the condition is
reported while it remains true.

### 10. Edge-triggered behavior

Complete program:
`exercises/06_epoll_edge_triggered.c`

~~~bash
./build/chapters/04-advanced-file-io/exercises/06_epoll_edge_triggered
~~~

Expected output:

~~~text
first_wait=1 second_wait=0 first=A drained=2 stopped=EAGAIN
~~~

With `EPOLLET`, the transition into readiness produces the notification.
Reading only `A` does not create another transition for the already-present
`BC`. The safe pattern is:

1. make the descriptor nonblocking;
2. wait for an edge;
3. repeat the operation until it would block;
4. stop on `EAGAIN` or `EWOULDBLOCK`.

Without nonblocking mode, “drain until empty” can block forever after the last
available byte.

### 11. Many-descriptor experiment

~~~bash
./build/chapters/04-advanced-file-io/experiments/epoll_many_fds
~~~

Expected output:

~~~text
registered=64 ready=3 returned_only_ready=yes
~~~

The interest set contains 64 read descriptors, but only three are made ready.
`epoll_wait()` returns those ready events instead of requiring the application
to scan every registered descriptor on each iteration.

### `epoll` mistakes

- Believing an event means an operation must succeed forever; readiness can
  change between observation and use.
- Ignoring `EPOLLERR` and `EPOLLHUP`.
- Reusing only one event-array slot but telling `epoll_wait()` it has more.
- Using edge-triggered mode with blocking descriptors.
- Failing to drain an edge-triggered descriptor to `EAGAIN`.
- Treating a timeout return of zero as a failure.
- Forgetting that the epoll instance itself is a descriptor and must be closed.

---

## Part III — Memory-mapped files

### 12. What `mmap()` changes

With `read()`, the application explicitly requests a copy into a buffer. With
`mmap()`, the application asks the kernel to associate a virtual-address range
with a file region. Normal loads and stores then access that range.

~~~c
void *address = mmap(NULL, length, protection, flags, fd, offset);
~~~

A failed mapping returns `MAP_FAILED`, not `NULL`.

The kernel usually does not read every page when `mmap()` returns. The first
access to a missing page causes a page fault. The kernel resolves the fault,
connects a physical page or page-cache page to the process page tables, and
restarts the instruction.

### 13. Mapping lifetime

Complete program:
`exercises/07_mmap_read.c`

~~~bash
./build/chapters/04-advanced-file-io/exercises/07_mmap_read
~~~

Expected output:

~~~text
descriptor_closed=yes mapped_text=mapped bytes remain available
~~~

Once `mmap()` succeeds, the descriptor can be closed without invalidating the
mapping. The kernel maintains the mapping's own reference to the underlying
file. The virtual address remains valid until `munmap()`, process exit, or a
replacement mapping removes it.

Do not infer that the mapping is immune to every file change. If another actor
truncates the file below a page the process later touches, that access can
raise `SIGBUS`.

### 14. Shared and private mappings

The flags choose the relationship between stores and the file.

| Flag | Store behavior |
|---|---|
| `MAP_SHARED` | Dirty shared pages can update the mapped file |
| `MAP_PRIVATE` | Stores create private copy-on-write pages |

Shared exercise:

~~~bash
./build/chapters/04-advanced-file-io/exercises/08_mmap_shared_write
~~~

Expected output:

~~~text
mapping=shared!! file=shared!! msync=yes
~~~

Private exercise:

~~~bash
./build/chapters/04-advanced-file-io/exercises/09_mmap_private_copy
~~~

Expected output:

~~~text
memory=private! file=original copy_on_write=yes
~~~

`MAP_PRIVATE` does not mean the entire file is copied at mapping time. Pages
can initially share file-backed data. A write fault creates a private page for
the writing process.

### 15. Protection versus open mode

The `prot` argument describes permitted memory accesses:

- `PROT_READ` permits loads;
- `PROT_WRITE` permits stores;
- `PROT_EXEC` permits instruction fetches;
- `PROT_NONE` permits no access.

The mapping request must also be compatible with how the file was opened. For
example, a writable shared mapping normally needs a descriptor open for
writing. Protection bits are checked by the virtual-memory subsystem when the
process accesses pages.

### 16. Pages and aligned offsets

Complete program:
`exercises/10_page_size_alignment.c`

~~~bash
./build/chapters/04-advanced-file-io/exercises/10_page_size_alignment
~~~

Typical output on a system with 4096-byte pages:

~~~text
page_size=4096 offset=4096 aligned=yes marker=PAGE2
~~~

The actual page size is discovered at runtime, so it may differ. A file offset
passed to `mmap()` must satisfy the system's alignment requirement. Mapping
lengths are rounded internally to page boundaries, although access beyond the
logical object can still be invalid.

A zero-length mapping is invalid. Use `fstat()` first and handle an empty file
without calling `mmap()`.

### 17. Unmapping

~~~c
if (munmap(address, length) == -1) {
    perror("munmap");
}
~~~

After successful `munmap()`, pointers into that region are invalid. This is
the mapping equivalent of a use-after-free bug. Keep ownership and cleanup
paths clear, especially after partial initialization failures.

### 18. Resizing with `mremap()`

Complete Linux-specific program:
`exercises/11_mremap_resize.c`

~~~bash
./build/chapters/04-advanced-file-io/exercises/11_mremap_resize
~~~

Typical output:

~~~text
old_size=4096 new_size=8192 preserved=yes expanded=yes
~~~

With `MREMAP_MAYMOVE`, the kernel may relocate the mapping. Therefore the old
address must be considered invalid after success. Always store and use the
returned address. `mremap()` is a Linux extension, not a portable POSIX API.

### 19. Changing protection with `mprotect()`

Complete program:
`exercises/12_mprotect_region.c`

~~~bash
./build/chapters/04-advanced-file-io/exercises/12_mprotect_region
~~~

Expected output:

~~~text
text=protected mapping protection=read-only page_granularity=yes
~~~

The program writes while the anonymous mapping is read/write and then changes
it to read-only. It does not intentionally crash by writing afterward. The
important lesson is that protection applies at page granularity. Runtime
systems use the same underlying idea for guard pages, read-only metadata, and
switching generated code between writable and executable states.

### 20. `msync()`: visibility and durability are different

`msync()` operates on a mapped address range.

- `MS_ASYNC` schedules writeback and returns without waiting for completion.
- `MS_SYNC` waits for the requested synchronization.
- `MS_INVALIDATE` asks to invalidate other cached copies where appropriate.

Mapped writes to a shared page and descriptor reads commonly interact through
the same page cache. Thus a descriptor read can observe a mapped write before
an explicit `msync()`. That coherence is not the same as durable persistence
on physical storage.

Run the experiment:

~~~bash
./build/chapters/04-advanced-file-io/experiments/mmap_coherence
~~~

Expected output:

~~~text
descriptor_before_msync=warm coherent=yes synchronization=done
~~~

The first observation is about visibility through the kernel cache. The later
successful `MS_SYNC` request is about synchronization. Storage devices and
filesystems add further durability details; applications that require crash
consistency need a protocol, not merely one optimistic call.

### 21. Page-fault experiment

~~~bash
./build/chapters/04-advanced-file-io/experiments/page_faults
~~~

Example output:

~~~text
pages_touched=1024 minor_fault_delta=64 major_fault_delta=1 checksum=0
~~~

The fault counts are measurements, not fixed expected values. Readahead,
existing cache state, filesystem behavior, and the environment change them.
A minor fault does not require storage I/O; a major fault does. One observed
fault can make several following pages resident.

### Mapping mistakes

- Testing the result against `NULL` instead of `MAP_FAILED`.
- Mapping a zero-length file.
- Passing a misaligned file offset.
- Using a protection incompatible with descriptor access mode.
- Accessing a pointer after `munmap()` or after a moving `mremap()`.
- Assuming `MAP_PRIVATE` changes the file.
- Assuming `MAP_SHARED` means bytes are already durable on the device.
- Ignoring `SIGBUS` risk when a mapped file is truncated.
- Treating a mapping as a universal performance win without measuring faults.

---

## Part IV — Access-pattern advice and readahead

### 22. Advice is a hint

Linux and POSIX expose calls that let an application describe expected access.
The kernel can use that information for readahead and cache decisions. Advice
does not reserve memory, guarantee cache residence, or make a future read
nonblocking.

### 23. `madvise()` for an address range

Complete program:
`exercises/13_madvise_mapping.c`

~~~bash
./build/chapters/04-advanced-file-io/exercises/13_madvise_mapping
~~~

Typical output:

~~~text
advice=sequential+willneed bytes=8192 checksum=155 accepted=yes
~~~

Common advice values include:

| Advice | Intended meaning |
|---|---|
| `MADV_NORMAL` | No special pattern |
| `MADV_RANDOM` | Random access; aggressive readahead may not help |
| `MADV_SEQUENTIAL` | Access likely moves forward |
| `MADV_WILLNEED` | Pages likely needed soon |
| `MADV_DONTNEED` | Pages not expected to be needed soon |

The exact effect can vary by kernel, mapping type, and advice value.

### 24. `posix_fadvise()` for a file range

Complete program:
`exercises/14_posix_fadvise.c`

~~~bash
./build/chapters/04-advanced-file-io/exercises/14_posix_fadvise
~~~

Expected output:

~~~text
advice=sequential+willneed range=whole-file accepted=yes
~~~

This API has an important error convention:

~~~c
int result = posix_fadvise(fd, offset, length, advice);
if (result != 0) {
    fprintf(stderr, "posix_fadvise: %s\n", strerror(result));
}
~~~

Unlike many system-call wrappers, `posix_fadvise()` returns an error number
directly. Do not test only for `-1`, and do not assume `errno` holds the error.
This corrects a common mistake in older examples and is worth remembering.

A length of zero means from the offset through the end of the file.

### 25. Linux `readahead()`

The Linux-specific `readahead()` call asks the kernel to initiate reading a
file range into the page cache. It can return before storage completes, and a
subsequent access can still block.

~~~bash
./build/chapters/04-advanced-file-io/experiments/readahead_probe
~~~

Typical supported output:

~~~text
readahead_result=accepted asynchronous_hint=yes
~~~

Some filesystems or environments do not support the request. The experiment
reports that outcome rather than declaring the whole chapter broken.

### Advice mistakes

- Treating advice as a command with guaranteed timing.
- Calling `perror()` for a direct-return error API such as
  `posix_fadvise()`.
- Assuming `WILLNEED` means the bytes are resident immediately.
- Applying `DONTNEED` and then expecting the next access to be cheap.
- Adding advice without measuring the actual workload.

---

## Part V — Synchronized, synchronous, and asynchronous I/O

### 26. Vocabulary

These words are similar but describe different guarantees.

| Term | Practical meaning |
|---|---|
| Synchronized write | The call's completion includes a stated level of data or metadata integrity |
| Synchronous operation | The caller waits while the operation completes |
| Asynchronous operation | The caller submits work and collects completion later |

An operation can be synchronous without guaranteeing that data survived a
power failure. Returning from ordinary `write()` generally means the kernel
accepted bytes, often into cache; it is not automatically a durability point.

Likewise, an asynchronous interface can eventually perform work through a
kernel mechanism or a library worker thread. The observable contract matters
more than assuming an implementation.

### 27. POSIX AIO lifecycle

The control block describes one operation:

~~~c
struct aiocb request;
memset(&request, 0, sizeof(request));
request.aio_fildes = fd;
request.aio_buf = buffer;
request.aio_nbytes = buffer_size;
request.aio_offset = 0;
~~~

The basic read lifecycle is:

1. initialize an `aiocb` and a stable data buffer;
2. submit with `aio_read()`;
3. observe `aio_error()` or wait with `aio_suspend()`;
4. once complete, call `aio_return()` exactly once;
5. only then reuse or release the request and buffer.

Complete program:
`exercises/15_asynchronous_read.c`

~~~bash
./build/chapters/04-advanced-file-io/exercises/15_asynchronous_read
~~~

Expected output:

~~~text
submitted=yes completed=yes bytes=17 text=asynchronous-data
~~~

`aio_error()` returning `EINPROGRESS` means the request remains active. Once it
returns zero, `aio_return()` provides the operation's byte count. If
`aio_error()` provides another error number, format that number with
`strerror()`.

### 28. Several requests in flight

~~~bash
./build/chapters/04-advanced-file-io/experiments/aio_parallel_reads
~~~

Expected output:

~~~text
requests=3 completed=3 data=alpha,bravo,gamma
~~~

All three requests are submitted before results are collected. This is the
central asynchronous opportunity: useful work or other requests can proceed
while operations are outstanding.

POSIX AIO is useful for learning the contract, but modern Linux applications
may also evaluate interfaces such as `io_uring` for appropriate workloads.
That newer API is outside this book chapter. Choose based on required
portability, kernel support, complexity, and measurement—not fashion.

### AIO mistakes

- Allocating the `aiocb` or buffer in storage that goes out of scope early.
- Modifying a request while it is active.
- Closing the descriptor before outstanding operations are resolved.
- Calling `aio_return()` before completion or more than once.
- Confusing submission success with operation success.
- Busy-spinning on `aio_error()` instead of using a completion strategy.
- Assuming every libc implements POSIX AIO using the same mechanism.

---

## Part VI — I/O scheduling and performance

### 29. Where scheduling fits

File APIs operate above the filesystem, page cache, block layer, device driver,
and storage hardware. When multiple block requests are pending, the block
layer and device can influence their order. On a rotating disk, seek distance
and rotational delay historically made physical locality extremely important.

The book discusses scheduler names and behavior from its publication era,
including anticipatory, deadline, CFQ, and noop approaches. Modern Linux uses
the multi-queue block layer on common systems, where names may include
`mq-deadline`, `kyber`, `bfq`, and `none`. Virtual disks and NVMe devices can
show still different choices.

Treat the historical discussion as a model of tradeoffs:

- throughput versus latency;
- fairness versus aggressive batching;
- reads versus writes;
- sequential locality versus request deadlines.

Do not assume an old scheduler name exists on the current machine.

### 30. Inspect this system

~~~bash
./build/chapters/04-advanced-file-io/experiments/scheduler_inventory
~~~

Output depends on available block devices. A line may resemble:

~~~text
device=vda schedulers=[none]
devices_inspected=1 historical_names_may_differ=yes
~~~

The bracketed scheduler is the active choice. Containers and WSL environments
may expose limited or virtualized information. Observation is still useful:
it shows why system-programming claims must be tied to the system actually
running the code.

### 31. Locality heuristics

When processing many files, order can influence I/O locality. Possible sorting
keys become progressively more filesystem-specific:

| Key | Benefit | Limitation |
|---|---|---|
| Pathname | Easy and portable | Weak relationship to physical placement |
| Inode number | Cheap metadata heuristic | Not a physical-block guarantee |
| Physical block | Closest to device layout | Privileged and filesystem-specific |

Complete inode-order program:
`exercises/16_inode_order.c`

~~~bash
./build/chapters/04-advanced-file-io/exercises/16_inode_order
~~~

The inode numbers vary, but the summary should include:

~~~text
files=3 ordered=yes strategy=inode-heuristic
~~~

The word “heuristic” is essential. Filesystems can relocate blocks, use
extents, layer encryption or copy-on-write, and map through virtual devices.
SSDs do not pay mechanical seek cost in the same way as rotating disks.

### 32. Why this lab does not automate `FIBMAP`

`FIBMAP` can translate a logical file block to a physical block on supporting
filesystems, but commonly requires `CAP_SYS_RAWIO`. Its output is tied to a
particular filesystem and block layout, and physical mappings can change.

Granting elevated privilege is unnecessary for the chapter's learning goals.
The lab explains the concept and uses an unprivileged inode-order heuristic
instead.

### 33. A performance method that survives hardware changes

1. Define the result that matters: throughput, tail latency, CPU cost, memory,
   or durability.
2. Record the storage stack: filesystem, mount options, device type,
   virtualization, scheduler, and kernel.
3. Separate cold-cache and warm-cache experiments.
4. Change one variable at a time.
5. Trace system calls and collect appropriate kernel metrics.
6. Run enough repetitions to see variability.
7. Keep correctness checks beside performance checks.
8. Re-measure on the deployment system.

Optimizing by folklore is especially dangerous in I/O because the page cache
can hide the device and a synthetic benchmark can accidentally measure memory.

---

## Complete program index

Every important idea in the chapter has a complete, compilable example.

| Program | Main idea |
|---|---|
| `01_writev_record.c` | Robust partial `writev()` progress |
| `02_readv_segments.c` | Scattered input buffers |
| `03_epoll_readiness.c` | Interest-set lifecycle |
| `04_epoll_timeout.c` | Timeout versus failure |
| `05_epoll_level_triggered.c` | Readiness persists while data remains |
| `06_epoll_edge_triggered.c` | Nonblocking drain to `EAGAIN` |
| `07_mmap_read.c` | Mapping lifetime after `close()` |
| `08_mmap_shared_write.c` | Shared mapped writes and `msync()` |
| `09_mmap_private_copy.c` | Private copy-on-write mapping |
| `10_page_size_alignment.c` | Runtime page size and aligned offsets |
| `11_mremap_resize.c` | Linux mapping resize and possible move |
| `12_mprotect_region.c` | Page-granular protection change |
| `13_madvise_mapping.c` | Advice for mapped memory |
| `14_posix_fadvise.c` | Advice for file ranges and direct error returns |
| `15_asynchronous_read.c` | Submit, wait, and collect POSIX AIO |
| `16_inode_order.c` | Locality heuristic rather than physical promise |

## Tracing the kernel boundary

Build first, then trace one program:

~~~bash
make chapter CHAPTER=04
chapters/04-advanced-file-io/scripts/trace.sh 08_mmap_shared_write
less chapters/04-advanced-file-io/observations/strace/08_mmap_shared_write.strace
~~~

Questions to ask while reading a trace:

- Which operations are system calls and which work occurs through ordinary
  memory instructions?
- When does the file descriptor close relative to the mapping lifetime?
- Does one logical operation use `write()` or `writev()`?
- Which calls merely register interest or advice?
- For AIO, does the libc implementation create or use helper threads?

Traces are evidence from one run, not universal API guarantees.

## Chapter-wide common mistakes

1. Choosing a sophisticated interface before identifying the bottleneck.
2. Ignoring partial byte counts and `EINTR`.
3. Confusing readiness with completion.
4. Using blocking I/O in an edge-triggered drain loop.
5. Comparing an `mmap()` result with `NULL`.
6. Forgetting page and offset alignment.
7. Using stale pointers after mapping changes.
8. Confusing page-cache visibility with storage durability.
9. Treating advice as a guarantee.
10. Handling `posix_fadvise()` like an `errno`-only system call.
11. Reusing an active AIO control block or buffer.
12. Applying rotating-disk advice blindly to SSD, NVMe, or virtual storage.
13. Running privileged physical-layout tools for an unprivileged learning goal.
14. Benchmarking without recording cache state and environment.

## Exercises to complete independently

### Exercise A — Four-part vectored log record

Extend `01_writev_record.c` with timestamp, severity, message, and newline
vectors. Force the helper to resume from a simulated partial position. Explain
which vector and byte offset are current after each step.

### Exercise B — Reusable epoll event loop

Create three nonblocking pipes. Register them with a small structure stored in
`.data.ptr`. Write to two pipes, wait, drain them to `EAGAIN`, and prove the
third pipe is not returned.

### Exercise C — Safe empty-file mapper

Write a utility that opens a path, calls `fstat()`, and prints a clear message
for a zero-byte file instead of calling `mmap()`. For a nonempty file, map it,
calculate a checksum, unmap it, and close all resources.

### Exercise D — Shared versus private comparison

Map the same input twice, once shared and once private. Change distinct bytes,
then inspect the mapping contents and descriptor-visible file contents. Predict
all three values before running.

### Exercise E — Protection boundary

Allocate two anonymous pages and protect only the second with `PROT_NONE`.
Install a temporary `SIGSEGV` handler or use a child process so the experiment
can report the fault safely. Explain why protection is page-granular.

### Exercise F — Advice measurement

Compare sequential traversal with and without sequential advice. Record file
size, cache state, elapsed time, page-fault counts, device, filesystem, and at
least ten trials. A valid conclusion may be “no meaningful difference.”

### Exercise G — AIO lifetime audit

Draw the lifetime of the descriptor, control blocks, and buffers in
`aio_parallel_reads.c`. Mark exactly where each may be released. Then add a
fourth request without copying and pasting the collection logic.

### Exercise H — Scheduler report

Run `scheduler_inventory`, identify whether the devices are physical or
virtual, and explain why the available scheduler list differs from the book's
historical list.

## Connections to compiler and runtime engineering

- Vectored I/O is useful when a runtime has metadata and payloads in separate
  allocations but wants fewer kernel crossings.
- `epoll` is a foundation for Linux event loops used by servers, language
  runtimes, and asynchronous frameworks.
- Memory mapping underlies executable loading, shared libraries, mapped object
  stores, JIT code regions, and guard pages.
- `mprotect()` enforces transitions such as writable-to-executable generated
  code and read-only runtime metadata.
- Page faults make apparently ordinary memory accesses enter the kernel.
- Advice calls expose a recurring systems principle: optimization hints may be
  ignored without changing correctness.
- AIO makes ownership and lifetime explicit, which is central to safe async
  runtimes.
- Scheduler history shows why performance knowledge must include hardware and
  kernel context.

## Mastery checklist

Before moving on, you should be able to explain without looking up the answer:

- [ ] how a short `writev()` maps back into an I/O vector;
- [ ] why `epoll` readiness is not operation completion;
- [ ] the difference between level-triggered and edge-triggered handling;
- [ ] why edge-triggered descriptors should be nonblocking;
- [ ] why a mapping survives closing its original descriptor;
- [ ] why `MAP_FAILED` is the correct failure sentinel;
- [ ] the difference between `MAP_SHARED` and `MAP_PRIVATE`;
- [ ] how page size constrains mapping offsets and protections;
- [ ] why a successful moving `mremap()` invalidates the old address;
- [ ] the difference between coherence, synchronization, and durability;
- [ ] why advice calls do not guarantee future nonblocking access;
- [ ] the direct error-return convention of `posix_fadvise()`;
- [ ] the lifetime rules of a POSIX AIO control block and buffer;
- [ ] why the book's scheduler names may not appear on a modern system;
- [ ] why inode ordering is a heuristic rather than physical proof;
- [ ] how to design a measurement that distinguishes cache from device I/O.

When those answers are connected to observed program behavior, Chapter 4 has
become part of your systems model rather than a list of APIs.
