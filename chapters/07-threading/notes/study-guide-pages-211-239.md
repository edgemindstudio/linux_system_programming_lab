# Chapter 7 Study Guide - Threading

Book: Robert Love, *Linux System Programming*, second edition
Scope: Chapter 7, printed pages 211-239
Repository: `linux_system_programming_lab`

This guide is an original learning companion. It explains the chapter's ideas
through programs in this repository and does not reproduce the book.

## 1. Chapter Goal

Chapter 7 asks you to separate two ideas that a single-threaded program lets
you treat as one:

1. the process as a container of resources; and
2. the thread as a schedulable path of execution.

A process owns an address space, open files, credentials, and other resources.
A thread owns an execution context: registers, a stack, scheduling state, and
a current instruction. One process can contain several threads, so several
execution paths can operate on the same memory.

That shared memory is both the attraction and the danger. Communication can be
as simple as dereferencing a pointer, but a pointer does not provide ordering,
mutual exclusion, or lifetime safety.

The chapter's central lesson is therefore:

> Creating a thread is easy. Designing ownership, synchronization, and
> termination correctly is the real work.

## 2. Chapter Summary

Robert Love highlights the following progression:

- Binaries are dormant programs, processes are running resource containers,
  and threads are schedulable execution units inside processes.
- Multiple threads can improve abstraction, responsiveness, blocking-I/O
  handling, memory sharing, and CPU parallelism.
- Threads also increase design and debugging complexity because execution
  order becomes nondeterministic while memory remains shared.
- Linux uses a 1:1 model: each user-visible Pthread corresponds to a kernel
  schedulable task.
- Thread-per-connection and event-driven systems are two major ways to
  organize concurrent work.
- Concurrency means overlapping progress; parallelism means simultaneous
  execution on multiple processing units.
- A race occurs when correctness depends on uncontrolled ordering.
- A mutex can make a critical region mutually exclusive, but poor lock design
  can introduce deadlock and excessive contention.
- Pthreads provides creation, identity, termination, cancellation, joining,
  detaching, and synchronization interfaces.
- Every thread needs an explicit lifecycle and resource-ownership plan.

## 3. Binary, Process, and Thread

### 3.1 Binary

A binary is a file containing instructions and data in a format the operating
system can load. It is not executing.

### 3.2 Process

A process is a running program plus its kernel-managed resources. Important
process-level resources include:

- virtual address space;
- open file descriptors;
- credentials;
- signal dispositions;
- current working directory;
- resource limits.

### 3.3 Thread

A thread is one independently schedulable execution path within the process.
Each thread has its own:

- register values;
- stack;
- instruction position;
- kernel task identity;
- scheduling state;
- signal mask and some thread-specific state.

Threads in one process normally share:

- program text;
- global and static objects;
- heap allocations;
- memory mappings;
- file descriptors;
- process credentials.

### 3.4 The key distinction

~~~text
Process = resource container
Thread  = execution context
~~~

When `fork()` creates a process, Linux presents a logically separate address
space using copy-on-write. When `pthread_create()` creates a thread, the new
thread begins in the same address space immediately.

## 4. Why Use Multiple Threads?

### 4.1 Natural work decomposition

Some problems contain independent work units. A server can handle requests, a
runtime can perform background collection, and a compiler can process
independent translation units.

Threads let the program express those units as separate execution paths.

### 4.2 CPU parallelism

If the machine has multiple logical CPUs, runnable threads can execute at the
same instant. This can increase throughput when the work is genuinely
parallelizable and synchronization overhead is controlled.

More threads do not automatically mean more speed. Important limits include:

- number of available CPUs;
- serial parts of the algorithm;
- lock contention;
- cache traffic;
- memory bandwidth;
- scheduling and creation overhead.

### 4.3 Responsiveness

A long computation in one thread need not stop another thread from processing
user input or supervisory work.

### 4.4 Blocking I/O

