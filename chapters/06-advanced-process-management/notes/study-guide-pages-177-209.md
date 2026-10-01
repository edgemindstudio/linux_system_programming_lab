# Chapter 6 Study Guide - Advanced Process Management

Book: Robert Love, *Linux System Programming*, second edition
Scope: Chapter 6, printed pages 177-209
Repository: `linux_system_programming_lab`

## How to Use This Guide

Chapter 5 answered, "How is a process created, replaced, and collected?"
Chapter 6 asks a different set of questions:

1. When a process is runnable, when does it actually execute?
2. How much CPU service does it receive relative to competitors?
3. On which CPUs may Linux place it?
4. Which delays matter to a deadline-sensitive program?
5. What resources may the process consume?

Work through the lab in that order. For each program:

1. read the header comment;
2. identify the process attribute being observed or changed;
3. predict which values depend on your machine;
4. run the program;
5. trace it when a system-call boundary matters;
6. explain the result in your own words.

All files in `exercises/` and `experiments/` are complete standalone C
programs. You never need to assemble fragments before compiling them.

## Build the Lab

From the repository root:

~~~bash
make chapter CHAPTER=06
make test CHAPTER=06
~~~

Expected final test line:

~~~text
PASS: all deterministic Chapter 6 checks completed
~~~

Executables and generated data appear under:

~~~text
build/chapters/06-advanced-process-management/
├── data/
├── exercises/
└── experiments/
~~~

## Chapter Summary

The Linux scheduler divides finite CPU capacity among runnable tasks. A task
waiting for an event is blocked and does not compete for the processor. A
runnable task is eligible, but eligibility is not the same as currently
running.

Love builds the chapter around several controls:

- preemptive scheduling lets the kernel interrupt one task and run another;
- normal scheduling tries to make progress and distribute CPU service fairly;
- nice values influence relative service within normal scheduling;
- processor affinity limits the CPUs on which a task may run;
- real-time policies give strict priority relationships to deadline-sensitive
  work;
- memory locking and prefaulting can remove some page-fault delays;
- resource limits place kernel-enforced ceilings on process consumption.

These controls solve different problems. A nice value is not an affinity mask.
An affinity mask is not a real-time guarantee. A real-time policy does not
eliminate page faults. A resource limit is not a scheduler priority.

The professional habit is therefore to inspect the complete execution
contract instead of reaching for one powerful-looking setting.

## Modern Linux Context: CFS and EEVDF

The book explains the Completely Fair Scheduler (CFS), which replaced Linux's
older O(1) scheduler in Linux 2.6.23. CFS remains the right historical model
for understanding weighted fairness, virtual runtime, and the effect of nice
values.

Modern kernels have continued evolving. Official kernel documentation says
Linux began transitioning the fair scheduling class toward Earliest Eligible
Virtual Deadline First (EEVDF) in Linux 6.6. EEVDF retains virtual-runtime and
fair-share ideas while adding lag and virtual deadlines to improve latency
selection.

This does not make the book useless. It changes how we phrase the lesson:

- stable user-space APIs such as `getpriority()`, `sched_getaffinity()`, and
  `getrlimit()` remain the programming contract;
- CFS details are a historical implementation model, not an eternal ABI;
- `/proc/self/sched` fields are useful observations, not portable promises;
- experiments must record the kernel and environment that produced them.

The lab therefore teaches Love's concepts and explicitly labels current
kernel internals as environment-dependent.

## Page-to-Lab Map

| Printed pages | Main idea | Lab evidence |
|---|---|---|
| 177-180 | Runnable tasks, preemption, CPU-bound and I/O-bound work | `01_scheduler_snapshot`, `cpu_io_bound` |
| 180-183 | Fair scheduling and yielding | `02_yield_processor`, `cfs_nice_competition`, `proc_sched_inventory` |
| 183-186 | Nice values, get/set priority, I/O priority | exercises 03-05, `nice_permission_probe`, `io_priority_inventory` |
| 186-190 | Processor affinity | exercises 06-07, `affinity_migrations` |
| 190-201 | Real time, policies, priorities, intervals, and safety | exercises 08-12 |
| 201-204 | Determinism, prefaulting, and memory locking | `18_mlock_one_page`, `memlock_faults` |
| 204-209 | Soft and hard resource limits | exercises 13-17 |

## 1. Runnable, Running, and Blocked

