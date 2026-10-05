# Chapter 11 Study Guide: Time

Book: Robert Love, *Linux System Programming*, second edition
Scope: Chapter 11, printed pages 363-394
Repository: `linux_system_programming_lab`

This guide is an original companion to the chapter. It reorganizes the topic
around decisions a systems programmer must make and connects the book's APIs
to current Linux behavior. It does not reproduce the book.

## Learning Objectives

After completing this chapter, you should be able to:

1. distinguish wall, monotonic, boot, process, and thread time;
2. choose a clock from the semantics of the problem;
3. represent seconds and fractional seconds without violating invariants;
4. query a clock and its nominal resolution;
5. measure elapsed duration without wall-clock jumps;
6. measure CPU consumption separately from elapsed time;
7. convert between epoch time, UTC, local time, and formatted strings;
8. explain how timezone and daylight-saving rules affect conversion;
9. sleep for a relative duration and recover from interruption;
10. schedule work against an absolute deadline;
11. compare alarm, interval-timer, POSIX-timer, and timerfd models;
12. recognize timer overruns and delayed wake-ups;
13. observe time behavior without assuming every clock read is a system call;
14. connect timer design to runtimes, profilers, schedulers, and event loops.

## 1. Begin With the Question

Many bugs arise because a program asks for "the time" before defining what it
means. The correct source depends on the question.

### Civil timestamp

Questions:

- When was this record created?
- What date should appear in a log?
- Has a certificate's calendar expiration passed?

Use a realtime or calendar clock. The result must relate to UTC and the Unix
epoch so that it can be exchanged with other systems.

### Elapsed duration

Questions:

- How long did this request take?
- Has a timeout interval expired?
- When is the next animation frame due?

Use a monotonic clock. The important property is stable ordering, not a civil
date.

### CPU consumption

Questions:

- How much processor execution did this algorithm consume?
- Which thread is responsible for CPU load?
- How much CPU time should a profiler attribute to this process?

Use a process or thread CPU clock. Wall time includes periods when the task was
not executing.

### Time since boot

Questions:

- How long has the system been running?
- Should a deadline include time spent suspended?

Use an uptime-oriented clock and decide whether suspend time belongs in the
model.

## 2. The Five Clocks Used in This Lab

### CLOCK_REALTIME

This clock represents wall time relative to the Unix epoch. It is suitable for
timestamps. It can be stepped or gradually adjusted by privileged software and
time-synchronization services.

Consequences:

- it may jump forward;
- it may jump backward;
- a difference between two samples can be distorted by correction;
- an absolute realtime deadline follows civil-clock policy.

### CLOCK_MONOTONIC

This clock represents an increasing timeline with an unspecified origin.

Consequences:

- it cannot be converted into a date;
- it is suitable for elapsed durations;
- it is suitable for most timeout deadlines;
- ordinary wall-clock correction does not make it go backward.

On Linux, monotonic time normally excludes intervals when the system is
suspended.

### CLOCK_BOOTTIME

This Linux clock resembles monotonic time but includes suspended intervals.

Choose it when a policy should continue aging while the machine sleeps. An
authentication timeout or maintenance deadline may require that behavior.

### CLOCK_PROCESS_CPUTIME_ID

This clock accumulates CPU charged to all threads in the process. A
multithreaded process can accumulate CPU time on multiple processors at once.
Its process CPU interval can therefore exceed the corresponding wall interval.

### CLOCK_THREAD_CPUTIME_ID

This clock accumulates CPU charged to the calling thread. It is useful for
per-thread profiling and for separating worker costs inside one process.

## 3. Absolute and Relative Time

An absolute value identifies a point on some clock's timeline.

Examples:

- Unix timestamp 1,800,000,000;
- monotonic timestamp 42,300 seconds after an unspecified origin;
- 2026-10-05 12:00 UTC.

A relative value is a duration or displacement.

Examples:

- sleep for 20 milliseconds;
- expire 500 milliseconds from now;
- the operation took 73 microseconds.

The two categories must not be mixed. Adding two absolute timestamps has no
useful meaning. Subtracting compatible absolute timestamps produces a relative
duration. Adding a relative duration to an absolute timestamp produces a new
absolute deadline.

## 4. The Unix Epoch

