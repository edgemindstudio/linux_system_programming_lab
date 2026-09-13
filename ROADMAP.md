# Linux System Programming Laboratory Roadmap

This roadmap follows the eleven chapters of Robert Love's *Linux System
Programming*, second edition. Progress is measured by demonstrated
understanding, implementation, observation, and explanation—not by reading
alone.

## Progress States

- Not started
- Reading
- Implementing
- Experimenting
- Reviewing
- Complete
- Revisit later

## Book Progress

| Chapter | Title | Status |
|---:|---|---|
| 1 | Introduction and Essential Concepts | In progress |
| 2 | File I/O | Lab prepared; study in progress |
| 3 | Buffered I/O | Not started |
| 4 | Advanced File I/O | Not started |
| 5 | Process Management | Not started |
| 6 | Advanced Process Management | Not started |
| 7 | Threading | Not started |
| 8 | File and Directory Management | Not started |
| 9 | Memory Management | Not started |
| 10 | Signals | Not started |
| 11 | Time | Not started |

## Repository Foundation

**Status:** Complete

The repository has:

- a documented learning method;
- strict C17 compilation warnings;
- an introductory smoke test;
- reproducible build commands;
- chapter-selectable builds and tests;
- an ignored, disposable build tree;
- a structure validator;
- Git history and a remote repository.

## Chapter 1 — Introduction and Essential Concepts

**Status:** In progress

Focus:

- system programming;
- user space and kernel space;
- system calls and the C library;
- APIs and ABIs;
- standards and portability;
- files, processes, threads, users, signals, and IPC;
- headers, return values, errno, and manual pages.

Completion requires explaining the user-to-kernel path, distinguishing library
calls from system calls, interpreting failures correctly, and connecting APIs
and ABIs to compiler/runtime work.

## Chapter 2 — File I/O

**Status:** Lab prepared; study in progress

Focus:

- file descriptors and standard descriptors;
- open, creat, read, write, close, and lseek;
- access modes, creation flags, permissions, and umask;
- short reads, partial writes, EINTR, and nonblocking I/O;
- append behavior and synchronized I/O;
- direct I/O;
- positional I/O and truncation;
- select, pselect, poll, the VFS, page cache, and writeback.

The prepared laboratory contains twelve focused exercises, seven experiments,
a complete study guide, a mental model, a tracing helper, and deterministic
tests.

Completion requires running and explaining the programs rather than merely
building them.

## Chapter 3 — Buffered I/O

**Status:** Not started

Focus:

- FILE streams;
- fopen, fdopen, fclose, fread, fwrite, fgets, and fprintf;
- buffering modes and block sizes;
- flushing, stream positions, EOF, and errors;
- thread safety and unlocked stream operations;
- the relationship between standard I/O and descriptor-based I/O.

## Chapter 4 — Advanced File I/O

**Status:** Not started

Focus:

- scatter/gather I/O;
- epoll and readiness models;
- mmap, munmap, protection, and synchronization;
- I/O advice and readahead;
- synchronous and asynchronous operations;
- I/O scheduling and performance.

## Chapter 5 — Process Management

**Status:** Not started

Focus:

- programs, processes, threads, process IDs, and hierarchy;
- exec, fork, copy-on-write, and termination;
- waiting, exit status, and zombie processes;
- users, groups, sessions, process groups, and daemons.

## Chapter 6 — Advanced Process Management

**Status:** Not started

Focus:

- process scheduling and priorities;
- CPU-bound and I/O-bound behavior;
- processor affinity and I/O priorities;
- real-time scheduling, latency, jitter, and determinism;
- memory locking and resource limits.

## Chapter 7 — Threading

**Status:** Not started

Focus:

- concurrency, parallelism, and race conditions;
- threading models and common patterns;
- Pthreads creation, identity, termination, join, and detach;
- mutexes, synchronization, and deadlocks.

## Chapter 8 — File and Directory Management

**Status:** Not started

Focus:

- file metadata, permissions, and ownership;
- extended attributes;
- directories and directory streams;
- hard links, symbolic links, unlinking, copying, and moving;
- device nodes and random data;
- inotify and filesystem-event monitoring.

## Chapter 9 — Memory Management

**Status:** Not started

Focus:

- process address spaces, pages, and memory regions;
- dynamic allocation, resizing, freeing, and alignment;
- data-segment and anonymous-mapping mechanisms;
- stack allocations and memory manipulation;
- memory locking, residency, overcommit, and OOM behavior.

## Chapter 10 — Signals

**Status:** Not started

Focus:

- signal identifiers, delivery, disposition, and inheritance;
- sending, blocking, pending, and waiting;
- reentrancy and async-signal-safe behavior;
- signal sets, siginfo, and payloads.

## Chapter 11 — Time

**Status:** Not started

Focus:

- time representations and POSIX clocks;
- resolution and clock selection;
- wall-clock and process time;
- setting and adjusting clocks;
- sleeping, waiting, overruns, alarms, and timers.

## Cross-Chapter Projects

**Status:** Not started

Projects live in projects/ because they combine mechanisms from multiple
chapters. Candidate projects include:

- a robust copy and inspection utility;
- a minimal command shell;
- a process supervisor;
- a memory-mapped binary inspector;
- a concurrent log-processing tool;
- a compiler-driver prototype.

Projects are not a substitute for Chapter 11. They begin only when their
supporting chapters have been studied.

## Standard Chapter Workflow

Every important topic follows this sequence:

1. Identify the problem.
2. Build the mental model.
3. Read the relevant material.
4. Explain the concept in my own words.
5. Predict Linux behavior.
6. Write pseudocode.
7. Implement a first version.
8. Compile with strict warnings.
9. Run and inspect the behavior.
10. Record evidence.
11. Explain discrepancies.
12. Add tests where appropriate.
13. Connect the mechanism to compiler/runtime engineering.
14. Review the topic.
15. Commit the completed learning unit.

## Current Next Step

Study Chapter 2 in order, beginning with standard file descriptors and
descriptor allocation. Keep the prepared programs unchanged until each concept
has been explained, predicted, run, and observed.