One thread can block on I/O while other threads continue. Alternatives include
nonblocking I/O, readiness multiplexing, asynchronous I/O, and event loops.

### 4.5 Shared-memory efficiency

Threads communicate through the process address space. They do not need a
separate IPC transport merely to access common data.

## 5. Costs and Reasons Not to Use Threads

Threading adds costs that are easy to underestimate:

- per-thread stack and kernel bookkeeping;
- creation, termination, and context-switch costs;
- synchronization operations;
- contention and cache-line movement;
- nondeterministic scheduling;
- races, deadlocks, starvation, and priority inversion;
- harder testing and debugging;
- more complicated shutdown and error recovery.

A single-threaded event loop or multiple isolated processes may be easier and
safer for a particular design. Threading is a tool, not a default requirement.

## 6. Threading Models

### 6.1 1:1 kernel-level threading

~~~text
one user thread <-> one kernel schedulable task
~~~

Linux Pthreads uses this model. The kernel can schedule threads independently
across CPUs. Blocking one kernel thread does not automatically block all other
threads in the process.

### 6.2 N:1 user-level threading

~~~text
many user threads -> one kernel task
~~~

A user-space scheduler switches among user threads. Switching can be cheap,
but one blocking kernel operation can block the entire kernel task, and a
single kernel task cannot execute on several CPUs simultaneously.

### 6.3 N:M hybrid threading

~~~text
many user threads -> a smaller or equal set of kernel tasks
~~~

This model attempts to combine user-space scheduling with kernel parallelism.
It also introduces a difficult mapping and coordination problem.

### 6.4 Coroutines and fibers

Coroutines and fibers provide cooperative user-space execution. They are often
useful for structured concurrency or asynchronous work but do not inherently
provide kernel-level parallelism.

## 7. Threading Patterns

### 7.1 Thread per connection or task

One thread owns one request until completion.

Advantages:

- straightforward sequential control flow;
- blocking I/O is easy to express;
- request-local state can stay on one stack.

Costs:

- thread count grows with concurrent work;
- stacks and kernel tasks consume resources;
- very high concurrency can create scheduling overhead.

The `thread_per_task` experiment models this approach.

### 7.2 Bounded worker pool

A fixed number of worker threads repeatedly claim tasks from a queue.

Advantages:

- bounded resource consumption;
- thread creation cost is amortized;
- concurrency can be tuned to CPUs or blocking behavior.

Costs:

- queue and shutdown synchronization are required;
- long tasks can delay short tasks;
- worker starvation and load imbalance remain possible.

The `bounded_worker_pool` experiment demonstrates worker reuse.

### 7.3 Event-driven design

An event loop waits for readiness and dispatches callbacks or tasks. CPU work
may be handed to a small worker pool.

Advantages:

- many waiting connections do not require one thread each;
- thread count can remain close to the amount of executable work.

Costs:

- application control flow becomes stateful;
- callback and lifetime management can be difficult;
- blocking work must not stall the event loop.

Chapter 2's readiness interfaces and Chapter 4's `epoll` exercises connect
directly to this model.

## 8. Concurrency Versus Parallelism

Concurrency means two activities have overlapping lifetimes and can make
progress in an interleaved order.

Parallelism means two activities execute simultaneously.

~~~text
One CPU:
Thread A  run ---- wait ---- run
Thread B  ---- run ---- run -----

Two CPUs:
CPU 0     Thread A ------------->
CPU 1     Thread B ------------->
~~~

The first system is concurrent without parallel execution. Both systems can
expose a race because both permit operations to be ordered differently.

## 9. Race Conditions

A race condition exists when correct behavior depends on uncontrolled timing
or ordering among concurrent activities.

### 9.1 `counter++` is not one indivisible operation

Conceptually, it contains:

~~~text
load counter
add one
store counter
~~~

Two threads can both load the same old value, independently add one, and both
store the same new value. One increment is lost.

### 9.2 Data race versus logical race

