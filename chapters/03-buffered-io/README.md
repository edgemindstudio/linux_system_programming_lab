# Chapter 3 — Buffered I/O

**Status:** Lab prepared; study in progress

## Purpose

Understand how the C standard I/O library places a user-space `FILE` stream
between application code and descriptor-based Linux I/O, making character,
line, formatted, and binary operations convenient while reducing system-call
overhead.

## Book Scope

Robert Love, *Linux System Programming*, second edition, Chapter 3, printed
pages 67–90.

## Study Material

- `notes/study-guide-pages-67-90.md`
- `mental-models/buffered-io.md`

## Exercises

1. Standard streams and their descriptors
2. `fopen()` modes and update-stream direction changes
3. Bridging a descriptor into a stream with `fdopen()`
4. Character I/O and `ungetc()`
5. Line input with a bounded `fgets()` buffer
6. Bounded input using a caller-chosen delimiter
7. Binary records with `fread()` and `fwrite()`
8. String and formatted output
9. A robust buffered file copy
10. Stream seeking, position queries, and rewind
11. Explicit flushing and kernel-visible file size
12. Distinguishing EOF from stream error
13. Full, line, and unbuffered modes
14. Manual stream locking and unlocked operations

## Experiments

- preferred I/O block size versus `BUFSIZ`;
- visibility of data before and after a flush;
- newline-triggered flushing through a pipe;
- system-call amortization with and without buffering;
- consequences of mixing standard I/O with descriptor I/O;
- atomic multi-call records protected by `flockfile()`.

## Commands

~~~bash
make chapter CHAPTER=03
make test CHAPTER=03
make tidy-chapter CHAPTER=03
chapters/03-buffered-io/scripts/trace.sh 09_buffered_copy \
    /etc/hosts build/chapters/03-buffered-io/data/hosts.copy
~~~

Generated files are placed under:

~~~text
build/chapters/03-buffered-io/
~~~

## Completion Rule

The lab being prepared is not the same as mastering the chapter. Before moving
to Chapter 4, predict and observe when bytes live in the application buffer,
when the C library calls the kernel, how stream state changes, and why mixing
I/O layers requires explicit coordination.
