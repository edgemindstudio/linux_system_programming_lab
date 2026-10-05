# Chapter 11 Observations

Store deliberately retained Chapter 11 evidence here. Generated traces belong
under `observations/strace/`, which is ignored by Git.

Useful questions for each observation:

- Which clock semantics did the program require?
- Did libc use a system call, the vDSO, or both?
- Was the value absolute, relative, civil, monotonic, or CPU-accounting time?
- What resolution did the interface report?
- Could wall-clock correction affect the result?
- Did a sleep complete, return `EINTR`, or wake late?
- Was a periodic schedule relative or deadline-based?
- Were expirations coalesced, counted, or reported as overruns?
- Did the experiment depend on Linux-specific procfs, sysfs, or timerfd?

Do not commit machine-specific traces merely because they were generated.
Retain evidence only when it supports a written observation useful to future
study.