In the C memory model, conflicting unsynchronized accesses to a non-atomic
object from different threads form a data race and produce undefined behavior.
The program is not merely “occasionally wrong”; the language no longer defines
its behavior.

A logical race can occur even when individual operations are atomic. For
example:

~~~c
old = atomic_load(&counter);
atomic_store(&counter, old + 1);
~~~

Each access is atomic, so there is no C data race. The compound transaction can
still lose an update. The `logical_race_atomic` experiment demonstrates this
safely and compares it with `atomic_fetch_add()`.

### 9.3 Critical region

A critical region is the set of operations that must not interleave with a
conflicting operation. Define it from the invariant, not from the number of
source lines.

For a bank withdrawal, the indivisible transaction is:

1. read the balance;
2. decide whether funds are sufficient;
3. subtract the amount.

Locking only the final assignment would not protect the invariant.

## 10. Synchronization and Atomicity

Synchronization provides ordering and coordination between threads.

Common mechanisms include:

- mutexes;
- condition variables;
- read/write locks;
- semaphores;
- barriers;
- atomic operations;
- message queues.

The correct mechanism depends on the required invariant and communication
pattern.

An operation is atomic when other observers cannot see it partially complete.
A mutex can make a multi-instruction critical region appear atomic relative to
other code that follows the same locking contract.

## 11. Mutexes

A mutex allows one owning thread at a time.

~~~c
pthread_mutex_lock(&object->mutex);
/* inspect or update protected state */
pthread_mutex_unlock(&object->mutex);
~~~

### 11.1 Lock data, not code

Associate a mutex with the data and invariant it protects:

~~~c
struct account {
    pthread_mutex_t mutex; /* protects balance */
    int balance;
};
~~~

Every code path that reads or writes `balance` under concurrent access follows
the same rule. The rule survives refactoring better than saying that one
particular function is “locked.”

### 11.2 Granularity

A single global lock is simple but can serialize unrelated work. A lock per
account permits operations on different accounts to proceed concurrently.

Finer-grained locking may improve scalability, but it increases:

- the number of ownership rules;
- the likelihood of acquiring several locks;
- the risk of deadlock;
- review and debugging complexity.

Correctness comes first. Measure contention before adding lock complexity.

### 11.3 Fast and slow paths

An uncontended Pthread mutex can often be acquired in user space. Contention
may require a Linux `futex` operation so the kernel can sleep and wake tasks.
Consequently, `strace` may show no syscall for an uncontended lock but show
`futex()` when threads actually wait.

## 12. Deadlock

A deadlock is a cycle of waiting in which no participant can make progress.

### 12.1 ABBA example

~~~text
Thread 1 owns A and waits for B
Thread 2 owns B and waits for A
~~~

Neither can release its first lock because neither can acquire its second.

### 12.2 Prevention with lock ordering

Assign every lock a stable position in one global order:

~~~text
account 1 before account 2 before account 3
~~~

Even a transfer from account 3 to account 1 locks account 1 first. Direction
of business work must not change direction of lock acquisition.

Exercise 16 applies this rule to two opposite transfers.

### 12.3 Self-deadlock

A thread can deadlock by acquiring a nonrecursive mutex it already owns. A
default mutex may simply block. Exercise 17 uses an error-checking mutex so the
second acquisition returns `EDEADLK` during development.

### 12.4 Timed waits

`pthread_mutex_timedlock()` can bound how long a diagnostic or recovery path
waits. It does not prove the lock hierarchy is correct. The `deadlock_timeout`
experiment deliberately holds a mutex until a worker's timed request returns
`ETIMEDOUT`.

### 12.5 Priority inversion

Priority inversion occurs when a high-priority thread waits for a resource held
by a lower-priority thread, while intermediate-priority work prevents the
lower-priority owner from running. Priority-inheritance mutex protocols can
address specific real-time cases, but ordinary programs still need short,
well-designed critical regions.

## 13. Pthreads on Linux