A process can exist without competing for CPU time.

### Running

A running task is currently executing instructions on a logical CPU. On a
system where this process may use nine logical CPUs, at most nine eligible
tasks can literally execute at one instant on those CPUs.

### Runnable

A runnable task has work it could execute, but it may be waiting in a run
queue because another task currently owns the CPU.

### Blocked

A blocked task is waiting for something such as:

- bytes from a file, pipe, socket, or terminal;
- a timer expiration;
- a child process;
- a synchronization event;
- a device operation;
- a page that must be supplied by the kernel.

Blocked tasks are not losing a scheduling competition. They are temporarily
ineligible because their required event has not occurred.

### Run the scheduling snapshot

~~~bash
./build/chapters/06-advanced-process-management/exercises/01_scheduler_snapshot
~~~

Representative output:

~~~text
policy=SCHED_OTHER nice=0 allowed_cpus=9 online_cpus=9
policy_known=yes allowed_cpu_positive=yes online_cpu_positive=yes
~~~

Your CPU counts may differ. The values mean:

- `SCHED_OTHER` is the ordinary time-sharing policy;
- nice 0 is the usual inherited default;
- `allowed_cpus` is the effective affinity mask;
- `online_cpus` describes processors currently online to the environment.

The two counts need not match. Containers, WSL, cpusets, `taskset`, and service
managers may give a process a subset of online CPUs.

## 2. Preemption and the Illusion of Simultaneous Work

Linux uses preemptive multitasking. The kernel can stop one running task and
select another without requiring the first task's cooperation.

This matters because cooperative scheduling trusts every program to yield
often. One broken infinite loop could otherwise monopolize a CPU. With
preemption, the scheduler remains responsible for sharing execution.

A context switch changes which task is executing. Context switches have costs:

- register and execution-state changes;
- scheduler work;
- disruption to cache and translation locality;
- possible migration to another CPU.

Very short service intervals improve responsiveness but increase switching
overhead. Very long intervals improve locality but may increase response time.
Scheduler design balances these competing goals.

## 3. CPU-Bound and I/O-Bound Are Behaviors

A CPU-bound task remains runnable and consumes most CPU time offered to it. A
large calculation, encoder, compiler optimization pass, or busy loop may be
CPU-bound for a period.

An I/O-bound task frequently blocks while waiting for external work. A command
copying data, an interactive shell, or a server waiting for requests may spend
much more wall time blocked than running.

These are behaviors, not permanent labels. A compiler can be I/O-bound while
reading headers, CPU-bound during optimization, and I/O-bound again while
writing an object file.

Run the observation experiment:

~~~bash
./build/chapters/06-advanced-process-management/experiments/cpu_io_bound
~~~

Representative output:

~~~text
kind=C voluntary=0 involuntary=14 user_us=146981
kind=I voluntary=20 involuntary=0 user_us=386
reports_collected=yes observation_is_environment_dependent=yes
~~~

The sleeping worker voluntarily blocks repeatedly. The busy worker consumes
user CPU time and may be preempted involuntarily. Exact counts are not tests of
correctness; they are measurements from the current run.

## 4. Fair Scheduling

Love's CFS explanation replaces the idea of one fixed timeslice assigned to
every task with proportional service. A task's virtual runtime represents
weighted CPU service. A task owed service is favored relative to one that has
received more.

The key lesson is proportional fairness under contention:

- with only one runnable task, it can use the CPU regardless of a modest nice
  disadvantage;
- with several runnable tasks, weights influence how service is divided;
- sleeping and waking change which tasks are competing;
- service is evaluated over time, not guaranteed in every short interval.

Run the contention experiment:

~~~bash
./build/chapters/06-advanced-process-management/experiments/cfs_nice_competition
~~~

Representative output:

~~~text
cpu=0 worker_a_nice=0 iterations=310001664
cpu=0 worker_b_nice=10 iterations=39043072
competition_completed=yes observation_only=yes
~~~

Both workers are pinned to one allowed CPU so they genuinely contend. The
nice-0 worker will usually do more work, but iteration ratios are not portable
benchmarks. Host load, virtualization, clock calls, CPU frequency, and kernel
implementation all affect the result.

Inspect current scheduler accounting:

~~~bash
./build/chapters/06-advanced-process-management/experiments/proc_sched_inventory
~~~

