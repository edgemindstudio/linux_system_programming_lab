# Mental Model: Time in Linux Programs

## One Word, Several Quantities

"Time" does not identify one universal counter. A program must first decide
what question it is asking.

| Question | Appropriate source |
|---|---|
| What civil date should be recorded? | `CLOCK_REALTIME` or `time()` |
| How long did an operation take? | `CLOCK_MONOTONIC` |
| How long since boot, including suspend? | `CLOCK_BOOTTIME` |
| How much CPU did this process consume? | `CLOCK_PROCESS_CPUTIME_ID` |
| How much CPU did this thread consume? | `CLOCK_THREAD_CPUTIME_ID` |

Choosing a clock is a semantic decision before it is an API decision.

## Clock Layers

~~~text
hardware counters / clock source
            |
            v
Linux timekeeping and clock adjustment
            |
            +--> realtime clock ------> civil timestamps
            |
            +--> monotonic clock -----> elapsed intervals
            |
            +--> boot clock ----------> uptime including suspend
            |
            +--> CPU accounting ------> process and thread clocks
            |
            v
libc and POSIX interfaces
~~~

The kernel may use a hardware counter such as TSC, HPET, or a paravirtualized
clock source. User space should rely on clock semantics, not a named device or
an assumed timer frequency.

## Realtime Is Correctable

`CLOCK_REALTIME` represents the system's current civil time. It is anchored to
the Unix epoch and is useful for audit timestamps, file and database dates,
calendar events, and communication with users and other systems.

It may change when an administrator, virtual-machine host, or synchronization
service corrects the wall clock. Therefore subtracting two realtime samples
does not provide the strongest elapsed-time guarantee.

## Monotonic Time Is for Intervals

`CLOCK_MONOTONIC` has an unspecified origin. Its absolute value is not a date.
Its useful property is ordering: later samples do not precede earlier samples.

~~~text
start = monotonic_now()
perform_operation()
end = monotonic_now()
elapsed = end - start
~~~

A duration has meaning even though neither timestamp can be printed as a
calendar date.

## Boot Time and Suspend

On Linux, `CLOCK_BOOTTIME` resembles monotonic time but includes time spent in
system suspend. That distinction matters for credentials that expire while a
laptop sleeps, maintenance scheduled relative to boot, and watchdogs whose
deadlines should include suspend time.

## CPU Clocks

CPU clocks answer a different question: how much processor execution was
charged to a process or thread?

~~~text
wall interval = running + runnable waiting + sleeping + blocked
CPU interval  = time actually executing on a processor
~~~

A sleeping program accumulates wall time with almost no CPU time. A
multithreaded process running simultaneously on several processors can consume
more process CPU time than the corresponding wall interval.

## Representation

### time_t

`time_t` represents calendar seconds. Treat it as an opaque arithmetic type;
do not hard-code its width.

### timeval

~~~c
struct timeval {
    time_t      tv_sec;
    suseconds_t tv_usec;
};
~~~

The fractional field must remain in `[0, 1,000,000)`.

### timespec

~~~c
struct timespec {
    time_t tv_sec;
    long   tv_nsec;
};
~~~

The fractional field must remain in `[0, 1,000,000,000)`. Nanosecond
representation does not promise nanosecond accuracy. It merely allows the
interface to express that granularity.

### struct tm

`struct tm` is broken-down civil time: year, month, day, hour, minute, second,
weekday, day of year, and daylight-saving state. Several fields use historical
offsets: `tm_year` counts from 1900, `tm_mon` ranges from 0 through 11, and
`tm_mday` ranges from 1 through 31.

## Arithmetic Requires Normalization

Subtract the seconds and nanoseconds separately. When the nanosecond result is
negative, borrow one second:

~~~text
seconds     -= 1
nanoseconds += 1,000,000,000
~~~

The normalized result again satisfies the timespec invariant.

## Resolution, Precision, Accuracy, and Latency