POSIX specifies the interface. On modern Linux, glibc supplies the Native POSIX
Thread Library (NPTL), which uses the Linux task model and kernel facilities
such as `clone` and `futex`.

The important boundary is:

~~~text
Application
    |
Pthreads API in glibc
    |
Linux task, memory, and futex mechanisms
~~~

Do not build portable application logic around the exact flags glibc passes to
`clone`. Use the Pthreads contract unless Linux-specific identity or tooling is
the explicit goal.

## 14. Compile and Link Correctly

Use `-pthread` for compilation and linking:

~~~bash
clang -std=c17 -Wall -Wextra -Wpedantic -pthread program.c -o program
~~~

The flag is more appropriate than merely appending a thread library because it
can affect compilation as well as linking.

The repository Makefile automatically adds `-pthread` to every Chapter 7
target.

## 15. Pthread Error Convention

Most Pthread functions do not report failure by returning `-1` and setting
`errno`. They return the error number directly.

Correct pattern:

~~~c
int result = pthread_create(&thread, NULL, worker, argument);
if (result != 0) {
    errno = result;
    perror("pthread_create");
}
~~~

Common mistake:

~~~c
if (pthread_create(...) == -1) {
    perror("pthread_create");
}
~~~

The incorrect version can miss failures and print an unrelated `errno`.

## 16. Creating Threads

The core interface is:

~~~c
int pthread_create(pthread_t *thread,
                   const pthread_attr_t *attributes,
                   void *(*start_routine)(void *),
                   void *argument);
~~~

The worker signature is fixed:

~~~c
void *worker(void *argument);
~~~

`pthread_create()` can return before the new worker runs, after it begins, or
even after it finishes. Never rely on creation order as execution order.

## 17. Argument Lifetime

The Pthread library passes the pointer; it does not copy the pointed-to object.

Broken pattern:

~~~c
for (int i = 0; i < count; ++i) {
    pthread_create(&threads[i], NULL, worker, &i);
}
~~~

All workers receive the same address. They can observe changed values, and the
loop variable eventually leaves scope.

Safer pattern:

~~~c
struct task tasks[count];
for (int i = 0; i < count; ++i) {
    tasks[i].index = i;
    pthread_create(&threads[i], NULL, worker, &tasks[i]);
}
~~~

The task array must remain alive until every worker has finished with it.
Exercise 03 demonstrates this design.

## 18. Thread Identity

Three identities appear in Linux threading work:

### 18.1 Process ID

`getpid()` returns the same PID from all threads in one process.

### 18.2 Kernel task ID

Linux assigns each thread a task ID. This is useful for `/proc/self/task`,
debugging, tracing, and Linux-specific diagnostics.

### 18.3 Pthread ID

`pthread_t` is opaque. Obtain the caller's value with `pthread_self()` and
compare two values with `pthread_equal()`.

Do not assume `pthread_t` is an integer or print it with `%ld` in portable
code.

## 19. Termination

A thread terminates when:

- its start routine returns;
- it calls `pthread_exit()`;
- deferred or asynchronous cancellation takes effect.

The entire process terminates when:

- the initial thread returns from `main()`;
- any thread calls `exit()`;
- the process executes a new program image;
- a fatal process-directed event terminates it.

Exercise 09 demonstrates that the initial thread can call `pthread_exit()` and
leave another worker alive. This is a teaching mechanism, not necessarily the
clearest production shutdown design.

## 20. Joining and Returned Values

New threads are joinable by default. Joining:

1. waits for termination if necessary;
2. retrieves the worker's returned pointer if requested;
3. releases resources retained for the join.

~~~c
void *returned = NULL;
int result = pthread_join(thread, &returned);
~~~

Only one thread should join a given joinable thread. A pointer returned by a
worker must refer to storage that remains alive:

- dynamically allocated storage;
- global or static storage;
- a caller-owned object whose lifetime is guaranteed.

Never return the address of a worker's automatic local variable. Its stack
storage stops being valid when the thread exits.