Typical selected fields include virtual runtime, switch counts, policy, and
internal priority. Treat their names and meanings as kernel-version-specific.

## 5. Yielding the Processor

`sched_yield()` says that the caller is willing to move behind other runnable
tasks at the same static priority.

It does not mean:

- sleep for a known duration;
- wait until another process finishes;
- release a mutex;
- guarantee another task will execute;
- make an event become ready.

Run:

~~~bash
./build/chapters/06-advanced-process-management/exercises/02_yield_processor
~~~

Representative output:

~~~text
sched_yield_result=0 elapsed_ns=1682
elapsed_nonnegative=yes yield_is_not_sleep=yes
~~~

The elapsed value may be tiny if no equal-priority competitor needs the CPU.
A loop that repeatedly checks a condition and calls `sched_yield()` is still a
polling loop. Prefer blocking synchronization that expresses the actual event.

## 6. Nice Values

The normal Linux nice range is conventionally:

~~~text
-20                 0                 +19
most favored     ordinary         least favored
~~~

The direction often causes mistakes: a larger numeric nice value means the
process is being *nicer* to competing work and receives less weight.

Nice values affect normal scheduling under contention. They do not create a
hard CPU reservation, and they do not make a blocked process runnable.

### The special getpriority() rule

`getpriority()` can successfully return `-1`, but system calls also commonly
use `-1` for failure. Correct code therefore does this:

~~~c
errno = 0;
value = getpriority(PRIO_PROCESS, 0);
if (value == -1 && errno != 0) {
    /* failure */
}
~~~

Run:

~~~bash
./build/chapters/06-advanced-process-management/exercises/03_getpriority_errno
~~~

Representative output:

~~~text
nice=0 priority_read=yes errno_after=0
minus_one_would_be_success=not-observed errno_protocol_used=yes
~~~

### Lower only this short-lived process

~~~bash
./build/chapters/06-advanced-process-management/exercises/04_lower_own_priority
~~~

Representative output:

~~~text
before=0 requested=1 after=1
priority_not_raised=yes shell_unchanged=yes
~~~

The shell is not modified. It launched a child process, and the child changed
only its own nice value before terminating.

### Nice values across fork()

~~~bash
./build/chapters/06-advanced-process-management/exercises/05_priority_inheritance
~~~

Expected relationship:

~~~text
child_inherited=yes values_match=yes
~~~

The child begins with the parent's nice value. Parent and child are separate
tasks after `fork()`, so later changes do not automatically rewrite each
other's value.

### Permission to raise priority

An unprivileged process can normally make itself less favored. Making itself
more favored requires `CAP_SYS_NICE` or authorization through `RLIMIT_NICE`.

Run the isolated probe:

~~~bash
./build/chapters/06-advanced-process-management/experiments/nice_permission_probe
~~~

Common unprivileged output:

~~~text
before=0 requested=-1 raise_allowed=no errno=13
probe_isolated_in_child=yes parent_priority_unchanged=yes
~~~

The exact result depends on capabilities and limits. The important safety
property is that any successful elevation exists only in the child, which
exits immediately.

## 7. I/O Priority

CPU scheduling and storage scheduling are separate layers. Linux exposes I/O
priority values, but whether they influence actual completion order depends on
the active I/O scheduler and the device stack.

Run:

~~~bash
./build/chapters/06-advanced-process-management/experiments/io_priority_inventory
~~~

Representative output:

~~~text
ioprio_supported=yes raw=0 class=0 class_name=none/default data=0
device_enforcement_not_assumed=yes
~~~

This experiment uses the Linux system call directly because many glibc
versions do not expose a convenient wrapper. A default class may derive
behavior from the process's CPU nice value. Do not infer device-level effect
from the mere existence of a numeric setting.

The `ionice` utility is useful for human experiments:

~~~bash
ionice -p $$
ionice -c 3 command
~~~

Do not apply I/O-priority experiments to important workloads until you know
the scheduler and storage stack involved.

## 8. Processor Affinity

Affinity is a set of logical CPU identifiers. Linux may run the task on any CPU
in the effective set.

Read the current mask:

~~~bash
./build/chapters/06-advanced-process-management/exercises/06_affinity_inventory
~~~

Representative output:

~~~text
allowed_count=9 first_cpu=0 last_cpu=8
mask_nonempty=yes identifiers_ordered=yes
~~~

