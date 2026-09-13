# Chapter 2 — File I/O

**Status:** Lab prepared; study in progress

## Purpose

Understand descriptor-based Linux I/O from opening a file through reading,
writing, positioning, synchronization, closing, and readiness notification.

## Study Material

- notes/study-guide-pages-31-61.md
- mental-models/file-io.md

## Exercises

1. Standard file descriptors
2. Opening a file
3. Access modes
4. File creation
5. creat versus open
6. Reading a file
7. Writing a file
8. Robust binary-safe copying
9. Seeking and offsets
10. Positional I/O
11. Truncating and extending
12. Waiting with select

## Experiments

- direct I/O and alignment;
- durable writes;
- descriptor allocation and reuse;
- nonblocking pipes;
- open flags;
- permissions and umask;
- sparse files.

## Commands

~~~bash
make chapter CHAPTER=02
make test CHAPTER=02
make tidy-chapter CHAPTER=02
chapters/02-file-io/scripts/trace.sh 06_read_file /etc/hosts
~~~

Generated files are placed under:

~~~text
build/chapters/02-file-io/
~~~

## Completion Rule

The lab being prepared is not the same as the chapter being complete. Each
exercise must be predicted, run, observed, and explained.