Unix calendar time is commonly represented as seconds since 1970-01-01
00:00:00 UTC. The epoch provides a shared origin, not a timezone.

Important consequences:

- the stored value is independent of how it will later be displayed;
- converting it to local time requires timezone policy;
- negative values can represent instants before the epoch on implementations
  that support them;
- a `time_t` value should not be assumed to have a particular integer width.

The historical 2038 problem concerns signed 32-bit epoch seconds. Modern
64-bit Linux systems normally use a wider representation, but file formats,
network protocols, embedded systems, and compatibility ABIs may still encode
narrow values. A program must consider the representation at every boundary,
not only in its local `time_t`.

## 5. Kernel Timekeeping: Stable Concepts and Historical Details

The book introduces periodic timer ticks, `HZ`, and jiffies. Those concepts
remain relevant to kernel internals and accounting history, but applications
must not assume one fixed tick frequency.

### HZ

The kernel configuration may select a timer frequency. That choice is not a
portable user-space ABI constant.

### _SC_CLK_TCK

`sysconf(_SC_CLK_TCK)` reports the unit used by interfaces such as `times()`.
It is not a general statement about every hardware or kernel clock.

### Tickless operation

Current kernels can suppress unnecessary periodic ticks while processors are
idle. Therefore a simple picture in which every user-visible clock advances
only on one periodic interrupt is incomplete.

### Hardware clock source

Linux selects an underlying clock source appropriate for the platform. It may
be a processor counter, platform timer, or virtualized clock. The stable
contract for applications is the clock API and its semantics.

The experiment `clocksource_inventory.c` observes the current selection when
sysfs exposes it. The program treats absence as environment-dependent rather
than as a failure of the time API.

## 6. time_t

`time_t` represents calendar time for interfaces such as `time()` and for the
seconds field of other structures.

Good practice:

- treat it as opaque;
- compare it with values converted to `time_t`;
- print it only after a deliberate conversion to a known sufficiently wide
  type;
- do not serialize its raw memory representation;
- do not assume signedness or byte width across platforms.

Exercise 11.01 inspects sizes without making them part of program correctness.

## 7. struct timeval

`struct timeval` contains:

- `tv_sec`: whole seconds;
- `tv_usec`: microseconds within the current second.

For a normalized nonnegative value:

~~~text
0 <= tv_usec < 1,000,000
~~~

The type appears in `gettimeofday()`, `setitimer()`, socket timeouts, and older
interfaces.

The six decimal digits available in `tv_usec` describe representation. They do
not prove microsecond accuracy.

## 8. struct timespec

`struct timespec` contains:

- `tv_sec`: whole seconds;
- `tv_nsec`: nanoseconds within the current second.

For a normalized nonnegative value:

~~~text
0 <= tv_nsec < 1,000,000,000
~~~

POSIX clock, sleep, and timer interfaces prefer this type.

### Adding a duration

1. add seconds;
2. add nanoseconds;
3. while nanoseconds reach one billion, carry one second;
4. restore the normalized invariant.

### Subtracting timestamps

1. subtract seconds;
2. subtract nanoseconds;
3. if nanoseconds are negative, borrow one second;
4. add one billion to the nanosecond field.

Exercise 11.06 uses a fixed example that forces a borrow.

## 9. struct tm

`struct tm` represents civil fields rather than a linear timestamp.

Common fields include:

- seconds;
- minutes;
- hour;
- day of month;
- month;
- years since 1900;
- weekday;
- day of year;
- daylight-saving indicator.

The indexing conventions are easy to misuse:

- January is month zero;
- the year 2026 is stored as 126;
- day-of-month begins at one;
- `tm_isdst = -1` asks `mktime()` to determine daylight-saving state.

Because `struct tm` expresses civil policy, two field sets that look similar
can be ambiguous around a backward daylight-saving transition or invalid
during a forward transition.

## 10. Reading Wall Time

### time()

`time()` returns whole epoch seconds. Passing a pointer also stores the same
value through that pointer.

Failure is represented by `(time_t)-1`.

Use it when whole-second calendar resolution is sufficient.

### gettimeofday()

`gettimeofday()` returns a `timeval`. Its timezone argument is obsolete and
should be `NULL`.

