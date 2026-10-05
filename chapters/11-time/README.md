# Chapter 11 - Time

**Status:** Lab prepared; study in progress

## Purpose

Understand the distinct clocks Linux exposes, represent and convert calendar
time correctly, measure elapsed and consumed CPU time, sleep without avoidable
drift, and build safe timer-driven programs.

## Book Scope

Robert Love, *Linux System Programming*, second edition, Chapter 11, printed
pages 363-394.

## Study Material

- `notes/study-guide-pages-363-394.md`
- `mental-models/time.md`

## Exercises

1. Inspect the standard time data types
2. Read seconds since the Unix epoch with `time()`
3. Read wall time with `gettimeofday()`
4. Distinguish realtime and monotonic clocks
5. Query clock resolution with `clock_getres()`
6. Normalize `timespec` arithmetic
7. Measure process CPU time
8. Measure current-thread CPU time
9. Read process accounting ticks with `times()`
10. Convert one timestamp to UTC and local civil time
11. Format broken-down time with `strftime()`
12. Round-trip civil time through `mktime()`
13. Measure a completed `nanosleep()`
14. Recover remaining sleep time after `EINTR`
15. Sleep to an absolute monotonic deadline
16. Schedule a whole-second alarm
17. Arm a subsecond interval timer
18. Create, receive, and delete a POSIX timer

## Experiments

- compare wall elapsed time with consumed process CPU time;
- sample realtime and monotonic clocks repeatedly;
- compare relative periodic sleeps with absolute deadlines;
- observe POSIX timer overruns;
- consume accumulated expirations through Linux `timerfd`;
- compare `CLOCK_BOOTTIME` with `/proc/uptime`;
- inspect the active kernel clock source through sysfs.

## Commands

~~~bash
make chapter CHAPTER=11
make test CHAPTER=11
make tidy-chapter CHAPTER=11
chapters/11-time/scripts/trace.sh 04_realtime_and_monotonic
chapters/11-time/scripts/trace.sh timerfd_expirations
~~~

Generated files are placed under:

~~~text
build/chapters/11-time/
~~~

## Safety and Determinism

The lab never changes the host clock or timezone configuration. The mktime
exercise changes `TZ` only inside its short-lived process. Sleeps and timers
use bounded durations, tests run each program under `timeout`, and signal-based
timers install a disposition or block their notification before arming.
Clock-source sysfs inspection degrades cleanly when a container or kernel does
not expose that optional interface.

## Completion Rule

Building the lab is not the same as mastering it. Before declaring the chapter
complete, explain wall, monotonic, boot, process, and thread time; absolute and
relative deadlines; resolution versus precision and accuracy; epoch and civil
time conversion; timezone and daylight-saving policy; normalized timespec
arithmetic; interruption and remaining sleep; periodic drift; alarm,
`setitimer()`, POSIX timers, overruns, and `timerfd`; and why elapsed-duration
logic should normally use a monotonic clock.
