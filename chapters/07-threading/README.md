# Chapter 7 - Threading

**Status:** Lab prepared; study in progress

## Purpose

Understand how a Linux process can contain multiple independently schedulable
threads that share one address space. The chapter moves from threading models
and design patterns through races, synchronization, deadlocks, and the core
POSIX threads lifecycle.

## Book Scope

Robert Love, *Linux System Programming*, second edition, Chapter 7, printed
pages 211-239.

## Study Material

- `notes/study-guide-pages-211-239.md`
- `mental-models/threading.md`

## Exercises

1. Compare process, kernel-task, and Pthread identities
2. Create and join one thread
3. Give multiple workers stable argument storage
4. Observe the shared process address space
5. Distinguish shared memory from per-thread stacks
6. Transfer ownership of a worker result through `pthread_join()`
7. Join several peer threads
8. Coordinate completion of a detached thread
9. Let the initial thread call `pthread_exit()`
10. Compare opaque `pthread_t` values correctly
11. Request and observe deferred cancellation
12. Run cleanup handlers during cancellation
13. Protect a shared counter with a mutex
14. Observe `EBUSY` with `pthread_mutex_trylock()`
15. Preserve a bank-account invariant
16. Prevent ABBA deadlock with a global lock order
17. Detect recursive locking with an error-checking mutex
18. Hand work off with a condition variable and predicate loop

## Experiments

- demonstrate a logical lost-update race without invoking undefined behavior;
- count environment-dependent mutex contention;
- compare private stack addresses with a shared heap address;
- inspect live Linux threads through `/proc/self/task`;
- model thread-per-task work;
- reuse a bounded worker pool;
- bound a lock wait with `pthread_mutex_timedlock()`.

## Commands

~~~bash
make chapter CHAPTER=07
make test CHAPTER=07
make tidy-chapter CHAPTER=07
chapters/07-threading/scripts/trace.sh 01_thread_identity
chapters/07-threading/scripts/trace.sh mutex_contention
~~~

Generated files are placed under:

~~~text
build/chapters/07-threading/
~~~

## Safety Choices

The lab never leaves a program intentionally deadlocked. Deadlock behavior is
demonstrated with consistent lock ordering, error-checking mutexes, and bounded
timed waits. The race experiment uses C11 atomics so its lost update is a
defined logical race rather than a C data race with undefined behavior. Every
joinable thread is joined; detached completion has its own condition-variable
protocol; cancellation cleanup releases acquired memory.

## Completion Rule

Building the lab is not the same as mastering it. Before moving to Chapter 8,
explain processes versus threads, concurrency versus parallelism, 1:1 versus
N:1 versus N:M threading, thread-per-connection versus event-driven designs,
argument and result lifetimes, join versus detach, cancellation, critical
regions, mutex ownership, lock granularity, and deadlock prevention.