It remains widely available, but new code that already works with clock IDs
and `timespec` often prefers `clock_gettime()`.

### clock_gettime()

`clock_gettime()` makes clock choice explicit:

~~~c
clock_gettime(CLOCK_MONOTONIC, &sample);
~~~

This is the central measurement interface in the chapter lab.

## 11. vDSO and Why strace May Show Nothing

Some high-frequency clock reads can be satisfied through the Linux virtual
dynamic shared object, or vDSO. The kernel maps code and read-only timing data
into the process so libc can obtain a value without entering the kernel on
every call.

Consequences for observation:

- a successful `clock_gettime()` may produce no corresponding strace line;
- absence from strace does not prove the function was not called;
- a debugger, disassembler, `ldd`, `readelf`, or performance tool may provide
  complementary evidence;
- clocks that cannot be served from mapped state may still require a syscall.

This is an important general systems lesson: a library API and a kernel
service do not necessarily imply one system call per invocation.

## 12. Resolution Is Not Accuracy

`clock_getres()` reports a clock's nominal resolution.

Do not confuse:

- **resolution:** smallest representable or reported increment;
- **precision:** repeatability;
- **accuracy:** closeness to a reference;
- **latency:** delay before the caller actually runs after a deadline;
- **granularity of representation:** fractional digits in the type.

If a clock reports one nanosecond, a process is not guaranteed to wake within
one nanosecond. Scheduling, interrupt handling, virtualization, power states,
and system load all affect wake-up latency.

## 13. CPU Time

### Process CPU clock

`CLOCK_PROCESS_CPUTIME_ID` counts processor execution for the process. It is
appropriate for measuring computational cost when blocking time should not be
charged.

### Thread CPU clock

`CLOCK_THREAD_CPUTIME_ID` narrows accounting to the current thread.

### times()

`times()` reports accounting fields such as:

- process user time;
- process system time;
- waited-for children user time;
- waited-for children system time.

The values are `clock_t` ticks. Convert them with the runtime value from
`sysconf(_SC_CLK_TCK)`. A short computation can legitimately measure zero
ticks because it did not cross the accounting granularity.

### clock()

The ISO C `clock()` interface also reports processor time, scaled by
`CLOCKS_PER_SEC`. Do not confuse `CLOCKS_PER_SEC` with the kernel's `HZ` or
with `_SC_CLK_TCK`; they serve different interface contracts.

## 14. Converting Epoch Time to Civil Time

### UTC

`gmtime_r()` converts `time_t` to a caller-owned UTC `struct tm`.

### Local time

`localtime_r()` performs the same conversion using process timezone rules.

### Why use the _r forms?

Traditional `gmtime()` and `localtime()` may return pointers to shared static
storage. A later conversion can overwrite an earlier result, and concurrent
calls require special care. The reentrant variants write into storage supplied
by the caller.

## 15. Timezones Are Policy and Data

A timezone is not just a fixed numeric offset. Real timezone rules can contain:

- historical offset changes;
- daylight-saving transitions;
- political changes;
- abbreviations;
- exceptions that vary by year.

The `TZ` environment variable influences libc conversion. `tzset()` makes the
process apply the current setting.

Exercise 11.12 sets `TZ=UTC0` inside the process to make an `mktime()`
round-trip deterministic. It does not alter the machine's timezone.

For durable interchange, store an epoch value or another clearly defined UTC
representation. Add a timezone identifier separately when the original civil
context matters.

## 16. Formatting With strftime()

`strftime()` formats a `struct tm` into bounded caller storage.

Correct handling requires:

1. provide the true buffer size;
2. check for a zero return;
3. know which directives depend on locale;
4. know whether the input represents UTC or local time;
5. avoid labeling local time as UTC merely because the format resembles ISO
   8601.

Exercise 11.11 uses numeric directives and a fixed `struct tm` so its expected
output is independent of month and weekday names.

## 17. Converting Civil Time With mktime()

`mktime()` interprets a `struct tm` as local time and returns a `time_t`.

It can normalize fields. For example, an out-of-range day may carry into the
next month. That behavior is useful but means a caller should inspect the
normalized structure if invalid input must be rejected rather than adjusted.

`mktime()` can return `(time_t)-1`. Because that bit pattern may also represent
a valid pre-epoch time on some implementations, portable error handling around
very early dates deserves care.