Never assume CPU 0 is allowed. The program searches the current mask.

### Pin and restore

~~~bash
./build/chapters/06-advanced-process-management/exercises/07_affinity_pin_restore
~~~

Representative output:

~~~text
selected_cpu=0 pinned_count=1 restored_count=9
pin_verified=yes restore_verified=yes
~~~

The important lifecycle is:

1. retrieve the existing mask;
2. choose a CPU already present in it;
3. apply the narrower mask;
4. retrieve and verify the effective result;
5. restore the original mask.

Pinning may improve locality or simplify an experiment, but unnecessary
pinning can create hotspots and stop Linux from balancing work.

### Observe migration rather than assuming it

~~~bash
./build/chapters/06-advanced-process-management/experiments/affinity_migrations
~~~

Both of these are valid short-run outcomes:

~~~text
distinct_cpus=1 migrations=0
distinct_cpus=3 migrations=7
~~~

Zero migration does not prove that the process is pinned. It only says no
migration was observed during that sampling window.

## 9. Real-Time Means Meeting Deadlines

Real-time is frequently confused with "fast." A system is real-time when
correctness depends on completing required work within timing constraints.

### Vocabulary

- **Latency:** elapsed time between an event and the relevant response.
- **Jitter:** variation in latency across repeated events.
- **Deadline:** latest acceptable completion time.
- **Hard real time:** missing a deadline is a system failure.
- **Soft real time:** misses reduce quality or usefulness but are tolerated.

Average speed is insufficient. A program averaging 100 microseconds can still
be unsuitable if rare executions take 20 milliseconds and the deadline is 1
millisecond.

## 10. Scheduling Policies and Static Priority

Inspect the current policy:

~~~bash
./build/chapters/06-advanced-process-management/exercises/08_policy_inventory
~~~

Representative output:

~~~text
policy=SCHED_OTHER policy_number=0 static_priority=0
policy_read=yes parameter_read=yes
~~~

### SCHED_OTHER

The standard normal policy. Its static `sched_priority` is zero. Nice and the
normal fair-scheduling implementation decide relative service within this
class.

### SCHED_BATCH

A Linux normal policy for noninteractive, CPU-intensive work. It also uses
static priority zero.

### SCHED_IDLE

A Linux policy for extremely low-priority background work. It is lower than
ordinary nice-based work.

### SCHED_FIFO

A fixed-priority real-time policy without round-robin time slicing among
equal-priority work. A runnable FIFO task continues until it blocks, yields,
is preempted by a higher real-time priority, or terminates.

### SCHED_RR

Real-time round-robin. It follows FIFO priority rules but rotates equal-
priority tasks after a quantum.

### SCHED_DEADLINE

A newer Linux-specific policy not covered by the book's original treatment.
It uses runtime, deadline, and period parameters with admission control. It
requires the Linux `sched_setattr()` interface and careful system design. This
lab does not activate it.

## 11. Discover Priority Ranges

Do not hardcode real-time priority endpoints in portable code.

~~~bash
./build/chapters/06-advanced-process-management/exercises/09_policy_priority_ranges
~~~

Typical Linux output:

~~~text
other=0..0 fifo=1..99 rr=1..99
normal_priority_zero=yes realtime_ranges_valid=yes
~~~

POSIX requires a useful range but not Linux's exact 1-99 values. Query with
`sched_get_priority_min()` and `sched_get_priority_max()`.

## 12. Safely Practice sched_setscheduler()

The lab calls `sched_setscheduler()` only to reapply the current normal policy:

~~~bash
./build/chapters/06-advanced-process-management/exercises/10_safe_policy_update
~~~

Representative output:

~~~text
before_policy=0 after_policy=0 static_priority=0
safe_update_applied=yes policy_unchanged=yes priority_unchanged=yes
~~~

If the program somehow starts under a real-time policy, it skips the update.
The exercise teaches the API shape without creating a process capable of
starving the terminal or build environment.

## 13. Round-Robin Interval

~~~bash
./build/chapters/06-advanced-process-management/exercises/11_rr_interval
~~~

Possible Linux output under the normal policy:

~~~text
interval_sec=0 interval_nsec=0 total_ns=0
interval_query=yes representation_valid=yes
~~~

