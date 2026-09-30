# Chapter 3 Study Guide — Buffered I/O

Book: Robert Love, *Linux System Programming*, second edition
Scope: Chapter 3, printed pages 67–90
Repository: `linux_system_programming_lab`
Language: C17 on Linux

## How to Use This Chapter

Work in this order:

1. Read one concept section slowly.
2. State what you predict the matching program will do.
3. Read the complete, commented C source.
4. Build and run it.
5. Compare the output with your prediction.
6. Trace important programs with `strace`.
7. Explain where each byte exists at each stage.
8. Complete the independent exercises near the end of this guide.

Build this chapter:

~~~bash
make chapter CHAPTER=03
~~~

Run all deterministic checks:

~~~bash
make test CHAPTER=03
~~~

## Chapter Summary

Chapter 2 gave the program file descriptors and direct functions such as
`open()`, `read()`, `write()`, `lseek()`, and `close()`.

Chapter 3 adds another layer in user space: the C standard I/O library.

Instead of managing only an integer descriptor, application code can work with
a `FILE *` stream and functions such as:

- `fopen()` and `fclose()`;
- `fgetc()`, `fgets()`, and `fread()`;
- `fputc()`, `fputs()`, `fprintf()`, and `fwrite()`;
- `fseeko()`, `ftello()`, and `rewind()`;
- `fflush()`;
- `feof()`, `ferror()`, and `clearerr()`;
- `fileno()` and `fdopen()`;
- `setvbuf()`;
- `flockfile()`, `funlockfile()`, and unlocked operations.

The central idea is not merely that these names are more convenient. A stream
holds state in the process, including a user-space buffer. Many small
application operations can therefore become a much smaller number of system
calls.

What you must retain is:

> A `FILE *` is a stateful C-library object that manages an underlying file
> descriptor. Buffering can improve throughput, but it also means that bytes
> produced by the application may not yet have reached the kernel.

## 1. The Complete I/O Path

When an application uses `fputs()`, the path is usually:

~~~text
application data
      |
      | fputs()
      v
C-library FILE stream
      |
      | copy into user-space stream buffer
      v
pending bytes in this process
      |
      | buffer fills, newline policy, fflush(), or fclose()
      v
write(file_descriptor, ...)
      |
      v
Linux kernel and page cache
      |
      | later writeback or explicit synchronization
      v
storage device
~~~

This model has at least three distinct ideas of “written”:

1. The application gave bytes to the C library.
2. The C library gave bytes to the kernel.
3. The kernel made the bytes durable on storage.

These are not the same event.

`fwrite()` succeeding can mean bytes are accepted into the stream buffer.
`fflush()` moves pending stream bytes toward the underlying descriptor.
`fsync()` or `fdatasync()` concerns kernel-to-device durability.

## 2. Why User-Space Buffering Exists

System calls have overhead. The processor must cross from user mode into the
kernel, validate arguments, operate on kernel objects, and return.

Imagine producing 4,096 bytes one byte at a time.

Without a user-space buffer, the program might perform approximately:

~~~text
4,096 calls to write(fd, &byte, 1)
~~~

With a 4,096-byte stream buffer, the program can perform:

~~~text
4,096 calls to fputc(byte, stream)
one or a few calls to write(fd, buffer, amount)
~~~

The application still performs 4,096 logical operations. The expensive
user-to-kernel transitions are amortized across a larger transfer.

This is the meaning of user-buffered I/O.

## 3. Block Size Without the Confusion

Filesystems and storage operate efficiently in blocks. A process can still ask
`read()` for one byte; Linux accepts that request. Internally, however, the
kernel and device may have to work with a larger block or cached page.

Therefore:

- small logical operations are legal;
- many tiny system calls are often inefficient;
- odd transfer sizes can interact poorly with natural block boundaries;
- buffering lets the application retain convenient small operations while the
  library submits larger transfers.

Two values in this chapter must not be confused:

- `st_blksize` is the filesystem's preferred I/O transfer size reported by
  `stat()` or `fstat()`;
- `BUFSIZ` is a C-library constant suitable for a standard-I/O buffer.

They may differ because they belong to different layers.

Run:

~~~bash
./build/chapters/03-buffered-io/experiments/block_size
~~~

Typical output on the test environment:

~~~text
BUFSIZ=8192 preferred_block_size=4096
~~~

Your exact positive values may differ.

## 4. File Descriptors Versus FILE Streams