## 18. Setting the System Clock

The chapter describes APIs for changing wall time and adjusting its rate.
These operations normally require privilege and can affect every program on
the system.

This lab deliberately does not call them.

Reasons:

- changing the host clock is outside a learning exercise's safe scope;
- it can invalidate logs, certificates, builds, tests, and distributed state;
- a container or WSL environment may share or virtualize clock authority;
- modern systems normally delegate synchronization to services such as
  chrony, systemd-timesyncd, or another NTP implementation.

The important learning outcome is to recognize that realtime is adjustable,
not to modify it on a development machine.

## 19. Sleeping

Sleeping expresses that a task has no useful work until a duration or deadline
passes. It is not a promise of exact resumption time.

### sleep()

`sleep()` uses whole seconds and returns unslept seconds when interrupted. It
is simple but coarse.

### usleep()

`usleep()` offers microsecond units but is obsolete in modern POSIX profiles.
New code should normally use `nanosleep()` or `clock_nanosleep()`.

### nanosleep()

`nanosleep()` accepts a relative `timespec`. If a caught signal interrupts it:

- return value is `-1`;
- `errno` becomes `EINTR`;
- the remaining duration is stored when a pointer was provided.

Exercise 11.14 arranges that sequence using a child process and `SIGUSR1`.

## 20. Handling EINTR

An interrupted sleep is not automatically an application failure. The caller
must choose a policy.

### Complete the entire relative duration

~~~text
remaining = request
while nanosleep(remaining, remaining) fails with EINTR:
    continue
~~~

This is appropriate when the requirement is "sleep for at least this total
duration."

### Abandon on notification

Return to the event loop or perform signal-driven work. This is appropriate
when the signal changes what the program should do.

### Switch to an absolute deadline

Repeated relative restarts can accumulate time between interruptions and
calls. An absolute monotonic deadline avoids that drift.

## 21. Relative Periodic Drift

Consider a loop:

~~~text
perform 2 ms of work
sleep 10 ms
repeat
~~~

The period is not 10 milliseconds. It is at least 12 milliseconds plus
scheduler delay. If work duration varies, the schedule wanders further.

The experiment `relative_vs_absolute_drift.c` contrasts that approach with
deadlines derived from one origin.

## 22. Absolute Sleeps

`clock_nanosleep()` can accept `TIMER_ABSTIME`.

Advantages:

- work time does not automatically extend the next period;
- interruption does not require calculating a remaining relative interval;
- every deadline can be derived from one start timestamp and period count;
- lateness is easy to measure as `actual - deadline`.

Use a monotonic clock unless civil-clock correction is intentionally part of
the deadline semantics.

Unlike many errno-style interfaces, `clock_nanosleep()` returns an error number
directly. A nonzero result should be interpreted as an error code rather than
checking `errno` blindly.

## 23. Wake-Up Latency

Even when the timer expires exactly at a deadline, the program may resume
later.

Sources include:

- the current task's scheduling policy and priority;
- runnable competition;
- interrupt and softirq work;
- virtualization;
- processor power-state exit;
- page faults after wake-up;
- locks required before useful work can proceed.

Therefore a real-time requirement must describe acceptable lateness and must
be evaluated under representative load. Merely selecting a nanosecond API does
not create deterministic execution.

## 24. alarm()

`alarm(seconds)` schedules one `SIGALRM` for the process.

Properties:

- whole-second granularity;
- one alarm slot per process;
- a new call replaces the previous alarm;
- `alarm(0)` cancels it;
- the return value reports whole seconds remaining from a previous alarm;
- the default action for `SIGALRM` terminates the process.

Install a handler before arming. Exercise 11.16 does this and then waits for the
notification.

## 25. setitimer()

Interval timers provide a `timeval`-based timer with initial and repeating
intervals.

### ITIMER_REAL

Measures wall time and generates `SIGALRM`.

### ITIMER_VIRTUAL

Advances while the process executes in user mode and generates `SIGVTALRM`.

### ITIMER_PROF

Advances during user and system execution charged to the process and generates
`SIGPROF`.

These timer kinds historically support profiling and CPU accounting. Modern
profilers may use more advanced kernel facilities, but the semantic distinction
between wall and CPU timers remains important.

