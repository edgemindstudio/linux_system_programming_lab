# Chapter 3 Mental Model — Buffered I/O

## The Additional Layer

Chapter 2 used a file descriptor directly:

~~~text
application buffer
      |
      | read() / write()
      v
Linux kernel
      |
      v
page cache, filesystem, device
~~~

Chapter 3 inserts a C-library stream and its user-space buffer:

~~~text
application value or array
      |
      | fgetc(), fgets(), fprintf(), fread(), fwrite(), ...
      v
FILE stream state
  - user-space buffer
  - buffer position
  - EOF indicator
  - error indicator
  - orientation and locking state
      |
      | read(), write(), lseek(), close(), ...
      v
file descriptor
      |
      v
Linux kernel and page cache
~~~

The stream does not replace the descriptor. It manages the descriptor and adds
policy and state around it.

## Why Buffer in User Space?

Suppose an application emits 4,096 characters one at a time.

Without user-space buffering:

~~~text
4,096 application operations
        -> about 4,096 write() system calls
~~~

With a 4,096-byte standard-I/O buffer:

~~~text
4,096 fputc() operations
        -> fill one user-space buffer
        -> about one write() system call when flushed
~~~

The number of application-level operations is unchanged. The number of
user-to-kernel transitions is reduced.

## Three Buffering Modes

| Mode | C constant | Typical behavior |
|---|---|---|
| Full buffering | `_IOFBF` | Transfer when the buffer fills or is flushed |
| Line buffering | `_IOLBF` | Also flush output when a newline completes a line |
| No buffering | `_IONBF` | Send each library operation toward the descriptor immediately |

Regular-file streams are normally fully buffered. Terminal output is commonly
line buffered. Standard error is commonly unbuffered. These are library
policies, not properties of the underlying descriptor.

## Stream Ownership

`fopen()` creates both a stream and its underlying descriptor. `fdopen()` wraps
an existing compatible descriptor in a stream. In both cases, a successful
`fclose()` flushes pending output and closes the underlying descriptor.

After giving a descriptor to `fdopen()`, treat the stream as its owner. Do not
close the descriptor separately.

## One Logical Position, Additional Buffered State

A seekable stream ultimately operates on the descriptor's file offset, but the
C library may already have read ahead or may still hold unwritten bytes.
Therefore:

- use `fseeko()` and `ftello()` with the stream;
- flush or perform the required positioning operation when changing direction
  on an update stream;
- do not mix `read()`/`write()` with `fread()`/`fwrite()` casually;
- use `fflush()` when descriptor-level code must observe prior stream output.

## Return-Value Discipline

| Operation | Success information | Failure or completion |
|---|---|---|
| `fgetc()` | next byte as `unsigned char` converted to `int` | `EOF`; inspect `feof()` and `ferror()` |
| `fgets()` | destination pointer | `NULL`; inspect stream state |
| `fread()` | number of complete elements read | short count; inspect `feof()` and `ferror()` |
| `fwrite()` | number of complete elements written | short count indicates failure |
| `fputs()` / `fprintf()` | nonnegative result | `EOF` or negative result |
| `fflush()` / `fclose()` | zero | `EOF`, with `errno` describing the failure |
| `fseeko()` | zero | `-1` |
| `ftello()` | current position | `(off_t) -1` |

EOF is a state discovered after a read attempts to move beyond available input.
It is not a byte stored in the file.

## Thread Safety

Standard-I/O functions lock streams internally for individual operations. That
does not make a sequence of several operations atomic. Use `flockfile()` and
`funlockfile()` when several calls must form one indivisible stream transaction.
Inside an explicitly held stream lock, unlocked operations can avoid redundant
lock/unlock work.

## Connection to Compiler and Runtime Engineering

Compilers and runtimes use buffering when they:

- scan source code character by character while reading storage in blocks;
- accumulate diagnostics before writing to a terminal or log;
- serialize object metadata and profiles;
- manage text and binary formats through a shared stream abstraction;
- coordinate output from multiple worker threads;
- trade copying and syscall overhead against latency and durability.

The essential lesson is that convenience has state. A `FILE *` is not merely a
friendlier file descriptor; it is a stateful user-space I/O engine.