| Property | File descriptor | `FILE *` stream |
|---|---|---|
| Representation | Small integer | Pointer to C-library state |
| Managed by | Process/kernel interface | C library in user space |
| Core open function | `open()` | `fopen()` or `fdopen()` |
| Core input | `read()` | `fgetc()`, `fgets()`, `fread()` |
| Core output | `write()` | `fputc()`, `fputs()`, `fprintf()`, `fwrite()` |
| Positioning | `lseek()` | `fseeko()`, `ftello()`, `rewind()` |
| Close | `close()` | `fclose()` |
| User-space buffer | Not provided by the API | Normally present |
| EOF/error indicators | Return value and `errno` | Persistent stream indicators |
| Thread lock | Not part of descriptor abstraction | Associated with the stream |

A stream still needs a descriptor for ordinary file I/O. `fileno(stream)`
reveals it.

Run:

~~~bash
./build/chapters/03-buffered-io/exercises/01_standard_streams
~~~

Expected output:

~~~text
stdin  -> fd 0 (expected 0)
stdout -> fd 1 (expected 1)
stderr -> fd 2 (expected 2)
~~~

Shell redirection can change what these descriptors refer to while retaining
the same descriptor numbers.

## 5. Opening Streams with fopen()

The basic declaration is:

~~~c
FILE *fopen(const char *path, const char *mode);
~~~

On success, it returns a stream pointer. On failure, it returns `NULL` and sets
`errno`.

### Common mode strings

| Mode | Initial behavior | Input | Output |
|---|---|---:|---:|
| `"r"` | Existing file, position at beginning | Yes | No |
| `"r+"` | Existing file, position at beginning | Yes | Yes |
| `"w"` | Create or truncate | No | Yes |
| `"w+"` | Create or truncate | Yes | Yes |
| `"a"` | Create if needed, writes go to end | No | Yes |
| `"a+"` | Create if needed, writes go to end | Yes | Yes |

The optional `b` in modes such as `"rb"` and `"wb"` expresses binary mode.
Linux does not transform newline bytes between text and binary modes, but using
the `b` is still useful for portable intent.

### Update-stream direction changes

A stream opened with `+` permits both input and output. You must coordinate a
change of direction according to the standard:

- after output, use `fflush()` or a positioning operation before input;
- after input, use a positioning operation before output unless the input
  operation reached EOF.

The safest mental rule is: explicitly position the stream when switching
between reading and writing.

Run:

~~~bash
./build/chapters/03-buffered-io/exercises/02_fopen_modes
~~~

Expected output:

~~~text
contents after w+ then a:
alpha
beta
~~~

## 6. Bridging open() and fdopen()

`fdopen()` constructs a stream around an existing descriptor:

~~~c
FILE *fdopen(int fd, const char *mode);
~~~

Important ownership rule:

> After `fdopen()` succeeds, the stream owns the descriptor. Closing the stream
> with `fclose()` closes the underlying descriptor.

Do not call both `fclose(stream)` and `close(fd)` for the same owned descriptor.
That is a double-close bug, and the descriptor number might already have been
reused for a different resource.

The mode supplied to `fdopen()` must be compatible with the descriptor's access
mode. A descriptor opened `O_RDONLY` cannot validly become a `"w"` stream.

Run:

~~~bash
./build/chapters/03-buffered-io/exercises/03_fdopen_bridge
~~~

Typical output:

~~~text
open fd=3, stream fd=3, closed_by_fclose=yes
~~~

The value `3` is typical but not guaranteed. The important fact is that both
layers refer to the same descriptor and `fclose()` releases it.

## 7. Closing Streams Correctly

`fclose()` performs two logically important jobs for an output stream:

1. It flushes pending user-space output.
2. It closes the underlying descriptor.

Therefore, check its return value:

~~~c
if (fclose(stream) == EOF) {
    perror("fclose");
}
~~~

An earlier `fputs()` may appear to succeed because it only copied bytes into a
user-space buffer. A failure can surface later when `fclose()` tries to flush.

The GNU C library also provides `fcloseall()`, which attempts to close every
open stream. It is nonstandard and broad in effect. Focus on explicit ownership
and close each stream deliberately.

## 8. Character Input and EOF

`fgetc()` returns `int`, not `char`:

~~~c
int byte = fgetc(stream);
~~~

That choice is essential. It must represent:

- every possible `unsigned char` value; and
- the separate negative sentinel `EOF`.

Storing the result directly in `char` can make a valid byte indistinguishable
from `EOF` on some platforms.

### ungetc()