## 26. POSIX Timers

POSIX timers are independent objects created with `timer_create()`.

Lifecycle:

~~~text
choose clock and notification
          |
          v
     timer_create()
          |
          v
     timer_settime()
          |
          +--> zero initial value: disarmed
          +--> nonzero initial value: armed
          +--> nonzero interval: periodic
          |
          v
  receive notification / inspect overrun
          |
          v
      timer_delete()
~~~

Exercise 11.18 blocks a realtime signal before creating the timer and receives
the notification synchronously with `sigwaitinfo()`. This avoids asynchronous
handler restrictions while still demonstrating `SIGEV_SIGNAL` and its payload.

Some older systems require linking with `-lrt`. The repository adds that
library for POSIX timer targets even though newer glibc releases may no longer
need a separate library.

## 27. Timer Specifications

`struct itimerspec` contains:

- `it_value`: time to first expiration;
- `it_interval`: period after the first expiration.

Rules:

- zero `it_value` disarms the timer;
- zero `it_interval` makes it one-shot;
- nonzero `it_interval` makes it periodic;
- both nested `timespec` values must be normalized.

With flags set to zero, values are relative. With `TIMER_ABSTIME`, the initial
expiration is an absolute timestamp on the selected clock.

## 28. Timer Notifications

POSIX supports multiple notification styles, but portability and complexity
vary.

### SIGEV_SIGNAL

Generate a selected signal and optionally carry a `sigval`. This lab blocks
the signal and uses synchronous acceptance.

### SIGEV_NONE

Maintain timer state without asynchronous notification. A program can query
the timer.

### SIGEV_THREAD

Request function invocation in a thread-like context. The implementation and
resource behavior require careful study. It is convenient but can be less
predictable than integrating a timer with an existing event loop.

## 29. Overruns

A periodic timer can expire again before the program consumes its earlier
notification. The additional expirations are overruns.

They matter because one observed signal does not necessarily mean exactly one
period elapsed.

Possible policies:

- repeat work for every missed interval;
- process one update using the total count;
- skip obsolete intermediate work;
- stop because timing guarantees were violated.

`timer_getoverrun()` provides overrun information for POSIX timers. The exact
count is timing-dependent, so the experiment validates a nonnegative result
and reports whether a positive overrun was observed.

## 30. timerfd

Linux `timerfd_create()` represents a timer as a file descriptor.

Benefits:

- no asynchronous signal handler is required;
- readiness integrates with `select()`, `poll()`, and `epoll()`;
- `read()` returns a 64-bit expiration count;
- the count preserves multiple expirations between reads;
- descriptor ownership follows familiar close-on-exec and cleanup rules.

Typical lifecycle:

1. create with `CLOCK_MONOTONIC` and `TFD_CLOEXEC`;
2. configure with `timerfd_settime()`;
3. wait for descriptor readability;
4. read exactly eight bytes into `uint64_t`;
5. process the expiration count;
6. close the descriptor.

Because timerfd is Linux-specific, portable code needs another backend.

## 31. Comparing CLOCK_BOOTTIME and /proc/uptime

`/proc/uptime` exposes text describing system uptime and aggregate idle time.
`CLOCK_BOOTTIME` exposes a clock through the time API.

The experiment samples both and expects nearby positive values. They are read
at different instants and travel through different interfaces, so exact
equality would be the wrong invariant.

This illustrates a general evidence rule: compare according to documented
semantics and measurement uncertainty, not accidental bit-for-bit equality.

## 32. Time and Concurrency

Timeout logic frequently interacts with condition variables, mutexes, event
loops, and signals.

Important questions:

- Which clock defines the deadline?
- Is the deadline absolute or relative?
- Can the wait wake spuriously?
- Is the protected condition rechecked after waking?
- Can another thread change or cancel the timer?
- Who owns the timer object or descriptor?
- What happens if the deadline and completion occur concurrently?

A timeout is not merely a delay. It is part of a concurrent state machine.

## 33. Common Error Patterns

### Measuring duration with time()

Whole-second resolution and wall-clock correction make it unsuitable for many
benchmarks and timeouts.

### Treating CLOCK_REALTIME as monotonic

Clock synchronization or administration can invalidate elapsed calculations.