Another kernel may report a positive value. POSIX requires this query for
`SCHED_RR`; Linux also accepts it for other policies. A reported quantum is
not a guarantee that the process will run continuously for that duration.
Higher-priority work, blocking, interrupts, and other events still matter.

## 14. Inspect the Real-Time Boundary Before Acting

~~~bash
./build/chapters/06-advanced-process-management/exercises/12_realtime_capability_boundary
~~~

Representative output:

~~~text
euid=1000 rlimit_rtprio_soft=0 fifo_range=1..99
boundary_inspected=yes realtime_policy_changed=no limit_allows_realtime=no
~~~

The effective UID alone does not completely describe capability state,
especially in namespaces and containers. `RLIMIT_RTPRIO` can authorize an
unprivileged ceiling. The lab reports the boundary but intentionally does not
enter a real-time policy.

### Why this caution is necessary

A runnable high-priority FIFO loop may prevent normal tasks from executing.
Safe real-time development normally includes:

- a bounded work loop;
- known blocking or yielding points;
- `RLIMIT_RTTIME` where appropriate;
- retained CPU time for recovery work;
- watchdog and termination mechanisms;
- validation on a dedicated environment;
- privilege limited to the exact requirement.

Running an arbitrary infinite loop with real-time privilege is not a learning
exercise. It is a denial-of-service risk.

## 15. Determinism Requires More Than Scheduling

Even a high-priority task can be delayed by:

- a first-touch page fault;
- swapped or reclaimed memory;
- a dynamic allocation path;
- cache and TLB misses;
- CPU migration;
- interrupt handling;
- blocking file or network I/O;
- lock contention and priority inversion;
- frequency changes;
- unbounded library behavior.

This is why Love connects real-time scheduling to prefaulting, memory locking,
and affinity.

### Lock one page safely

~~~bash
./build/chapters/06-advanced-process-management/exercises/18_mlock_one_page
~~~

Success example:

~~~text
page_size=4096 mlock_supported=yes errno=0
page_prefaulted=yes lock_released=yes
~~~

Limit-denied example:

~~~text
page_size=4096 mlock_supported=no errno=12
page_prefaulted=yes lock_released=not-needed
~~~

The program first writes the page so that it is resident, then calls `mlock()`,
then `munlock()`. A denied lock is an environmental observation because
`RLIMIT_MEMLOCK` and capabilities differ across systems.

### Observe first-touch faults

~~~bash
./build/chapters/06-advanced-process-management/experiments/memlock_faults
~~~

Representative output:

~~~text
pages_touched=64 minor_fault_delta=64
mlock_succeeded=yes errno=0 prefault_before_lock=yes
~~~

The exact fault delta can vary. The experiment demonstrates that successful
allocation does not necessarily mean every page is already backed and
resident. First touch often performs work that a latency-sensitive path should
not discover unexpectedly.

## 16. Resource Limits

Resource limits are kernel-enforced ceilings stored as process state. Each
limit has:

- a **soft limit**, which is currently enforced;
- a **hard limit**, which bounds how high an unprivileged process may raise the
  soft limit.

Read a focused inventory:

~~~bash
./build/chapters/06-advanced-process-management/exercises/13_resource_limits_inventory
~~~

Representative output:

~~~text
RLIMIT_NOFILE soft=1024 hard=1048576
RLIMIT_STACK soft=8388608 hard=infinity
RLIMIT_CORE soft=0 hard=infinity
RLIMIT_NPROC soft=127000 hard=127000
RLIMIT_MEMLOCK soft=8388608 hard=8388608
limits_read=yes soft_is_active=yes hard_is_ceiling=yes
~~~

Your shell, login manager, container, service manager, administrator policy,
and kernel can all affect the inherited values.

### Important limits

| Limit | What it constrains |
|---|---|
| `RLIMIT_AS` | Virtual address-space size |
| `RLIMIT_CORE` | Core-dump file size |
| `RLIMIT_CPU` | CPU time before signals are generated |
| `RLIMIT_DATA` | Data-segment size on applicable allocation paths |
| `RLIMIT_FSIZE` | Maximum file size created by the process |
| `RLIMIT_MEMLOCK` | Bytes that may be locked in memory |
| `RLIMIT_NOFILE` | File-descriptor-number ceiling |
| `RLIMIT_NPROC` | Process/thread count for a real user ID |
| `RLIMIT_RTPRIO` | Unprivileged real-time-priority ceiling |
| `RLIMIT_RTTIME` | Continuous real-time CPU use without blocking |
| `RLIMIT_SIGPENDING` | Queued signals for a real user ID |
| `RLIMIT_STACK` | Process stack size |