Exercise 06 transfers a heap allocation and frees it in the joining thread.

## 21. Detaching

Detached threads release their Pthread termination resources automatically.
They cannot be joined.

Detachment does not answer:

- whether the application-level job succeeded;
- whether shared data can be destroyed;
- whether shutdown may proceed.

Exercise 08 uses a condition variable to create an explicit completion
protocol before main destroys the shared synchronization objects.

## 22. Cancellation

`pthread_cancel()` submits a cancellation request. It does not mean the target
has already terminated.

### 22.1 Deferred cancellation

Deferred cancellation takes effect at defined cancellation points. It is the
default and is usually safer because code can control where termination may
occur.

### 22.2 Asynchronous cancellation

Asynchronous cancellation may interrupt the target at nearly any point. If the
target holds a mutex, owns memory, or is modifying shared state, interruption
can leave the process inconsistent. Avoid it unless the code is specifically
designed for that severe constraint.

### 22.3 Cleanup handlers

Register cleanup handlers around resources that must be released during
cancellation. Exercise 12 allocates memory, registers a handler, and confirms
that cancellation invokes it.

Cancellation design must answer:

- which operations are cancellation points;
- what resources can be held there;
- which cleanup handlers run;
- who joins the canceled thread;
- how partial work is represented.

## 23. Condition Variables

The book focuses on mutexes, but the lab includes a small condition-variable
handoff because real lifecycle protocols need a way to wait for state without
polling.

Correct waiting pattern:

~~~c
pthread_mutex_lock(&mutex);
while (!ready) {
    pthread_cond_wait(&condition, &mutex);
}
use_ready_state();
pthread_mutex_unlock(&mutex);
~~~

The mutex protects the predicate. The wait operation atomically releases the
mutex while sleeping and reacquires it before returning.

Signal after changing the protected state:

~~~c
pthread_mutex_lock(&mutex);
ready = 1;
pthread_cond_signal(&condition);
pthread_mutex_unlock(&mutex);
~~~

## 24. Exercise Map and Expected Evidence

### Exercise 01 - Thread identity

Run:

~~~bash
./build/chapters/07-threading/exercises/01_thread_identity
~~~

Representative properties:

~~~text
process_shared=yes kernel_tasks_distinct=yes pthread_ids_distinct=yes
~~~

Exact numeric IDs vary on every run.

### Exercise 02 - Create and join

Expected evidence:

~~~text
answer=42 returned_same_address=yes
thread_created=yes thread_joined=yes result_visible=yes
~~~

### Exercise 03 - Stable arguments

Expected evidence:

~~~text
squares=4,9,16
arguments_stable=yes all_threads_joined=yes
~~~

### Exercise 04 - Shared address space

Expected evidence:

~~~text
value_after_join=73 updated_by_worker=1
address_space_shared=yes join_synchronized=yes
~~~

The join is important. Shared memory alone does not make an unsynchronized
read safe.

### Exercise 05 - Independent stacks

Expected evidence:

~~~text
addresses_nonzero=yes stack_addresses_distinct=yes
memory_model=shared-address-space-with-private-stacks
~~~

### Exercise 06 - Result ownership

Expected evidence:

~~~text
worker_result=42 ownership_transferred=yes
joining_thread_frees_result=yes value_correct=yes
~~~

### Exercise 07 - Join many

Expected evidence:

~~~text
threads_created=4 threads_joined=4 total=30
all_resources_reclaimed=yes results_complete=yes
~~~

### Exercise 08 - Detached completion

Expected evidence:

~~~text
detached=yes completion_observed=yes value=42
join_attempted=no separate_protocol_used=yes
~~~

### Exercise 09 - Initial-thread exit

Both lines must appear:

~~~text
initial_thread_action=pthread_exit
worker_completed_after_initial_thread_exit=yes
~~~

The relative scheduling around creation is not assumed, but the explicit
flush preserves the first line before the initial thread exits.

### Exercise 10 - Opaque identity comparison

