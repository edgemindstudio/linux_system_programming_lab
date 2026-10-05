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
| 3 | Buffered I/O | Lab prepared; study in progress |
| 4 | Advanced File I/O | Lab prepared; study in progress |
| 5 | Process Management | Lab prepared; study in progress |
| 6 | Advanced Process Management | Lab prepared; study in progress |
| 7 | Threading | Lab prepared; study in progress |
| 8 | File and Directory Management | Lab prepared; study in progress |
| 9 | Memory Management | Lab prepared; study in progress |
| 10 | Signals | Lab prepared; study in progress |
| 11 | Time | Lab prepared; study in progress |

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

**Status:** Lab prepared; study in progress

Focus:

- FILE streams;
- fopen, fdopen, fclose, fread, fwrite, fgets, and fprintf;
- buffering modes and block sizes;
- flushing, stream positions, EOF, and errors;
- thread safety and unlocked stream operations;
- the relationship between standard I/O and descriptor-based I/O.

The prepared laboratory contains fourteen focused exercises, six experiments,
a complete study guide, a mental model, a tracing helper, and deterministic
tests.

Completion requires predicting when data remains in user space, identifying
what causes a flush, distinguishing EOF from error, explaining stream and
descriptor ownership, and measuring how buffering changes system-call traffic.

## Chapter 4 — Advanced File I/O

**Status:** Lab prepared; study in progress

Focus:

- scatter/gather I/O;
- epoll and readiness models;
- mmap, munmap, protection, and synchronization;
- I/O advice and readahead;
- synchronous and asynchronous operations;
- I/O scheduling and performance.

The prepared laboratory contains sixteen focused exercises, seven experiments,
a complete study guide, a mental model, a tracing helper, and deterministic
tests.

Completion requires explaining partial vectored transfers, contrasting epoll
trigger modes, reasoning about mapping lifetimes and page faults,
distinguishing cache visibility from durability, treating advice as a hint,
managing asynchronous request lifetimes, and evaluating I/O performance in the
context of the current storage stack.

## Chapter 5 — Process Management

**Status:** Lab prepared; study in progress

Focus:

- programs, processes, threads, process IDs, and hierarchy;
- exec, fork, copy-on-write, and termination;
- waiting, exit status, and zombie processes;
- users, groups, sessions, process groups, and daemons.

The prepared laboratory contains nineteen focused exercises, seven experiments,
a complete study guide, a mental model, a tracing helper, and deterministic
tests.

Completion requires explaining the fork/exec split, copy-on-write, descriptor
inheritance, termination cleanup, wait-status decoding, zombie reaping,
credentials, sessions, process groups, and the difference between traditional
daemonization and modern service supervision.

## Chapter 6 — Advanced Process Management

**Status:** Lab prepared; study in progress

Focus:

- process scheduling and priorities;
- CPU-bound and I/O-bound behavior;
- processor affinity and I/O priorities;
- real-time scheduling, latency, jitter, and determinism;
- memory locking and resource limits.

The prepared laboratory contains eighteen focused exercises, seven
experiments, a complete study guide, a mental model, a tracing helper, and
deterministic tests.

Completion requires explaining runnable and blocked state, preemption,
CPU-bound and I/O-bound behavior, fair scheduling, nice values, affinity,
real-time policy safety, latency and jitter, memory preparation, and soft and
hard resource limits. It also requires distinguishing book-era scheduler
internals from stable user-space APIs and current kernel evidence.

## Chapter 7 — Threading

**Status:** Lab prepared; study in progress

Focus:

- concurrency, parallelism, and race conditions;
- threading models and common patterns;
- Pthreads creation, identity, termination, join, and detach;
- mutexes, synchronization, and deadlocks.

The prepared laboratory contains eighteen focused exercises, seven
experiments, a complete study guide, a mental model, a tracing helper, and
deterministic tests.

Completion requires separating process resources from per-thread execution
state, distinguishing concurrency from parallelism, choosing a threading
pattern deliberately, managing argument and result lifetimes, joining or
detaching every thread, handling deferred cancellation safely, associating
mutexes with protected data, preserving shared invariants, and preventing
deadlocks through a consistent lock hierarchy.

