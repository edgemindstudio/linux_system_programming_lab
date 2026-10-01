# Chapter 6 - Advanced Process Management

**Status:** Lab prepared; study in progress

## Purpose

Understand how Linux decides which runnable process executes, where it may
execute, and what resource ceilings constrain it. The chapter connects normal
scheduling, nice values, CPU affinity, real-time interfaces, deterministic
preparation, memory locking, and per-process resource limits.

## Book Scope

Robert Love, *Linux System Programming*, second edition, Chapter 6, printed
pages 177-209.

## Study Material

- `notes/study-guide-pages-177-209.md`
- `mental-models/advanced-process-management.md`

## Exercises

1. Snapshot policy, nice value, and allowed CPUs
2. Call `sched_yield()` and distinguish yielding from sleeping
3. Use the special `getpriority()` errno protocol
4. Safely lower the current process's priority
5. Observe nice-value inheritance across `fork()`
6. Inventory the CPU-affinity mask
7. Pin to one allowed CPU and restore the original mask
8. Inspect scheduling policy and static priority
9. Discover policy-specific priority ranges
10. Reapply a normal policy without entering real time
11. Query the round-robin interval
12. Inspect the real-time privilege and limit boundary
13. Inventory important resource limits
14. Temporarily lower and restore `RLIMIT_NOFILE`
15. Observe `RLIMIT_FSIZE` enforcement in a child
16. Verify resource-limit inheritance across `fork()`
17. Verify resource-limit preservation across `exec()`
18. Prefault, lock, unlock, and free one memory page

## Experiments

- compare CPU-bound work with blocking work;
- place two differently nice workers on one CPU;
- sample CPU placement and migrations;
- inspect Linux I/O priority through the raw system call;
- relate first-touch page faults to memory locking;
- probe priority-raising permission in an isolated child;
- inspect kernel-specific scheduler accounting in `/proc/self/sched`.

## Commands

~~~bash
make chapter CHAPTER=06
make test CHAPTER=06
make tidy-chapter CHAPTER=06
chapters/06-advanced-process-management/scripts/trace.sh 07_affinity_pin_restore
chapters/06-advanced-process-management/scripts/trace.sh nice_permission_probe
~~~

Generated files are placed under:

~~~text
build/chapters/06-advanced-process-management/
~~~

## Safety Choices

The lab never promotes a process into `SCHED_FIFO` or `SCHED_RR`. Those
policies can starve ordinary work when used incorrectly. It queries real-time
interfaces and limits, but the only scheduler update reapplies the process's
existing normal policy. Priority-raising probes run in short-lived children,
affinity changes are restored, memory locks are released, and hard resource
limits are never reduced in the parent.

## Completion Rule

Building the lab is not the same as mastering it. Before moving to Chapter 7,
explain runnable versus blocked state, CPU-bound versus I/O-bound behavior,
preemption, CFS fairness, the nice scale, affinity masks, real-time policies,
latency and jitter, the purpose and danger of memory locking, and the
difference between soft and hard resource limits.