Expected evidence:

~~~text
created_matches_worker_self=yes
main_differs_from_worker=yes opaque_ids_compared_portably=yes
~~~

### Exercise 11 - Deferred cancellation

Expected evidence:

~~~text
cancel_request_accepted=yes joined=yes
deferred_cancellation_observed=yes
~~~

### Exercise 12 - Cancellation cleanup

Expected evidence:

~~~text
thread_canceled=yes cleanup_called=yes
worker_allocation_released=yes
~~~

### Exercise 13 - Mutex counter

Expected evidence:

~~~text
expected=40000 actual=40000
critical_region_protected=yes all_threads_joined=yes
~~~

### Exercise 14 - Trylock

Expected evidence:

~~~text
busy_observed=yes worker_blocked=no
~~~

`EBUSY` is an expected state, not a fatal failure.

### Exercise 15 - Account invariant

The order of the two requests is deliberately unspecified. Final balance may
depend on which valid withdrawal enters the critical region first. Required
properties are:

~~~text
one_succeeded=yes no_overdraft=yes invariant_preserved=yes
~~~

### Exercise 16 - Lock ordering

Expected evidence:

~~~text
both_transfers_completed=yes lock_order_consistent=yes total_preserved=yes
~~~

### Exercise 17 - Error-checking mutex

Expected evidence:

~~~text
self_deadlock_detected=yes program_hung=no
~~~

### Exercise 18 - Condition handoff

Expected evidence:

~~~text
ready=1 transformed_value=42
predicate_loop_used=yes handoff_completed=yes
~~~

## 25. Experiment Map

### `logical_race_atomic`

Two workers deliberately split an atomic increment into load and store. A
barrier makes both take the same snapshot before storing. The corrected
`atomic_fetch_add()` performs a single atomic read-modify-write transaction.

Expected relationship:

~~~text
split_load_store < expected
atomic_fetch_add == expected
~~~

### `mutex_contention`

The final count must be correct. The number of `EBUSY` observations varies with
the scheduler and hardware.

### `thread_stack_addresses`

Workers record different automatic-object addresses and the same heap-object
address.

### `proc_task_inventory`

While four workers wait, Linux should expose at least five task directories in
`/proc/self/task`.

### `thread_per_task`

Six tasks use six workers. The design is direct, but thread count scales with
live task count.

### `bounded_worker_pool`

Two workers process eight tasks. Thread count is bounded and creation cost is
reused.

### `deadlock_timeout`

A worker cannot acquire a mutex held by main and returns `ETIMEDOUT` after a
bounded wait. The process never hangs.

## 26. Trace the Linux Boundary

Build and trace:

~~~bash
make chapter CHAPTER=07
chapters/07-threading/scripts/trace.sh 01_thread_identity
~~~

Inspect:

~~~bash
grep -E 'clone|futex|getpid|gettid|exit' \
  chapters/07-threading/observations/strace/01_thread_identity.strace
~~~

What to expect:

- glibc normally creates the worker with `clone` or `clone3`;
- main and worker have one PID but different kernel task IDs;
- joins or contended synchronization may involve `futex`;
- thread termination may appear as an individual `exit`;
- final process termination uses process-wide semantics.

Do not expect one syscall for every Pthread call. Pthreads is a library API,
and user-space fast paths can satisfy uncontended operations.

## 27. Common Mistakes

### Mistake 1 - Treating `pthread_t` as an integer

Use `pthread_equal()` instead of arithmetic or assumed print formats.

### Mistake 2 - Checking Pthread failures through stale `errno`

Most functions return the error number directly.

### Mistake 3 - Passing the address of one changing loop variable

Give each worker stable storage that outlives its use.

### Mistake 4 - Returning a pointer to a worker-local object

The worker's automatic storage becomes invalid at thread termination.

### Mistake 5 - Returning from `main()` before workers finish

Returning from `main()` terminates the process. Join workers or use a clearly
designed alternative lifecycle.