Some limits are Linux-specific, and some historical limits are not enforced by
modern kernels. Always consult the current manual page rather than assuming
that every named limit acts identically on every system.

## 17. Safely Change a Soft Limit

~~~bash
./build/chapters/06-advanced-process-management/exercises/14_lower_soft_nofile
~~~

Representative output:

~~~text
temporary_soft=64 hard_preserved=yes
temporary_applied=yes original_restored=yes
~~~

The program never lowers the hard limit. It temporarily lowers the soft value
and restores it while the original hard ceiling still permits restoration.

Common rule:

~~~text
soft <= hard
~~~

An unprivileged process may normally:

- lower its soft limit;
- raise its soft limit up to the hard limit;
- lower its hard limit;
- not raise its hard limit again.

Because lowering the hard limit is difficult to reverse, experiments should do
that only in disposable child processes when it is truly necessary.

## 18. Observe Kernel Enforcement

~~~bash
./build/chapters/06-advanced-process-management/exercises/15_rlimit_fsize_enforcement
~~~

Expected summary:

~~~text
file_size=1024 child_exit=0
size_limited=yes parent_survived=yes
~~~

The child receives a 1024-byte `RLIMIT_FSIZE`. Linux allows only 1024 bytes to
reach the file; the next write fails with `EFBIG` and produces `SIGXFSZ`. The
child ignores that signal so it can verify the error and exit normally.

The limit is installed only in the child. The parent does not sacrifice its
own ability to write larger files.

## 19. Limits Across fork() and exec()

Resource limits are inherited across `fork()`:

~~~bash
./build/chapters/06-advanced-process-management/exercises/16_limit_inheritance
~~~

Expected relationship:

~~~text
limit_inherited=yes child_exit=0 parent_restored=yes
~~~

They are also preserved across `exec()`:

~~~bash
./build/chapters/06-advanced-process-management/exercises/17_exec_limit_preservation
~~~

Expected relationship:

~~~text
limit_preserved_across_exec=yes child_exit=0
~~~

This explains why a tool launched by a compiler driver, shell, IDE, service, or
container can fail under a ceiling it never set itself. The limit came from its
execution lineage.

## 20. Trace the Kernel Boundary

The chapter tracing helper follows children and records scheduling, affinity,
priority, resource-limit, memory-locking, and process-management calls.

Trace affinity changes:

~~~bash
chapters/06-advanced-process-management/scripts/trace.sh 07_affinity_pin_restore

grep -E 'sched_(get|set)affinity' \
  chapters/06-advanced-process-management/observations/strace/07_affinity_pin_restore.strace
~~~

Trace a resource limit:

~~~bash
chapters/06-advanced-process-management/scripts/trace.sh 15_rlimit_fsize_enforcement

grep -E 'clone|prlimit64|write|wait4' \
  chapters/06-advanced-process-management/observations/strace/15_rlimit_fsize_enforcement.strace
~~~

Trace the privilege probe:

~~~bash
chapters/06-advanced-process-management/scripts/trace.sh nice_permission_probe

grep -E 'clone|setpriority|getpriority|wait4' \
  chapters/06-advanced-process-management/observations/strace/nice_permission_probe.strace
~~~

On modern glibc, `getrlimit()` and `setrlimit()` may appear as the more general
`prlimit64` system call. This is another example of the C-library interface not
necessarily matching the exact kernel entry point seen by `strace`.

## 21. Common Mistakes and Corrections

### Mistake: assuming every existing process competes for the CPU

Only runnable tasks compete. Blocked tasks are waiting for events.

### Mistake: treating CPU-bound and I/O-bound as permanent process types

They describe current behavior. One application can move between both.

### Mistake: believing sched_yield() is a sleep

It does not request a duration and may return immediately.

### Mistake: using sched_yield() as synchronization

Use a blocking primitive representing the actual condition or event.

### Mistake: reversing the nice scale

Numerically larger nice values make normal tasks less favored.

### Mistake: checking only whether getpriority() returned -1

Clear and inspect `errno` because `-1` is also a successful nice value.

### Mistake: expecting nice to guarantee a CPU percentage

Nice changes relative weight under contention; it is not a reservation.

