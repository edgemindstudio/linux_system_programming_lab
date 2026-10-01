# Chapter 7 Mental Model - Threading

## One Process, Multiple Schedulable Threads

~~~text
Process
├── shared virtual address space
│   ├── program text
│   ├── globals and static objects
│   ├── heap
│   └── mapped files and libraries
├── shared open-file table and signal dispositions
│
├── initial thread
│   ├── private stack
│   ├── register state
│   └── scheduling state
├── worker thread A
│   ├── private stack
│   ├── register state
│   └── scheduling state
└── worker thread B
    ├── private stack
    ├── register state
    └── scheduling state
~~~

The process owns resources. A thread is an execution path through those
resources. Sharing makes communication cheap, but also makes unsynchronized
access dangerous.

## Concurrency Is Not Automatically Parallelism

~~~text
Concurrency: work overlaps in time
Parallelism: work executes at the same instant on different CPUs
~~~

A single CPU can interleave several threads concurrently. Multiple CPUs are
required for true CPU parallelism. Either form can expose ordering bugs.

## Linux and Pthreads Use Different Identities

| Identity | Meaning | Correct interface |
|---|---|---|
| PID | Shared process identity | `getpid()` |
| kernel TID | Linux task identity | `gettid()` or `SYS_gettid` |
| `pthread_t` | Opaque Pthreads identity | `pthread_self()`, `pthread_equal()` |

Portable application code should normally use `pthread_t`. Kernel TIDs are
useful when relating a thread to Linux-specific tools such as `/proc` or
debuggers.

## Lifecycle

~~~text
pthread_create()
       |
       v
start routine runs concurrently
       |
       +--> return pointer / pthread_exit(pointer)
       |
       +--> deferred cancellation
       v
terminated but joinable ------------> pthread_join() reclaims resources
       |
       +-----------------------------> pthread_detach() enables auto-reclaim
~~~

Every joinable thread should be joined exactly once. A thread that will never
be joined should be detached. Detachment is a resource-lifetime choice, not a
completion notification.

## Race and Critical Region

~~~text
read shared value
modify private copy
write shared value
~~~

Another thread may interleave anywhere between those steps. Even `counter++`
is a compound operation. The data that must remain consistent determines the
critical region.

## Lock Data, Not Code

~~~text
struct account {
    pthread_mutex_t mutex;  protects -> balance
    int balance;
};
~~~

The useful rule is not “this function owns the lock.” It is “access to this
data requires this lock.” That rule continues to make sense as the codebase
grows and new functions access the same object.

## Mutex Contract

~~~text
lock
  inspect and update protected invariant
unlock
~~~

- Keep critical regions as small as correctness permits.
- Never access protected state without its mutex.
- Release every acquired mutex on every exit path.
- Do not hold locks across slow work unless the invariant requires it.
- Check acquisition order whenever more than one lock is needed.

## ABBA Deadlock

~~~text
Thread 1: lock A -> waits for B
Thread 2: lock B -> waits for A
~~~

Both threads wait forever because each holds what the other needs. A global
ordering rule removes the cycle:

~~~text
All threads: lock lower ID -> lock higher ID
~~~

Timeouts can make a failure bounded and observable, but ordering prevents the
failure structurally.

## Condition Variables

A mutex protects state. A condition variable lets a thread sleep until the
state might have changed.

~~~c
pthread_mutex_lock(&mutex);
while (!predicate) {
    pthread_cond_wait(&condition, &mutex);
}
/* predicate is true while mutex is held */
pthread_mutex_unlock(&mutex);
~~~

The loop is essential because wakeups may be spurious and because another
thread may consume the state before the awakened thread reacquires the mutex.

## Design Before Code

Before creating threads, answer:

1. What work is concurrent?
2. What data is shared?
3. Which lock protects each shared invariant?
4. In what global order are multiple locks acquired?
5. Who owns arguments and returned objects?
6. Who joins or detaches each thread?
7. Where may cancellation occur, and what cleanup is required?
8. Is a thread-per-task, bounded pool, event loop, or process model clearer?

Threads amplify architecture. A clean ownership and synchronization design
becomes scalable; an unclear design becomes timing-dependent.