### Mistake 6 - Forgetting to join or detach

A terminated joinable thread retains resources until joined.

### Mistake 7 - Assuming detach means “wait in the background”

Detach removes joining; it does not create an application-level completion
signal.

### Mistake 8 - Believing one source line is atomic

`counter++` and check-then-update sequences are compound operations.

### Mistake 9 - Locking too little

Protect the complete invariant, not only the final store.

### Mistake 10 - Locking too much

Holding a global mutex across unrelated or slow work destroys concurrency.

### Mistake 11 - Acquiring locks in inconsistent order

Define and document one hierarchy before code becomes complex.

### Mistake 12 - Canceling a thread that owns resources without cleanup

Use deferred cancellation and cleanup handlers, or design cooperative shutdown
without cancellation.

### Mistake 13 - Replacing a condition predicate loop with `if`

Wakeups do not guarantee the state remains ready after mutex reacquisition.

### Mistake 14 - Assuming successful tests prove race freedom

Timing bugs can remain dormant. Combine design review, stress tests,
sanitizers, and explicit synchronization reasoning.

## 28. Guided Exercises for You

### Exercise A - Predict identity

Before running Exercise 01, write down:

1. Which values should match?
2. Which values should differ?
3. Why is `pthread_t` not printed numerically?

### Exercise B - Break argument lifetime safely

On paper, explain what can happen if three workers all receive `&i` from one
loop. Do not add undefined behavior to the committed lab. Then explain why the
task-array design fixes both identity and lifetime.

### Exercise C - Ownership table

For Exercises 02, 06, and 08, write a table containing:

- object;
- allocating or initializing thread;
- consuming thread;
- synchronization operation;
- cleanup owner.

### Exercise D - Define the account invariant

Write the invariant from Exercise 15 in one sentence. Identify every field and
decision that must remain inside the mutex-protected region.

### Exercise E - Draw ABBA

Draw two threads, two mutexes, and the four events that produce circular wait.
Then rewrite both paths using the same acquisition order.

### Exercise F - Compare patterns

For a server with 50,000 mostly idle connections, compare:

- one thread per connection;
- an event loop with a bounded worker pool.

Discuss stacks, blocking calls, control-flow complexity, and CPU work.

### Exercise G - Trace fast paths

Trace `13_mutex_counter` and `mutex_contention`. Explain why the number of
`futex` calls does not necessarily equal the number of mutex acquisitions.

## 29. Review Questions

1. What belongs to a process, and what belongs to a thread?
2. Can concurrency exist on one CPU?
3. Why can parallel execution increase the number of possible interleavings?
4. What does Linux's 1:1 thread model mean?
5. Why can a user-level N:1 thread block all its peers?
6. When is a bounded worker pool preferable to thread per task?
7. Why is `counter++` unsafe without synchronization?
8. What is the difference between a C data race and a logical race?
9. What data invariant does a mutex protect?
10. Why should critical regions be small?
11. How does global lock ordering prevent ABBA deadlock?
12. Why is `-pthread` used during compilation and linking?
13. How do Pthread errors differ from conventional `errno` APIs?
14. Why must `pthread_t` be compared with `pthread_equal()`?
15. What lifetime must a worker argument have?
16. What storage may safely be returned through `pthread_join()`?
17. What is the difference between join and detach?
18. Why is deferred cancellation safer than asynchronous cancellation?
19. What does a cleanup handler protect?
20. Why does condition-variable waiting use a `while` loop?

## 30. Compact Answers

1. Process resources are shared; each thread has execution and stack state.
2. Yes. The scheduler can interleave concurrent threads on one CPU.
3. Operations can occur simultaneously as well as in different orders.
4. Each user Pthread maps to one kernel-schedulable task.
5. The kernel sees only one underlying task.
6. When live work can greatly exceed the desired thread count.
7. It is a load-modify-store sequence, not one indivisible transaction.
8. A C data race is undefined behavior; a logical race violates a higher-level
   invariant even if primitive accesses are defined.
