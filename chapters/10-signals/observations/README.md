# Chapter 10 Observations

This directory holds deliberately retained evidence from Chapter 10 runs.
Generated traces remain untracked because PIDs, timing, addresses, loader
activity, interruption points, and scheduling order vary between runs.

Create a focused system-call trace with:

~~~bash
chapters/10-signals/scripts/trace.sh 02_sigaction_handler
chapters/10-signals/scripts/trace.sh 15_sigsuspend_wait
chapters/10-signals/scripts/trace.sh signalfd_dispatch
~~~

Traces are written to `observations/strace/`.

Useful evidence to record in your own words includes:

- the `rt_sigaction` call that installs a disposition;
- the `rt_sigprocmask` calls that modify and restore a mask;
- how `kill`, `tgkill`, or `rt_sigqueueinfo` generates a signal;
- where `rt_sigsuspend` or a blocking call returns through `EINTR`;
- how `SA_RESTART` changes the visible return path of `read()`;
- how standard pending state differs from real-time queueing;
- how `signalfd4` turns selected blocked signals into readable records;
- which parts of a trace are guarantees and which are scheduling observations.