`ungetc(byte, stream)` pushes one byte back so the next input operation sees it
again. At least one byte of pushback is guaranteed. Do not design portable code
that assumes unlimited pushback.

Run:

~~~bash
./build/chapters/03-buffered-io/exercises/04_character_io
~~~

Expected output:

~~~text
first=A pushed_back=A next=B output=AB
~~~

## 9. Line Input with fgets()

`fgets()` reads at most `size - 1` bytes and appends `\0` after successful
input. It stops at:

- a newline that fits in the destination;
- EOF; or
- the capacity limit.

If it reads a newline, it stores that newline.

This means one `fgets()` call is not guaranteed to return one whole logical
line. A long line arrives as multiple chunks.

Run:

~~~bash
./build/chapters/03-buffered-io/exercises/05_line_io
~~~

The important final observation is:

~~~text
summary: chunks=7 logical_lines=3
~~~

Three logical lines required seven bounded reads because the array could hold
only seven data bytes plus its terminating null byte.

## 10. Reading to an Arbitrary Delimiter

Sometimes newline is not the record separator. A configuration format might
use commas, colons, or another delimiter.

The program `06_delimited_input.c` builds a bounded helper using `fgetc()`.
The delimiter is consumed but not stored. The destination remains protected by
an explicit capacity check.

Run:

~~~bash
./build/chapters/03-buffered-io/exercises/06_delimited_input
~~~

Expected output:

~~~text
field 1=alpha
field 2=beta
field 3=gamma
~~~

Repeated `fgetc()` calls do not normally mean repeated `read()` system calls.
The C library usually serves those characters from its already-filled input
buffer.

## 11. Binary I/O with fread() and fwrite()

The important signatures are conceptually:

~~~c
size_t fread(void *buffer, size_t element_size,
             size_t element_count, FILE *stream);

size_t fwrite(const void *buffer, size_t element_size,
              size_t element_count, FILE *stream);
~~~

Both functions return an element count, not a byte count.

For example, if you request ten `uint32_t` values and `fread()` returns `7`, it
read seven complete elements—not seven bytes.

A short `fread()` can mean EOF or error. Use `feof()` and `ferror()` to decide.
A short `fwrite()` indicates failure.

Run:

~~~bash
./build/chapters/03-buffered-io/exercises/07_binary_io
~~~

Expected output:

~~~text
elements=4 first=0x01020304 last=0xffffffff
~~~

### Binary portability warning

Writing a C object directly does not automatically create a portable format.
Potential differences include:

- integer width;
- byte order;
- structure padding;
- alignment;
- floating-point representation;
- ABI choices.

Real portable formats define field sizes, byte order, encoding, and layout
explicitly.

## 12. Character, String, Formatted, and Binary Output

The C library offers output at several levels:

| Function | Logical input |
|---|---|
| `fputc()` | One byte |
| `fputs()` | A null-terminated string, excluding its `\0` |
| `fprintf()` | Values converted according to a format string |
| `fwrite()` | A specified number of fixed-size elements |

All may place data into the same kind of stream buffer. The difference is how
the application describes its data, not whether the kernel sees a system call
immediately.

Run:

~~~bash
./build/chapters/03-buffered-io/exercises/08_formatted_output
~~~

Expected output:

~~~text
formatted file:
compiler=clang
optimization=0
warnings=strict
~~~

## 13. A Robust Buffered Copy

Exercise 09 is the Chapter 3 counterpart to Chapter 2's descriptor-based copy.

The loop must still handle:

- returned counts;
- short output;
- EOF;
- stream errors;
- input close failure;
- output close and flush failure.

Run:

~~~bash
./build/chapters/03-buffered-io/exercises/09_buffered_copy \
  /etc/hosts \
  build/chapters/03-buffered-io/data/hosts.copy
~~~

Expected form:

~~~text
copied N bytes with standard I/O
~~~

Verify byte identity:

~~~bash
cmp /etc/hosts build/chapters/03-buffered-io/data/hosts.copy
echo $?
~~~

Expected:

~~~text
0
~~~

## 14. Seeking and Stream Position

Use stream positioning functions on a stream:

- `fseeko(stream, offset, origin)` changes position;
- `ftello(stream)` reports position;
- `rewind(stream)` returns to the beginning and clears indicators.

Prefer the `off_t`-based `fseeko()` and `ftello()` interfaces for Linux system
programming.

The origins have the familiar meanings:

- `SEEK_SET`: relative to beginning;
- `SEEK_CUR`: relative to current position;
- `SEEK_END`: relative to end.