### Mistake: assuming CPU 0 belongs to the process

Read the affinity mask. Containers and cpusets may exclude CPU 0.

### Mistake: treating one observation of zero migrations as proof of pinning

A scheduler may simply have had no reason to migrate the task during that run.

### Mistake: calling real-time scheduling "faster"

Real-time concerns deadlines and bounded latency, not average throughput.

### Mistake: running an infinite SCHED_FIFO loop as root

It can starve normal work and make the system difficult to recover. This lab
does not perform that operation.

### Mistake: hardcoding real-time priority 1-99 as portable

Query the policy range using the scheduler API.

### Mistake: assuming a real-time policy removes page faults

Scheduling policy and memory residency are different mechanisms.

### Mistake: locking memory before understanding RLIMIT_MEMLOCK

The request may fail, or broad locking may consume resources needed elsewhere.

### Mistake: lowering a hard limit and expecting ordinary code to restore it

An unprivileged process generally cannot raise the hard limit again.

### Mistake: assuming exec resets limits

Resource limits are preserved across program-image replacement.

### Mistake: treating book-era scheduler internals as an ABI

The user-space interfaces are stable contracts; internal scheduling algorithms
continue evolving.

## 22. Guided Exercises for You

Do these after understanding the supplied programs. Keep each new experiment
inside the Chapter 6 directory and add deterministic checks only for behavior
Linux actually guarantees.

### Exercise A - Compare shell and program limits

1. Run `ulimit -a`.
2. Run `13_resource_limits_inventory`.
3. Match shell names to `RLIMIT_*` constants.
4. Explain differences caused by units or omitted limits.

### Exercise B - Launch under taskset

1. Read the current allowed mask.
2. Select one CPU actually present.
3. Run `taskset -c CPU 06_affinity_inventory`.
4. Predict and verify the reported count.

Do not assume that the host CPU numbering begins at zero inside every
environment.

### Exercise C - Repeat the migration sample

Run `affinity_migrations` ten times while the system is idle and ten times
under load. Record distinct CPUs and observed migration counts. Explain why
the experiment does not prove scheduler quality.

### Exercise D - Vary nice competition

Modify a copy of `cfs_nice_competition.c` to compare nice 0 with nice 5, 10,
15, and 19. Run several repetitions. Report medians and ranges rather than one
ratio.

### Exercise E - Prove the shell is unchanged

Read the shell's nice value before and after `04_lower_own_priority`. Explain
the parent/child boundary that protects the shell.

### Exercise F - Descriptor exhaustion in a child

In a child only, lower `RLIMIT_NOFILE`, repeatedly open `/dev/null`, and record
where `open()` fails with `EMFILE`. Reserve descriptors for reporting and
cleanup. Never assume the count equals the soft value exactly because standard
descriptors and library activity already consume entries.

### Exercise G - Stack limit reasoning

Do not crash the main shell. Design a child-process experiment that lowers
`RLIMIT_STACK` and executes bounded recursive work. State why compiler
optimization, architecture, frames, alternate signal stacks, and undefined
behavior make exact recursion depth difficult to predict.

### Exercise H - Real-time safety review

Write a design checklist for a hypothetical low-latency audio worker. Include
deadline, worst-case work, blocking points, memory preparation, affinity,
priority inversion, `RLIMIT_RTTIME`, watchdog behavior, privileges, and an
emergency recovery path. Do not activate a real-time policy.

### Exercise I - Compiler-driver inheritance

Design a launcher that lowers `RLIMIT_NOFILE`, executes a compiler, captures
stderr, and decodes the exit status. Predict which compilation stages might
fail first when many files are open.

### Exercise J - Current scheduler evidence

Record:

~~~bash
uname -a
cat /proc/version
cat /proc/self/sched
~~~

Separate stable API conclusions from kernel-version-specific observations.

## 23. Review Questions