### Assuming timespec fields are always normalized

Invalid nanoseconds can cause `EINVAL` or incorrect arithmetic.

### Ignoring EINTR

The program may sleep less than intended or treat a normal signal as a fatal
error.

### Restarting fixed relative periods

Work and scheduling overhead accumulate into drift.

### Arming before installing the handler

The timer can expire under the default signal disposition.

### Performing complex work in a timer signal handler

Most library functions are not async-signal-safe. Use a flag, self-pipe,
signalfd, or synchronous waiting design.

### Ignoring overruns

One notification can represent multiple elapsed periods.

### Assuming a clock read must appear in strace

The vDSO may satisfy it without a system call.

### Serializing raw libc structures

Type widths, padding, byte order, and ABI choices are not a stable file or
network format.

## 34. Failure and Return Conventions

Time APIs do not all report errors the same way.

### Traditional errno-style functions

Examples include `clock_gettime()`, `nanosleep()`, `setitimer()`, and
`timer_create()`.

They generally return `-1` and set `errno`.

### Direct error-number functions

`clock_nanosleep()` returns an error number directly. Pthread APIs follow a
similar convention.

### Sentinel-valued functions

`time()` uses `(time_t)-1`; `times()` uses `(clock_t)-1`; conversion functions
may return `NULL`.

Read each interface's contract rather than applying one universal pattern.

## 35. Safe Testing Strategy

Time tests are vulnerable to flakiness when they demand exact durations.

This lab uses semantic invariants:

- nanosecond and microsecond fields remain in range;
- monotonic samples do not move backward;
- a completed sleep does not finish before its request;
- an interrupted sleep reports `EINTR` and positive remaining time;
- an absolute sleep reaches or passes its deadline;
- timer notifications are received;
- expiration counts are positive rather than exact;
- two uptime samples are close rather than equal.

Every smoke-test invocation is wrapped in `timeout` so a regression cannot
hang the suite indefinitely.

## 36. Observation With strace

Useful trace targets include:

- `clock_gettime` and `clock_getres` when they are not served by vDSO;
- `clock_nanosleep`;
- `setitimer`;
- POSIX timer operations;
- `timerfd_create` and `timerfd_settime`;
- signal delivery and `rt_sigreturn`;
- `read()` of a timerfd expiration count.

Interpret absence carefully. A conversion such as `strftime()` is ordinary
user-space computation. A clock read may use vDSO. A sleep or timer setup
normally requires kernel participation.

## 37. Prediction-Driven Experiments

Before running each experiment, record a prediction.

### Wall versus CPU time

Prediction: sleeping advances wall elapsed time much more than process CPU
time.

### Realtime versus monotonic sampling

Prediction: monotonic samples never decrease. Realtime probably increases in
one short run, but its API semantics permit correction.

### Relative versus absolute periodic work

Prediction: work time accumulates into a relative sleep loop, while fixed
absolute deadlines remain anchored to the original schedule.

### Timer overrun

Prediction: a fast periodic timer blocked for many periods records one or more
additional expirations.

### timerfd accumulation

Prediction: delaying a read produces a count greater than one on an otherwise
normally scheduled system.

### Boot-time comparison

Prediction: `CLOCK_BOOTTIME` and `/proc/uptime` produce nearby positive values.

### Clock-source inventory

Prediction: if sysfs exposes the interface, the active source also appears in
the available-source list.

## 38. Relevance to Compiler and Runtime Engineering

### Benchmarking

Use monotonic elapsed time for end-to-end latency and CPU clocks for processor
cost. Report repetitions, warm-up policy, system load, compiler flags, and
uncertainty rather than one unexplained number.

### Profiling

Sampling and instrumentation profilers need clocks whose cost and semantics
match the measurement. Per-thread CPU time can separate concurrent workers.

### Garbage collection

Runtimes schedule incremental work, enforce pause budgets, and measure stop-the-
world intervals. Monotonic deadlines are central to these policies.

### Event loops

An event loop manages many absolute deadlines, often with a heap or timing
wheel. Linux timerfd can wake the loop through the same readiness mechanism as
network and file descriptors.

### JIT compilation

A runtime may trigger compilation after a method consumes a CPU or invocation
budget. Wall time and CPU time answer different optimization questions.