Run:

~~~bash
./build/chapters/03-buffered-io/exercises/10_stream_seek
~~~

Expected output:

~~~text
end=10 after_seek=6 slice=678 final=01XY456789
~~~

The `XY` bytes replace bytes at offsets 2 and 3. Seeking does not insert bytes.

## 15. Flushing Is Not Durability

`fflush(stream)` sends pending stream output to the underlying descriptor.

Before the flush, those bytes can exist only inside the process. A separate
descriptor cannot read them because the kernel has not received them.

Run:

~~~bash
./build/chapters/03-buffered-io/exercises/11_flush_stream
~~~

Expected output:

~~~text
kernel-visible size: before fflush=0 after fflush=19
~~~

Then run the independent-reader experiment:

~~~bash
./build/chapters/03-buffered-io/experiments/buffer_visibility
~~~

Expected output:

~~~text
separate reader: before_flush=0 after_flush=7 data=hidden
~~~

Remember the boundary:

~~~text
fflush(): C-library buffer -> descriptor/kernel
fsync():  kernel state -> durability request
~~~

## 16. EOF and Error Are Stream State

`fgetc()` can return `EOF` for two different reasons:

1. The stream reached end-of-file.
2. The stream encountered an error.

Check:

- `feof(stream)` for end-of-file;
- `ferror(stream)` for error;
- `clearerr(stream)` to clear both indicators.

The EOF indicator becomes set only after an input operation attempts to read
past available input. Merely reading the last valid byte does not yet prove EOF.

Run:

~~~bash
./build/chapters/03-buffered-io/exercises/12_eof_error_state
~~~

Expected output:

~~~text
EOF state: feof=1 ferror=0
after clearerr: feof=0 ferror=0
error state: feof=0 ferror=1
~~~

The error case uses Linux `/dev/full`, a special device that rejects writes with
`ENOSPC`.

## 17. Controlling Buffering

`setvbuf()` supports three modes:

| Mode | Meaning | Typical use |
|---|---|---|
| `_IOFBF` | Full buffering | Regular files |
| `_IOLBF` | Line buffering | Interactive terminal output |
| `_IONBF` | No user-space buffering | Standard error or latency-sensitive output |

Rules:

1. Call `setvbuf()` after opening the stream but before any other operation.
2. If you supply the buffer, keep it alive until after `fclose()`.
3. Do not assume the default mode without considering whether the stream is
   connected to a terminal or redirected.
4. Use defaults unless there is a measured or semantic reason to change them.

Run:

~~~bash
./build/chapters/03-buffered-io/exercises/13_control_buffering
~~~

Expected output:

~~~text
full: before=0 after_flush=4
line: before_newline=0 after_newline=5
none: immediately=4
~~~

The pipe experiment makes line behavior especially visible:

~~~bash
./build/chapters/03-buffered-io/experiments/line_buffering_pipe
~~~

Expected output:

~~~text
before_newline=EAGAIN after_newline=8 data=partial
~~~

Before the newline, the pipe has no kernel-visible bytes. The newline triggers
the line-buffer flush.

## 18. Obtaining the Descriptor with fileno()

`fileno(stream)` is useful when an operation exists only at the descriptor or
system-call layer.

Examples include:

- `fsync(fileno(stream))` after first calling `fflush(stream)`;
- `fstat(fileno(stream), &info)`;
- descriptor flags or specialized Linux operations.

However, mixing I/O functions from both layers is dangerous because the stream
may have read-ahead bytes or pending output that the descriptor interface does
not know about.

Safe sequence for output when a descriptor-level action must follow:

~~~text
stream output
    -> fflush(stream)
    -> descriptor-level action
~~~

Run:

~~~bash
./build/chapters/03-buffered-io/experiments/mixing_io_layers
~~~

Typical Linux/glibc observation:

~~~text
without coordination (nonportable observation): rawABC
after fflush (defined ordering): ABCraw
~~~

The first line is deliberately nonportable. Do not build a program that relies
on that result. The second line shows explicit coordination.

## 19. Thread Safety and Stream Locking

Standard-I/O operations protect stream internals with a lock. One call such as
`fputs()` will not have its internal state corrupted by another thread using the
same stream.

But individual-call safety is not multi-call atomicity.

Suppose one logical record requires three calls:

~~~text
write opening bracket
write record body
write closing bracket
~~~

Another thread can run between those calls unless the program holds the stream
lock across the full sequence.

Use:

- `flockfile()` to wait for and acquire the stream lock;
- `ftrylockfile()` to attempt acquisition without blocking;
- `funlockfile()` to release one acquisition level.

The lock is recursive for the owning thread, so acquisition and release counts
must match.

Unlocked functions such as `fputs_unlocked()` do not acquire a lock. Use them
only when:

- the caller already holds the stream lock; or
- program design confines the stream to one thread.

Run the focused exercise:

~~~bash
./build/chapters/03-buffered-io/exercises/14_manual_stream_locking
~~~

Expected output:

~~~text
record=[one logical record]
~~~

Run the actual two-thread experiment:

~~~bash
./build/chapters/03-buffered-io/experiments/threaded_records
~~~

Expected output:

~~~text
records=80 malformed=0 locking=preserved
~~~

Chapter 7 will study threads and synchronization in greater depth. Here the
important lesson is that the stream itself is shared user-space state.

## 20. Measuring Syscall Amortization

The `syscall_amortization` experiment produces identical bytes in two ways:

- `raw`: one one-byte `write()` request per application byte;
- `buffered`: one `fputc()` per byte followed by buffered output.

First confirm both modes work:

~~~bash
./build/chapters/03-buffered-io/experiments/syscall_amortization raw 4096
./build/chapters/03-buffered-io/experiments/syscall_amortization buffered 4096
cmp \
  build/chapters/03-buffered-io/data/amortization_raw.bin \
  build/chapters/03-buffered-io/data/amortization_buffered.bin
~~~

Then count `write()` calls:

~~~bash
strace -c -e write \
  ./build/chapters/03-buffered-io/experiments/syscall_amortization raw 4096

strace -c -e write \
  ./build/chapters/03-buffered-io/experiments/syscall_amortization buffered 4096
~~~

Expected relationship:

~~~text
raw mode:       thousands of write() calls
buffered mode:  a small number of write() calls
~~~

Exact counts can include writes used to print the program's summary, so compare
the scale rather than memorizing one number.

## 21. Critiques and Tradeoffs

Standard I/O improves convenience and usually reduces system calls, but it is
not free.

Potential costs include:

- another copy between an application object and the stream buffer;
- hidden timing of kernel I/O;
- more user-space state to coordinate;
- problems when mixed with descriptor-level I/O;
- locking overhead in multithreaded programs;
- APIs whose historical behavior is sometimes awkward.

This does not mean standard I/O is bad. It means the abstraction has a cost
model and behavioral contract.

Use standard I/O when character, line, formatting, or buffered binary access
matches the program. Use descriptor APIs when precise low-level control is the
better fit.

## 22. Common Mistakes

### Mistake 1: Treating FILE * as a descriptor

A stream pointer is not an integer descriptor. Use `fileno()` only when the
underlying descriptor is truly needed.

### Mistake 2: Checking fopen() against -1

`fopen()` returns `NULL` on failure. `open()` returns `-1`.

### Mistake 3: Ignoring fclose()

Close can reveal a delayed output failure because it flushes pending data.

### Mistake 4: Double-closing after fdopen()

After `fdopen()` succeeds, `fclose()` owns the close operation.

### Mistake 5: Storing fgetc() in char

Use `int` so all byte values remain distinguishable from `EOF`.

### Mistake 6: Assuming fgets() returns a whole line

Check whether the chunk contains a newline. Long lines require assembly.

### Mistake 7: Using gets()

Never use `gets()`. It cannot enforce a destination bound and was removed from
the modern C standard.

### Mistake 8: Treating fread()/fwrite() returns as bytes

They return complete element counts.

### Mistake 9: Using while (!feof(stream))

EOF is discovered only after an input operation fails to obtain more data.
Drive the loop with the input function's return value, then inspect indicators.

### Mistake 10: Assuming fflush() makes data durable

It crosses the C-library boundary, not necessarily the storage boundary.

### Mistake 11: Calling setvbuf() after I/O begins

Configure buffering immediately after opening and before other stream work.

### Mistake 12: Letting a supplied buffer die too early

Keep a caller-supplied `setvbuf()` array alive until the stream is closed.

### Mistake 13: Mixing write() and fwrite() without coordination

The stream can have hidden buffered state. Flush and position deliberately, or
keep one I/O layer as the owner.

### Mistake 14: Assuming one locked call protects a whole record

Use `flockfile()` when several calls form one atomic transaction.

### Mistake 15: Using unlocked functions without an ownership strategy

Unlocked functions require manual locking or strict thread confinement.

## 23. Independent Exercises