1. What is the difference between runnable and running?
2. Why does a blocked process not compete for CPU time?
3. What is preemption?
4. Why are very small scheduling intervals not free?
5. Can one program be CPU-bound and I/O-bound at different times?
6. What fairness idea does virtual runtime represent?
7. Why is `sched_yield()` usually a poor synchronization mechanism?
8. Which direction on the nice scale gives a normal process more weight?
9. Why must `errno` be cleared before `getpriority()`?
10. What does an affinity mask constrain?
11. Why might allowed CPUs differ from online CPUs?
12. Why does zero observed migration not prove affinity to one CPU?
13. What distinguishes real-time correctness from ordinary speed?
14. Define latency, jitter, and deadline.
15. How do `SCHED_FIFO` and `SCHED_RR` differ?
16. Why should priority ranges be queried?
17. What makes an uncontrolled FIFO loop dangerous?
18. Why might a real-time task prefault and lock memory?
19. What does `RLIMIT_MEMLOCK` constrain?
20. What is the difference between a soft and hard resource limit?
21. Which limit is currently enforced?
22. Why is lowering a hard limit risky?
23. What happens to limits across `fork()`?
24. What happens to limits across `exec()`?
25. Why might `strace` show `prlimit64` for a `getrlimit()` call?
26. Why should CFS be treated as a historical implementation model rather
    than a permanent ABI?

## 24. Mastery Checklist

Before calling the chapter studied, verify that you can:

- [ ] distinguish existing, runnable, running, and blocked tasks;
- [ ] explain preemption without describing it as process cooperation;
- [ ] classify observed behavior as CPU-bound or I/O-bound;
- [ ] explain weighted fairness in plain language;
- [ ] state what `sched_yield()` does and does not guarantee;
- [ ] read and safely change a nice value;
- [ ] use the `getpriority()` errno protocol;
- [ ] explain nice inheritance across `fork()`;
- [ ] inspect and modify an affinity mask without assuming CPU 0;
- [ ] explain why CPU migration observations vary;
- [ ] distinguish normal, FIFO, RR, and deadline policies;
- [ ] query policy priority ranges;
- [ ] explain why this lab avoids real-time promotion;
- [ ] define latency, jitter, and deadline;
- [ ] identify sources of nondeterministic delay outside the scheduler;
- [ ] prefault, lock, unlock, and free a page safely;
- [ ] distinguish soft and hard limits;
- [ ] read, lower, and restore a soft limit;
- [ ] explain limit enforcement, inheritance, and exec preservation;
- [ ] interpret scheduler and limit system calls in `strace`;
- [ ] separate stable API behavior from current kernel internals;
- [ ] connect these mechanisms to compiler and runtime systems.

## 25. Connection to Compilers and Runtimes

### Parallel compilation

A build tool decides how many compiler processes to keep runnable.
Oversubscribing CPUs can increase context switches and memory pressure.
Affinity may help a
controlled benchmark, but broad pinning can prevent load balancing.

### Background optimization

An IDE or JIT may lower the nice value weight of speculative or background
work so interactive tasks remain responsive. That choice is relative policy,
not a guarantee that optimization receives a fixed CPU share.

### Runtime pause latency

A garbage collector or managed runtime may care about worst-case pause time.
Scheduling is only one component. Page faults, allocation, locks, CPU
migration, interrupts, and cache behavior also shape latency and jitter.

### Toolchain process limits

Compiler drivers launch preprocessors, assemblers, linkers, and helpers.
Inherited `RLIMIT_NOFILE`, `RLIMIT_NPROC`, `RLIMIT_STACK`, or `RLIMIT_AS`
values can explain failures that appear far from the process that originally
configured the limit.

### JIT memory preparation

A latency-sensitive JIT may prefault memory before a critical phase. Memory
locking can remove paging uncertainty for selected regions, but it must stay
within resource budgets and does not replace correct synchronization or
scheduling design.

### Evidence-driven performance work

Scheduler tuning without measurement is guesswork. Record kernel version,
affinity, nice values, limits, workload, host pressure, and timing
distribution. Compare repeated runs and distinguish average throughput from
worst-case latency.

## Further Primary References

- Linux kernel EEVDF documentation:
  <https://docs.kernel.org/scheduler/sched-eevdf.html>
- Linux `sched(7)` overview:
  <https://man7.org/linux/man-pages/man7/sched.7.html>
- Linux resource-limit interface:
  <https://man7.org/linux/man-pages/man2/getrlimit.2.html>
- Linux CPU-affinity interface:
  <https://man7.org/linux/man-pages/man2/sched_setaffinity.2.html>
- Linux memory-locking interface:
  <https://man7.org/linux/man-pages/man2/mlock.2.html>

These references supplement the book. The lab remains organized around Love's
Chapter 6 concepts while explicitly checking current Linux behavior.
