# Chapter 9 Observations

This directory holds deliberately retained evidence from Chapter 9 runs.
Generated binaries and data remain under `build/` and are not committed.

Create a focused system-call trace with:

~~~bash
chapters/09-memory-management/scripts/trace.sh 09_anonymous_mmap
chapters/09-memory-management/scripts/trace.sh demand_paging_faults
~~~

Traces are written to `observations/strace/`, which is ignored because process
IDs, addresses, loader activity, library paths, page-fault counts, and timing
are machine- and run-dependent.

Useful evidence to record in your own words includes:

- which mappings appear in `/proc/self/maps`;
- where `brk()`, `mmap()`, `mremap()`, and `munmap()` occur;
- how first page touches change minor-fault counts;
- whether `realloc()` moved storage in a particular run;
- whether one-page locking was allowed by current policy and limits;
- the current overcommit mode and why it is not a per-process guarantee.