Complete these in order. Each has an observable success condition.

### Exercise A — Redirection and default buffering

Add `isatty(fileno(stdout))` to `01_standard_streams.c`. Run normally and with
stdout redirected to a file.

Success condition: explain why the descriptor number remains `1` while the
destination and typical buffering policy can change.

### Exercise B — Mode-failure matrix

Extend `02_fopen_modes.c` to try reading from a `"w"` stream and writing to an
`"r"` stream. Capture `ferror()` and `errno` without corrupting another file.

Success condition: distinguish stream access permissions from filesystem
permissions.

### Exercise C — Descriptor ownership

Duplicate a descriptor with `dup()` before passing the original to `fdopen()`.
Close the stream and prove the duplicate remains valid.

Success condition: explain open-file descriptions, descriptor entries, and why
one close does not release the duplicated descriptor.

### Exercise D — Assemble long lines

Modify `05_line_io.c` to reconstruct each complete logical line in a dynamically
resized buffer.

Success condition: print exactly three reconstructed lines without truncation.

### Exercise E — Portable binary format

Replace the direct integer representation in `07_binary_io.c` with an explicit
big-endian encoding.

Success condition: inspect the file with `od -An -tx1` and explain every byte.

### Exercise F — Close-time failure

Write buffered output to `/dev/full` without disabling buffering. Determine
whether failure first appears at `fwrite()`, `fflush()`, or `fclose()`.

Success condition: record the actual boundary and explain why it occurs there.

### Exercise G — Trace a character loop

Trace `04_character_io`:

~~~bash
chapters/03-buffered-io/scripts/trace.sh 04_character_io
~~~

Success condition: show that several character operations do not require one
`read()` or `write()` system call each.

### Exercise H — Buffer-size comparison

Run the amortization experiment with 1, 16, 4,096, and 100,000 bytes under
`strace -c`.

Success condition: create a small table of logical operations versus actual
`write()` calls.

### Exercise I — Safe mixed-layer durability

Open a stream, write a record, call `fflush()`, obtain its descriptor with
`fileno()`, and call `fsync()`.

Success condition: explain separately what `fflush()` and `fsync()` guarantee.

### Exercise J — Try-lock behavior

Extend the threaded experiment with `ftrylockfile()`.

Success condition: observe a nonzero return while another thread owns the lock,
without treating that busy result as a stream error.

## 24. What Robert Love Is Building Toward

This chapter is not an isolated tour of `stdio.h`. It establishes several ideas
that later systems work depends on:

- abstractions hide state;
- performance depends on crossing boundaries efficiently;
- logical operations need not map one-to-one to system calls;
- return values and persistent indicators form an error protocol;
- ownership must be explicit when abstractions wrap lower-level resources;
- concurrency safety at one-operation granularity may not protect a transaction;
- convenience, copying, latency, throughput, and control trade against one
  another.

These ideas reappear in compiler scanners, diagnostic systems, object writers,
runtime logs, profilers, network libraries, database buffers, and virtual
machines.

## 25. Mastery Checklist

Before marking Chapter 3 complete, you should be able to explain without notes:

- why user-space buffering reduces system-call overhead;
- the difference between a descriptor and `FILE *`;
- where the stream buffer lives;
- why `fgetc()` returns `int`;
- why `fgets()` may return only part of a logical line;
- why `fread()` returns elements rather than bytes;
- how `fdopen()` changes descriptor ownership;
- what `fclose()` does beyond releasing memory;
- why `fflush()` is not `fsync()`;
- how EOF differs from an error;
- when `feof()` becomes true;
- the three buffering modes;
- the lifetime rule for a caller-supplied stream buffer;
- why mixing stdio and descriptor I/O is risky;
- what `flockfile()` protects;
- when unlocked operations are safe;
- why binary dumps of C structures are not portable;
- how to prove buffering behavior with `strace`.

You should also be able to:

- write and close a stream with complete error checking;
- copy a binary file with `fread()` and `fwrite()`;
- process a line longer than one fixed buffer;
- seek and query a stream position;
- deliberately flush at a semantic boundary;
- diagnose a short read using `feof()` and `ferror()`;
- measure syscall reduction rather than merely claiming it.

## Final Retention Statement

Chapter 2 taught direct descriptor I/O. Chapter 3 teaches that most convenient
C I/O is a managed user-space layer above those descriptors. That layer earns
its convenience and performance by keeping state. Correct systems programmers
know where that state lives, when it moves, who owns the underlying resource,
and which boundary each operation actually crosses.