9. The shared state relationship that must remain valid.
10. Locks serialize work and can create contention.
11. It removes circular wait by prohibiting reverse acquisition.
12. The flag supplies both compiler and linker thread settings.
13. Most Pthread functions return error numbers directly.
14. POSIX defines `pthread_t` as opaque.
15. It must remain valid until the worker finishes using it.
16. Heap, static, global, or guaranteed caller-owned storage - not a dead stack
   local.
17. Join waits/reclaims and can receive a result; detach selects automatic
   reclamation and removes joining.
18. It acts at controlled cancellation points.
19. Resources and invariants that must be restored during cancellation.
20. Wakeups are notifications to recheck state, not proof of the predicate.

## 31. Compiler and Runtime Engineering Connections

### Parallel compiler work

Compilers may parse, optimize, or generate code for independent modules in
parallel. Shared caches, diagnostic streams, symbol tables, and work queues
need explicit ownership and synchronization.

### Runtime thread state

Language runtimes maintain per-thread stacks, roots, exception state,
allocation buffers, safepoint state, and thread-local storage while sharing the
heap and runtime metadata.

### Garbage collection

A collector may coordinate several mutator threads and collector workers.
Correctness depends on memory-ordering rules, barriers, safepoints, and careful
phase transitions.

### Work stealing

Modern runtimes often use worker pools with per-worker queues and stealing.
This improves load balance but makes queue synchronization and shutdown more
complex.

### JIT compilation

Background JIT workers can compile hot methods while application threads run.
Publishing generated code requires safe ownership, visibility, executable
memory rules, and coordination with code invalidation.

### Debuggers and profilers

Tools relate user-facing thread handles to kernel task IDs, stop and resume
threads, unwind separate stacks, and collect per-thread scheduling evidence.

The Chapter 7 mental model is therefore foundational for compiler backends,
virtual machines, garbage collectors, schedulers, profilers, and concurrent
runtime libraries.

## 32. Mastery Checklist

Before considering Chapter 7 complete, verify that you can:

- [ ] explain binary, process, and thread without conflating them;
- [ ] list important shared and per-thread resources;
- [ ] distinguish concurrency from parallelism;
- [ ] compare 1:1, N:1, and N:M models;
- [ ] compare thread-per-task, worker-pool, and event-driven designs;
- [ ] compile and link Pthreads code with `-pthread`;
- [ ] handle direct Pthread error returns correctly;
- [ ] create threads with lifetime-safe arguments;
- [ ] compare Pthread identities portably;
- [ ] explain return, `pthread_exit()`, and cancellation;
- [ ] join or detach every thread deliberately;
- [ ] transfer returned-object ownership safely;
- [ ] identify a critical region from a data invariant;
- [ ] explain why `counter++` is compound;
- [ ] distinguish data races from logical races;
- [ ] protect shared data with a mutex;
- [ ] choose lock granularity consciously;
- [ ] prevent ABBA deadlock with one lock order;
- [ ] explain deferred cancellation and cleanup handlers;
- [ ] wait on a condition predicate in a loop;
- [ ] interpret `clone` and `futex` evidence without confusing syscalls with
      the higher-level Pthreads contract;
- [ ] connect threading mechanisms to compiler and runtime architecture.

## 33. Primary Local References

Use the installed manual pages while studying:

~~~bash
man 7 pthreads
man 3 pthread_create
man 3 pthread_self
man 3 pthread_equal
man 3 pthread_exit
man 3 pthread_cancel
man 3 pthread_join
man 3 pthread_detach
man 3 pthread_mutex_init
man 3 pthread_mutex_lock
man 3 pthread_mutex_timedlock
man 3 pthread_cond_wait
man 2 clone
man 2 futex
~~~

The POSIX interface should guide portable application behavior. Linux manual
pages, `/proc`, `strace`, GDB, and sanitizer output provide implementation and
runtime evidence.