### Build systems

Filesystem timestamps describe artifact state, while monotonic measurements
describe task duration. Clock skew across machines complicates distributed
build validity.

## 39. Mastery Questions

Answer these without consulting the guide:

1. Why can realtime move backward while monotonic time cannot?
2. Why can process CPU time exceed wall time?
3. When should suspend time count toward a timeout?
4. What does the Unix epoch define, and what does it not define?
5. Why is `time_t` unsuitable as a raw portable serialization format?
6. What invariants apply to `timeval` and `timespec` fractional fields?
7. How do you subtract two timespec values when nanoseconds go negative?
8. Why does nanosecond representation not imply nanosecond accuracy?
9. How can `clock_gettime()` succeed without appearing in strace?
10. What is the difference between `_SC_CLK_TCK` and `CLOCKS_PER_SEC`?
11. Why should UTC storage and local display usually be separate?
12. What do `tm_year`, `tm_mon`, and `tm_isdst` mean?
13. Why can a local civil time be ambiguous?
14. What does `strftime()` return when the destination is insufficient?
15. How should a program respond when `nanosleep()` returns `EINTR`?
16. Why do repeated relative sleeps drift?
17. What advantage does `TIMER_ABSTIME` provide?
18. Why can a process wake after its timer's nominal expiration?
19. What signal does `alarm()` generate?
20. How do the three `setitimer()` timer kinds differ?
21. What makes POSIX timers more flexible than `alarm()`?
22. What is a timer overrun?
23. Why can one signal represent several periodic expirations?
24. What does a timerfd read return?
25. Why is timerfd attractive for an epoll-based runtime?

## 40. Practical Review Checklist

Before moving beyond Chapter 11, verify that you can:

- [ ] select a clock from requirements rather than habit;
- [ ] use `clock_gettime()` and check its result;
- [ ] query resolution without equating it to accuracy;
- [ ] perform normalized timespec arithmetic;
- [ ] measure wall and CPU time separately;
- [ ] convert epoch time to UTC and local `struct tm` values;
- [ ] format time into a bounded buffer;
- [ ] explain timezone and DST ambiguity;
- [ ] complete or abandon an interrupted sleep deliberately;
- [ ] construct an absolute monotonic deadline;
- [ ] explain periodic drift;
- [ ] install or block a signal before arming its timer;
- [ ] configure and delete a POSIX timer;
- [ ] interpret an overrun or expiration count;
- [ ] integrate timerfd conceptually with an event loop;
- [ ] explain why a vDSO clock read may be absent from strace;
- [ ] connect clock choice to profiling and runtime engineering.

## Compact API Map

| Need | Interface | Key caution |
|---|---|---|
| Whole-second wall time | `time()` | wall clock can change |
| Legacy microsecond wall time | `gettimeofday()` | obsolete timezone argument |
| Explicit clock sample | `clock_gettime()` | choose clock semantics |
| Nominal clock resolution | `clock_getres()` | not wake-up accuracy |
| Process accounting ticks | `times()` | use `_SC_CLK_TCK` |
| UTC conversion | `gmtime_r()` | caller-owned output |
| Local conversion | `localtime_r()` | timezone/DST policy |
| Civil-to-epoch conversion | `mktime()` | local time and normalization |
| Bounded text formatting | `strftime()` | zero can mean insufficient space |
| Relative high-resolution sleep | `nanosleep()` | handle `EINTR` |
| Clock-selected/absolute sleep | `clock_nanosleep()` | returns error number directly |
| Simple whole-second alarm | `alarm()` | one process slot |
| Legacy interval timer | `setitimer()` | signal-driven and timeval-based |
| Independent POSIX timer | `timer_create()` | lifecycle and overruns |
| Linux descriptor timer | `timerfd_create()` | Linux-specific, read 64-bit count |

## Final Perspective

Time programming is less about obtaining a number than preserving meaning.
A timestamp, elapsed interval, CPU budget, civil date, sleep request, and timer
expiration are different abstractions. Robust systems code names the intended
clock, keeps representations normalized, treats calendar conversion as policy,
expects interruption and scheduling delay, and accounts for missed periods.

That discipline is directly transferable to runtimes, profilers, schedulers,
databases, distributed systems, and performance engineering.