## Chapter 8 — File and Directory Management

**Status:** Lab prepared; study in progress

Focus:

- file metadata, permissions, and ownership;
- extended attributes;
- directories and directory streams;
- hard links, symbolic links, unlinking, copying, and moving;
- device nodes and random data;
- inotify and filesystem-event monitoring.

The prepared laboratory contains eighteen focused exercises, seven
experiments, a complete study guide, a mental model, a tracing helper, and
deterministic tests.

Completion requires separating pathnames, directory entries, inodes, and open
descriptors; interpreting metadata and permission bits; explaining ownership,
umask, and extended attributes; traversing directory streams without assuming
order or `d_type`; distinguishing hard links from symbolic links; reasoning
about unlink, rename, and copy lifetimes; connecting device nodes to drivers;
and managing the full inotify instance, watch, event, overflow, and cleanup
lifecycle.

## Chapter 9 — Memory Management

**Status:** Lab prepared; study in progress

Focus:

- process address spaces, pages, and memory regions;
- dynamic allocation, resizing, freeing, and alignment;
- data-segment and anonymous-mapping mechanisms;
- stack allocations and memory manipulation;
- memory locking, residency, overcommit, and OOM behavior.

The prepared laboratory contains nineteen focused exercises, seven
experiments, a complete study guide, a mental model, a tracing helper, and
deterministic tests.

Completion requires explaining virtual address spaces, regions, pages, demand
paging, allocator ownership, checked sizing, alignment, safe resizing,
program-break and mapping mechanisms, bounded stack allocation, raw byte
operations, residency, memory locking, resource limits, overcommit, and the
difference between allocation success and guaranteed future backing.

## Chapter 10 — Signals

**Status:** Lab prepared; study in progress

Focus:

- signal identifiers, delivery, disposition, and inheritance;
- sending, blocking, pending, and waiting;
- reentrancy and async-signal-safe behavior;
- signal sets, siginfo, and payloads.

The prepared laboratory contains eighteen focused exercises, seven
experiments, a complete study guide, a mental model, a tracing helper, and
deterministic tests.

Completion requires explaining generation, pending state, delivery,
dispositions, default actions, fork and exec inheritance, safe signal targets,
handler reentrancy, async-signal-safe operations, signal sets and masks,
standard-signal coalescing, real-time queueing, race-free waiting, `EINTR`,
`SA_RESTART`, `siginfo_t`, payloads, self-pipe integration, and Linux
`signalfd()` behavior.

## Chapter 11 — Time

**Status:** Lab prepared; study in progress

Focus:

- wall, monotonic, boot, process, and thread clocks;
- `time_t`, `timeval`, `timespec`, and broken-down civil time;
- clock resolution, normalized arithmetic, and safe conversion;
- UTC, local time, timezone policy, `mktime()`, and `strftime()`;
- relative sleeping, interruption, remaining time, and absolute deadlines;
- alarm, interval timers, POSIX timers, overruns, and Linux `timerfd`;
- vDSO clock reads, procfs uptime, sysfs clock-source evidence, and timing
  uncertainty.

The prepared laboratory contains eighteen focused exercises, seven
experiments, a complete study guide, a mental model, a tracing helper, and
deterministic tests.

Completion requires choosing clocks by semantics; separating timestamps,
elapsed durations, and CPU consumption; maintaining normalized time values;
converting civil time with explicit timezone policy; handling `EINTR` and
wake-up latency; using absolute deadlines to control periodic drift; managing
timer lifecycles and overruns; and explaining how timerfd integrates time with
descriptor-based event loops.

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

Continue the prepared chapters in order, recording predictions and Linux
evidence rather than treating successful builds as completion. For Chapter 11,
begin by separating civil, monotonic, boot, process, and thread time; then move
through representation, conversion, sleeping, absolute deadlines, timers,
overruns, and event-loop integration. Never change the host clock merely to
demonstrate that realtime is adjustable.