- **Representation:** digits available in the data type.
- **Resolution:** smallest nominal clock step reported by `clock_getres()`.
- **Precision:** repeatability of measurements.
- **Accuracy:** closeness to the intended physical or civil time.
- **Wake-up latency:** delay between a deadline and actual execution.

A clock can report one-nanosecond resolution while a sleeping process wakes
thousands of nanoseconds late.

## Calendar Conversion Pipeline

~~~text
time_t epoch value
      |
      +--> gmtime_r() -----> UTC struct tm
      |
      +--> localtime_r() --> local struct tm + timezone/DST policy
                                  |
                                  v
                              strftime()
~~~

The inverse local-time operation is `mktime()`. It normalizes fields and
consults timezone rules. An ambiguous or nonexistent civil time can occur near
daylight-saving transitions, so storing UTC timestamps is usually safer than
storing only local clock fields.

## Relative Sleep

`nanosleep()` requests a duration. A signal can interrupt it:

~~~text
nanosleep(request, remaining)
        |
        +--> completed: return 0
        |
        +--> signal: return -1, errno = EINTR, remaining updated
~~~

Restarting with the remaining duration is correct for one sleep, but repeated
relative sleeps can drift because work and scheduling delay are added to each
cycle.

## Absolute Deadlines

`clock_nanosleep(..., TIMER_ABSTIME, ...)` waits for a timestamp rather than a
duration.

~~~text
origin
  + period --> deadline 1
  + period --> deadline 2
  + period --> deadline 3
~~~

Every deadline remains anchored to the same origin. This prevents work time
from becoming part of the next requested period.

## Timer Families

| Interface | Resolution/model | Notification |
|---|---|---|
| `alarm()` | whole seconds, one process slot | `SIGALRM` |
| `setitimer()` | `timeval`, three timer kinds | timer-specific signal |
| POSIX timers | `timespec`, independent objects | signal/thread options |
| `timerfd` | Linux descriptor interface | readable counter |

## Signal Timer Discipline

Before arming a signal-producing timer:

1. install the handler or block the signal;
2. initialize every timer field;
3. arm the timer;
4. handle interruption and overruns;
5. disarm or delete the timer during cleanup.

Reversing the first two steps creates a race in which the default disposition
may run before the program is ready.

## Expiration Is Not Execution

A timer expiring means the deadline passed and a notification became eligible.
It does not guarantee immediate execution. The task may be descheduled, the
signal may be blocked, or the event loop may be busy.

Periodic timers therefore need an overrun policy. A program must decide
whether to process every missed period, coalesce several periods into one
update, skip directly to the newest state, or record the delay as a failure.

## timerfd and Event Loops

Linux `timerfd` turns timer state into descriptor readiness:

~~~text
timer expires
     |
     v
timerfd becomes readable
     |
     v
read uint64_t expiration count
~~~

The 64-bit count preserves how many expirations occurred since the previous
successful read. Because it is a file descriptor, it integrates naturally
with `poll()`, `select()`, and `epoll()` without asynchronous signal handlers.

## Compiler and Runtime Connection

Compilers, runtimes, virtual machines, and profilers rely on these distinctions:

- benchmark harnesses need monotonic elapsed time;
- profilers need process or thread CPU clocks;
- garbage collectors schedule periodic or deadline-driven work;
- runtimes implement timeout queues and event-loop timers;
- build systems timestamp artifacts but measure task durations monotonically;
- distributed traces need wall timestamps plus clock-sync metadata.

## Core Invariants

1. Use realtime for civil timestamps, not duration measurement.
2. Use monotonic clocks for elapsed intervals and deadlines.
3. Never infer accuracy from the number of fractional digits.
4. Normalize `timespec` and `timeval` arithmetic.
5. Treat timezone conversion as policy, not simple arithmetic.
6. Expect sleeps and timers to wake late, never exactly on schedule.
7. Handle `EINTR` deliberately.
8. Prefer absolute deadlines for periodic work.
9. Install or block timer signals before arming.
10. Account for missed expirations explicitly.
