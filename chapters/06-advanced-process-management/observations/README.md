# Chapter 6 Observations

Keep deliberate, reviewed evidence here. Generated traces belong in the
ignored `observations/strace/` directory unless a specific trace is selected
for permanent study.

Useful observations include:

- policy, nice value, and affinity differences between environments;
- CPU placement before and after affinity changes;
- `clone`, `sched_*`, `getpriority`, `setpriority`, and `prlimit64` calls;
- permission failures such as `EACCES` or `EPERM`;
- `/proc/self/sched` fields on the current kernel;
- soft and hard limit values inherited from the shell;
- minor page faults before and after first-touch access;
- evidence that a result varies under host load or virtualization.

Record the kernel version, CPU environment, container or WSL context, and
command used whenever those facts affect interpretation.
