# Chapter 5 Observations

Keep deliberately retained evidence from Chapter 5 process investigations
here. Generated data and routine test output belong under
`build/chapters/05-process-management/` and are not committed.

The tracing helper writes ignored traces to `observations/strace/`:

~~~bash
make chapter CHAPTER=05
chapters/05-process-management/scripts/trace.sh 04_fork_exec_wait
chapters/05-process-management/scripts/trace.sh subreaper_adoption
~~~

Useful observations include:

- the `clone()` or `clone3()` operation used beneath the C `fork()` wrapper;
- the unchanged PID across a successful `execve()`;
- inherited descriptors and close-on-exec behavior;
- `wait4()` or `waitid()` collecting child state;
- `setsid()` and `setpgid()` changing process relationships;
- transient zombie state in `/proc/PID/stat` before reaping;
- reparenting to a child subreaper in modern supervisors.

Do not commit raw traces containing unrelated environment details. Inspect
them, extract the relevant lesson, and retain only evidence that is useful and
safe to share.
