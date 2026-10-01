# Mental Model - Advanced Process Management

## The scheduler does not run every process

At any instant, Linux has many processes, but only runnable tasks compete for
CPU time. A task waiting for a file, pipe, timer, lock, terminal, or network
packet is blocked and does not need a processor until its event occurs.

~~~text
all processes
├── runnable  -> eligible to compete for a CPU
├── running   -> currently executing on a CPU
└── blocked   -> waiting for an event; not competing
~~~

On a machine with N usable logical CPUs, at most N ordinary tasks execute at
the same instant. More runnable tasks create contention; the scheduler chooses
which ones run next.

## Policy, priority, and placement are different controls

~~~text
scheduling policy
    decides the scheduling class and its rules

nice value
    adjusts relative weight inside normal scheduling

affinity mask
    limits the CPUs on which the task may execute

resource limits
    cap resources the process may consume or request
~~~

Changing one control does not automatically change the others.

## Normal scheduling and CFS

The Completely Fair Scheduler models fairness through virtual runtime. A task
that has received less weighted CPU service is favored relative to one that has
received more. Nice values change the weight of that accounting.

~~~text
lower numeric nice value
        -> greater scheduling weight

higher numeric nice value
        -> smaller scheduling weight
~~~

Nice is not a reservation. It does not mean "run exactly X percent" and it
cannot make a blocked task runnable.

## CPU-bound and I/O-bound behavior

~~~text
CPU-bound task
    remains runnable and consumes available CPU service

I/O-bound task
    runs briefly, requests or awaits an event, then blocks
~~~

Interactive responsiveness depends heavily on waking blocked tasks promptly.
Throughput-oriented computation benefits from long useful runs and cache
locality. Real programs often move between both behaviors.

## Yielding is only a hint to reschedule

`sched_yield()` tells Linux that the caller is willing to let another eligible
task at the same static priority run. It is not a timer, synchronization
primitive, or guarantee that another task exists. Correct synchronization uses
events, blocking I/O, condition variables, semaphores, futexes, or related
mechanisms rather than a yield loop.

## Affinity constrains placement

~~~text
machine online CPUs
        ∩
cpuset/container/system policy
        ∩
process affinity request
        =
effective CPUs available to the process
~~~

Pinning can reduce migration and improve locality, but it can also create load
imbalance and reduce the scheduler's freedom. Always inspect the existing mask
instead of assuming CPU 0 is available.

## Real-time means deadlines, not merely speed

A real-time workload is judged by whether required work finishes before a
deadline.

~~~text
latency = time from event to response
jitter  = variation in that latency
deadline = latest acceptable completion time
~~~

`SCHED_FIFO` and `SCHED_RR` outrank normal scheduling. A runnable real-time task
can prevent normal tasks from running. Policy changes therefore require
careful privilege, bounded work, blocking points, resource limits, watchdogs,
and recovery plans.

## Determinism is end-to-end

Selecting a scheduling policy cannot remove every delay. A real-time design
also considers:

- page faults;
- memory allocation;
- cache misses;
- CPU migration;
- interrupts and kernel activity;
- blocking I/O;
- locks and priority inversion;
- frequency and power management;
- unbounded library work.

Prefaulting and memory locking can remove some page-fault uncertainty, but they
consume a limited resource and do not make the whole program deterministic.

## Resource limits are inherited process state

Every limit has two values:

~~~text
soft limit -> active ceiling enforced now
hard limit -> maximum soft value available without privilege
~~~

A process may normally lower either value and may raise the soft value only up
to the hard value. Lowering the hard limit is intentionally difficult to undo.
Limits are copied across `fork()` and preserved across `exec()`.

## Compiler and runtime connection

Compiler drivers, build systems, JITs, garbage collectors, language servers,
and virtual machines all meet these mechanisms:

- worker affinity influences cache locality and NUMA placement;
- nice values affect background compilation under contention;
- process limits bound open files, subprocesses, stack, address space, and
  generated output;
- runtime pause latency depends on scheduling, page faults, and contention;
- real-time interfaces illustrate why bounded operations matter;
- inherited limits can explain failures after a compiler driver launches a
  tool or runtime helper.

The central question is not "How do I force Linux to run my process?" It is:
"What scheduling and resource contract does this process currently have, and
what evidence shows that the contract fits the workload?"
