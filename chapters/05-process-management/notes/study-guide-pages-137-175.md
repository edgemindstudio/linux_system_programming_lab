# Chapter 5 Study Guide — Process Management

Book: Robert Love, *Linux System Programming*, second edition
Scope: Chapter 5, printed pages 137–175
Repository: `linux_system_programming_lab`

## How to Use This Guide

Do not try to memorize every function name in one sitting. Work through the
chapter as a lifecycle:

1. identify one running process;
2. create a child with `fork()`;
3. replace the child's program with `exec()`;
4. terminate that program;
5. collect its status with a wait function;
6. place processes into groups and sessions;
7. connect the old daemon model to modern service supervision.

For each program:

1. read its header comment;
2. predict its output;
3. compile the whole chapter;
4. run that single program;
5. trace it when a system-call boundary matters;
6. explain the result without looking at this guide.

All source listings in `exercises/` and `experiments/` are complete standalone C
programs. Nothing in this guide asks you to assemble fragments into a working
file.

## Build the Lab

From the repository root:

~~~bash
make chapter CHAPTER=05
make test CHAPTER=05
~~~

Expected final test line:

~~~text
PASS: all deterministic Chapter 5 checks completed
~~~

Executables and generated data appear under:

~~~text
build/chapters/05-process-management/
├── data/
├── exercises/
└── experiments/
~~~

## Chapter Summary

Robert Love's central design point is that Unix separates two operations that
many systems present as one:

- `fork()` creates another process;
- an `exec` function replaces a process's current program image.

That separation makes pipelines, redirection, shells, supervisors, and service
launchers flexible. The child can adjust descriptors, credentials, process
groups, signal state, or other attributes after `fork()` and before `exec()`.

The other half of process creation is process collection. A parent owns the
responsibility for learning how each child changed state. When a child
terminates, Linux retains a small zombie record until the parent waits for it.
The wait status says whether the child exited normally, was killed by a signal,
stopped, or continued.

The chapter then widens from one parent and child to the surrounding process
structure:

- credentials answer which user and groups a process acts as;
- process groups let a shell control a pipeline as one job;
- sessions organize process groups around a login or service;
- traditional daemons detach from a terminal and run in the background.

## Page-to-Lab Map

| Printed pages | Main idea | Lab evidence |
|---|---|---|
| 137–140 | Program, process, thread, PID, hierarchy | `01_process_identity`, `process_tree`, `pid_runtime_context` |
| 140–144 | The exec family, arguments, environment, descriptor inheritance | `04_fork_exec_wait`, `05_exec_arguments_environment`, `06_close_on_exec` |
| 145–148 | `fork()`, copy-on-write, and `vfork()` | `02_fork_return_values`, `03_copy_on_write`, `cow_page_faults` |
| 148–151 | `exit()`, `_exit()`, `atexit()`, and `SIGCHLD` | `07_exit_and__exit`, `08_atexit_order`, `stdio_fork_duplication` |
| 151–160 | Zombies, `wait()`, `waitpid()`, `waitid()`, and `system()` | exercises 09–13, 17, 19; `zombie_lifecycle` |
| 160–167 | Orphans, users, groups, and credentials | `subreaper_adoption`, `14_process_credentials` |
| 167–172 | Sessions and process groups | `15_process_group`, `16_new_session` |
| 172–175 | Daemons and `daemon()` | `18_daemon_session_setup` |

## 1. Program, Process, and Thread

These words are related, but they are not interchangeable.

### Program

A program is passive. It is executable bytes stored in a file. Running the same
program three times creates three different process instances.

### Process

A process is an executing instance with resources and identity. It includes:

- an address space containing code, data, heap, stack, and mappings;
- one or more threads;
- file descriptors;
- a current directory and root directory;
- credentials;
- signal state;
- scheduling and resource-accounting state;
- a PID and relationships to other processes.

### Thread

A thread is an execution path inside a process. Each thread has its own stack,
register state, and instruction pointer. Threads in the same process share the
address space and many resources. Chapter 7 studies threads directly; for now,
most programs use one thread per process.

### Run the identity exercise

~~~bash
./build/chapters/05-process-management/exercises/01_process_identity
~~~

Representative output:

~~~text
pid=4201 parent=3117 process_group=4201 session=3117
pid_positive=yes parent_nonnegative=yes
~~~

Your numbers will differ. The relationships matter more than the numeric
values.

## 2. PIDs Are Names with Limited Lifetimes

`getpid()` returns the caller's PID. `getppid()` returns the current parent PID.
The type is `pid_t`, not necessarily plain `int`, so the programs cast to a
sufficiently wide type before printing.

A PID is unique only among the processes visible in the relevant PID namespace
at that moment. After a process is reaped, the kernel can reuse its number.
Therefore:

- a PID is not a permanent identity;
- a stale PID can eventually name a different process;
- host and container views may show different numbers for one task;
- modern code sometimes uses Linux pidfds when race-free process references are
  required.

The book describes a historical default PID ceiling of 32768. Do not hard-code
that value. Read the current system:

~~~bash
cat /proc/sys/kernel/pid_max
./build/chapters/05-process-management/experiments/pid_runtime_context
~~~

Representative modern output may resemble:

~~~text
self_pid=4205 pid_max=4194304 pid1_comm=systemd namespace_pid_levels=1
runtime_values_used=yes
~~~

Inside a container, `pid1_comm` might be an application, shell, or supervisor,
and `namespace_pid_levels` may be greater than one. That is real Linux behavior,
not a lab error.

## 3. The Process Hierarchy

With the important exception of the initial process in a PID namespace, a
process has a parent. Calling `fork()` creates a child and records that
relationship.

Run:

~~~bash
./build/chapters/05-process-management/experiments/process_tree
~~~

Expected relationship summary:

~~~text
child_parent_matches=yes grandchild_parent_matches=yes hierarchy_depth=2
~~~

The experiment deliberately has only the original parent print. If every
process printed independently, scheduling could reorder the lines. Correct
concurrent programs must not mistake one observed order for an API guarantee.

## 4. exec Replaces; It Does Not Create

The exec family loads a new program into the calling process. On success:

- the PID remains the same;
- the process does not return to the old code;
- the old address space is replaced;
- new `argv` and environment arrays are installed;
- many descriptors remain open;
- descriptors marked close-on-exec are closed;
- caught signal dispositions are reset;
- C-library state such as buffered streams and `atexit()` functions disappears.

### Reading the exec names

| Suffix | Meaning |
|---|---|
| `l` | arguments supplied as a variadic list |
| `v` | arguments supplied as a vector (`argv[]`) |
| `p` | search the `PATH` environment variable |
| `e` | caller supplies a new environment vector |

The kernel system call is `execve()`. Other family members are C-library
interfaces that prepare arguments, search `PATH` when requested, and invoke the
underlying operation.

### A successful exec never returns

This pattern is wrong in its reasoning:

~~~text
call exec
continue normal child work
~~~

If execution reaches the line after `exec`, the exec failed. In a newly forked
child, report the error carefully if needed and call `_exit()`. The lab uses
status 127 for an exec failure, matching a common shell convention.

### Run the full lifecycle

~~~bash
./build/chapters/05-process-management/exercises/04_fork_exec_wait
~~~

Expected output:

~~~text
output=hello-from-exec exited=yes status=0
~~~

What happened:

1. the parent created a pipe;
2. `fork()` created the child;
3. the child connected stdout to the pipe using `dup2()`;
4. `execl()` replaced the child with `/bin/echo`;
5. the new program inherited stdout and wrote into the pipe;
6. the parent read the bytes and reaped the child.

This is the foundation of shell redirection and compiler drivers that launch
assemblers and linkers.

## 5. Arguments and Environment

The new program begins with an argument vector and environment. `argv[0]` is
conventionally the program name, but the caller supplies it; the kernel does not
derive it for you.

Run:

~~~bash
./build/chapters/05-process-management/exercises/05_exec_arguments_environment
~~~

Expected output:

~~~text
argc=3 arg1=alpha arg2=beta env=execve same_pid=yes
~~~

The program execs `/proc/self/exe`, a Linux link to its own executable. It passes
a controlled environment containing `LAB_MODE=execve` and the pre-exec PID. The
second instance proves that the PID is unchanged.

Common mistakes:

- forgetting the final null pointer in an argument vector;
- passing an invalid pointer or a pointer to expired storage;
- expecting the old environment to remain when using `execve()` with a custom
  environment;
- treating `argv[0]` as trustworthy security information;
- using a `PATH`-searching exec from a privileged program.

## 6. File Descriptors Across exec

By default, an open descriptor survives exec. This is a feature: the child can
inherit a pipe, socket, redirected standard stream, or pre-opened service
resource.

It is also a risk. An unintended inherited descriptor may:

- keep a file or pipe open;
- prevent another process from observing end-of-file;
- expose sensitive data or authority;
- keep locks or resources alive unexpectedly.

`FD_CLOEXEC` closes a descriptor during a successful exec. Prefer atomic
creation flags such as `O_CLOEXEC`, `pipe2(O_CLOEXEC)`, or equivalent APIs when
available. Opening first and setting the flag later creates a race if another
thread can fork and exec between those operations.

Run:

~~~bash
./build/chapters/05-process-management/exercises/06_close_on_exec
~~~

Expected output:

~~~text
ordinary_inherited=yes cloexec_closed=yes
~~~

## 7. fork Creates Two Return Paths

`fork()` returns once in each process:

| Return | Meaning |
|---:|---|
| `-1` | failure; no child created and `errno` explains why |
| `0` | executing in the child |
| positive PID | executing in the parent; value identifies the child |

Run:

~~~bash
./build/chapters/05-process-management/exercises/02_fork_return_values
~~~

Representative output:

~~~text
parent_fork_result=4310 child_fork_result=0
child_pid_matches=yes child_parent_matches=yes child_exit=0
~~~

Never assume whether parent or child executes first. If ordering matters, build
it with a pipe, wait, semaphore, or another synchronization mechanism.

### fork and multithreaded programs

After a multithreaded process forks, only the calling thread exists in the
child. Locks held by vanished threads may remain locked in copied memory.
Between fork and exec, the child should call only async-signal-safe operations.
`pthread_atfork()` can help coordinate library state, and `posix_spawn()` is a
useful higher-level alternative for many launchers.

The lab's fork examples are single-threaded so this rule is explicit rather
than accidentally violated.

## 8. Copy-on-Write

Copying an entire address space just before exec would waste time and memory.
Linux instead uses copy-on-write (COW):

1. parent and child initially map the same physical pages;
2. the mappings do not allow an unnoticed shared write;
3. a write triggers a page fault;
4. the kernel supplies a private writable page to the writer;
5. the other process continues seeing its original bytes.

Run the value demonstration:

~~~bash
./build/chapters/05-process-management/exercises/03_copy_on_write
~~~

Expected output:

~~~text
parent_value=41 child_before=41 child_after=99
same_virtual_address=yes independent_values=yes
~~~

The same virtual address is meaningful only inside each process's virtual
address space. It does not prove ongoing shared physical storage.

Now observe the cost:

~~~bash
./build/chapters/05-process-management/experiments/cow_page_faults
~~~

Representative output:

~~~text
pages_written=128 minor_fault_delta=128 fault_delta_positive=yes
parent_first_byte=1 parent_unchanged=yes
~~~

The exact fault count can vary. The stable result is a positive minor-fault
delta and an unchanged parent buffer.

## 9. Why This Lab Does Not Run vfork()

`vfork()` historically avoided address-space copying by suspending the parent
while the child temporarily shared its address space directly. The child must
not return from the calling function, modify ordinary memory, or call arbitrary
functions; it must promptly exec or call `_exit()`.

Those constraints make small-looking changes dangerous. Copy-on-write greatly
reduced the original motivation, and modern libraries may use optimized process
creation internally. Reading the interface is valuable. Encouraging manual
experimentation with fragile undefined behavior is not.

Use `fork()` for these exercises. Evaluate `posix_spawn()` when building a
production launcher.

## 10. Process Termination

A process can terminate by:

- returning from `main()`;
- calling `exit()`;
- calling `_Exit()` or `_exit()`;
- receiving a terminating signal;
- triggering a fatal fault or kernel action.

### exit() versus _exit()

`exit()` performs normal C-runtime cleanup. It runs `atexit()` handlers in
reverse registration order and flushes open standard I/O streams.

`_exit()` skips that user-space cleanup and enters the termination path. It is
the correct escape from a post-fork child when exec fails.

Run:

~~~bash
./build/chapters/05-process-management/exercises/07_exit_and__exit
~~~

Expected output:

~~~text
exit_bytes=13 _exit_bytes=0
exit_flushed=yes _exit_skipped_stdio=yes
~~~

The zero-byte file was opened successfully. Its data stayed in the child's
user-space stdio buffer and was deliberately abandoned by `_exit()`.

### The duplicated-buffer trap

If data is buffered before `fork()`, both processes receive a copy of that
user-space buffer. Normal cleanup in both processes can write the same text
twice.

~~~bash
./build/chapters/05-process-management/experiments/stdio_fork_duplication
cat build/chapters/05-process-management/data/stdio_fork_duplication.txt
~~~

Expected summary and file contents:

~~~text
buffered_before_fork=yes copies_in_file=2 duplicated=yes
copied-buffer
copied-buffer
~~~

This connects Chapter 3 buffering directly to Chapter 5 process creation.

### atexit()

~~~bash
./build/chapters/05-process-management/exercises/08_atexit_order
~~~

Expected output:

~~~text
main=returning
handler=third
handler=second
handler=first
~~~

Handlers run only on normal termination. They do not survive a successful exec
and do not run after `_exit()` or most fatal signal terminations. `on_exit()` is
a nonportable GNU/glibc interface; prefer standard `atexit()` for portable code.

## 11. SIGCHLD, Zombies, and Reaping

When a child changes state, the parent can receive `SIGCHLD`. Signal handling is
covered deeply in Chapter 10, so Chapter 5 focuses on wait functions.

A terminated child becomes a zombie when its parent has not yet collected the
status. A zombie:

- is no longer executing;
- does not consume ordinary user memory or CPU time;
- does retain a process-table entry and termination information;
- disappears after an appropriate wait consumes its status.

Run:

~~~bash
./build/chapters/05-process-management/experiments/zombie_lifecycle
~~~

Typical output on a normal WSL procfs mount:

~~~text
zombie_observed=yes exit_status=17 proc_entry_removed=yes
~~~

Some container setups mount `/proc` from a different PID namespace, so the
program may report `zombie_observed=no` while still proving successful reaping
and removal. That limitation is itself namespace evidence.

## 12. Understanding the Wait Status

The integer written by `wait()` or `waitpid()` is encoded. It is not directly
the child's exit code.

Use the category test first:

| Test | Only then read | Meaning |
|---|---|---|
| `WIFEXITED(status)` | `WEXITSTATUS(status)` | normal exit |
| `WIFSIGNALED(status)` | `WTERMSIG(status)` | killed by signal |
| `WIFSTOPPED(status)` | `WSTOPSIG(status)` | stopped child |
| `WIFCONTINUED(status)` | no numeric accessor needed | resumed child |

`WCOREDUMP(status)` exists on Linux and several Unix systems but is not POSIX;
guard it with `#ifdef WCOREDUMP` in portable source.

### Normal exit

~~~bash
./build/chapters/05-process-management/exercises/09_wait_exit_status
~~~

Representative output:

~~~text
child=4401 exited=yes exit_status=42
~~~

Only the low eight bits of a normal exit value are available. Use exit codes in
the range 0–255 and document their meaning.

### Signal termination

~~~bash
./build/chapters/05-process-management/exercises/10_wait_signal_status
~~~

Expected output:

~~~text
signaled=yes signal_matches_SIGTERM=yes normal_exit=no
~~~

A child killed by signal 15 is not the same kernel wait state as a child that
calls `exit(143)`, even though a shell may present `128 + 15` for convenience.

## 13. wait(), waitpid(), and waitid()

### wait()

`wait(&status)` blocks until any child is waitable. Conceptually, it resembles
`waitpid(-1, &status, 0)`.

### waitpid()

The PID selector changes the target:

| `pid` argument | Selection |
|---:|---|
| greater than 0 | that exact child |
| `-1` | any child |
| `0` | any child in the caller's process group |
| less than `-1` | any child in process group `abs(pid)` |

Useful options include:

- `WNOHANG`: return zero if matching children exist but none is ready;
- `WUNTRACED`: also report stopped children;
- `WCONTINUED`: also report children resumed by `SIGCONT`.

Run the exact-child example:

~~~bash
./build/chapters/05-process-management/exercises/11_waitpid_specific_child
~~~

Expected output:

~~~text
first_status=22 second_status=11 specific_order=yes
~~~

The program controls one child with a pipe. The chosen reap order comes from
explicit PIDs, not scheduling luck.

Run nonblocking polling:

~~~bash
./build/chapters/05-process-management/exercises/12_waitpid_nohang
~~~

Expected output:

~~~text
initial_running=yes final_exit=7
~~~

A zero return from `waitpid(..., WNOHANG)` means “no matching state change is
ready.” It is not an error.

### waitid()

`waitid()` returns details through `siginfo_t`. `WNOWAIT` is especially useful
when one component needs to observe status while another remains responsible
for consuming it.

~~~bash
./build/chapters/05-process-management/exercises/13_waitid_wnowait
~~~

Expected output:

~~~text
observed_pid_matches=yes observed_status=33 reaped_same=yes
~~~

Do not forget the second wait. `WNOWAIT` deliberately leaves the child waitable.

### Reap all children

~~~bash
./build/chapters/05-process-management/exercises/19_reap_all_children
~~~

Expected output:

~~~text
created=3 reaped=3 status_sum=33 all_collected=yes
~~~

A supervisor usually loops because several children can change state before one
`SIGCHLD` notification is handled. Never assume one signal means exactly one
wait call.

## 14. system() and Shell Interpretation

`system(command)` is convenient because it asks a shell to interpret a command
string and waits for completion. That convenience includes shell syntax:

- separators such as `;`;
- pipelines;
- redirections;
- variable and command expansion;
- wildcard expansion;
- behavior influenced by the environment.

If untrusted text enters the command string, it may become syntax instead of
data. Privileged programs must be especially careful.

The safe-command exercise sends a semicolon as one literal argument:

~~~bash
./build/chapters/05-process-management/exercises/17_safe_command_runner
~~~

Expected output:

~~~text
output=hello; echo injected shell_interpreted=no status=0
~~~

No shell runs. `/usr/bin/printf` receives exactly one data argument. When a
shell is genuinely required, keep the command fixed or apply a design that does
not splice untrusted input into shell syntax.

## 15. Orphans, PID 1, and Subreapers

The historical explanation says that when a parent terminates, its children are
adopted by init, PID 1, which later reaps them.

Modern Linux requires a more precise explanation:

1. PID namespaces give each namespace its own PID 1;
2. `PR_SET_CHILD_SUBREAPER` lets a supervisor become the adoption point for
   orphaned descendants beneath it;
3. the nearest living subreaper ancestor is preferred before namespace PID 1.

Run:

~~~bash
./build/chapters/05-process-management/experiments/subreaper_adoption
~~~

Typical output:

~~~text
subreaper_supported=yes adopted_by_subreaper=yes proc_parent_matches=yes grandchild_exit=17
~~~

`adopted_by_subreaper=yes` is proven because the subreaper successfully waits
for the grandchild. `proc_parent_matches` can be `no` in an unusual procfs/PID
namespace mount arrangement even though the wait relationship is correct.

## 16. Process Credentials

Linux keeps several identity values because “who launched me?” and “whose
permissions am I currently using?” are different questions.

| Identity | Simplified role |
|---|---|
| Real UID/GID | original user and primary group identity |
| Effective UID/GID | identity used for many permission checks |
| Saved IDs | controlled return point for privilege transitions |
| Supplementary groups | additional group memberships |
| Filesystem UID/GID | Linux-specific historical filesystem check identity |
| Capabilities | individual privilege units traditionally bundled into root |

Run the safe inspection exercise:

~~~bash
./build/chapters/05-process-management/exercises/14_process_credentials
~~~

Representative unprivileged output:

~~~text
uid=1000 euid=1000 gid=1000 egid=1000 supplementary_groups=7
same_user_identity=yes same_group_identity=yes
~~~

Do not run a learning program as root merely to make `setuid()` succeed.
Credential transitions are security decisions. A robust privileged program
must also consider supplementary groups, capabilities, environment variables,
descriptors, filesystem state, namespaces, and whether privilege can be
regained.

## 17. Process Groups and Sessions

User groups and process groups are unrelated concepts.

A process group is a set of processes addressed together for job control. A
shell usually places every process in one pipeline into one group. Signals such
as terminal interrupt can then target the foreground group.

A session is a collection of process groups, often associated with one login
and a controlling terminal. Within a session, one process group is foreground
and others may be background.

### New process group

~~~bash
./build/chapters/05-process-management/exercises/15_process_group
~~~

Expected output:

~~~text
child_group_equals_child=yes parent_group_unchanged=yes
~~~

The parent uses `setpgid(child, child)`. A pipe prevents the child from checking
its group before the parent completes that operation.

### New session

~~~bash
./build/chapters/05-process-management/exercises/16_new_session
~~~

Expected output:

~~~text
pid_equals_pgid=yes pid_equals_sid=yes session_created=yes
~~~

`setsid()` creates a new session only if the caller is not already a process-
group leader. Forking first is the usual way to satisfy that condition. The
caller becomes both session leader and process-group leader and has no
controlling terminal.

## 18. Daemons Then and Now

A traditional daemon runs without a controlling terminal and normally lives in
the background for a long time. Classic setup includes:

1. fork and let the original parent exit;
2. call `setsid()`;
3. sometimes fork again to prevent later controlling-terminal acquisition;
4. choose a safe working directory;
5. set a deliberate umask;
6. close unintended descriptors;
7. connect descriptors 0, 1, and 2 to appropriate targets;
8. establish logging, signal handling, and a shutdown policy.

Run the bounded exercise:

~~~bash
./build/chapters/05-process-management/exercises/18_daemon_session_setup
~~~

Expected output:

~~~text
session_leader=yes process_group_leader=yes cwd_root=yes
standard_streams_redirected=yes child_exit=0
~~~

The child exits immediately and the parent reaps it. The program does not leave
an unmanaged background process behind.

On a modern systemd-managed machine, a service generally should not perform the
classic backgrounding dance. It can remain in the foreground while the service
manager supplies logging, restart policy, credentials, resource limits,
working-directory configuration, socket activation, and lifecycle tracking.
Learn the historical mechanism because it explains Unix process relationships;
choose deployment behavior that matches the actual supervisor.

## 19. Resource Usage at Reap Time

Linux provides `wait4()`, which combines status collection with a `struct
rusage` snapshot for the selected child.

~~~bash
./build/chapters/05-process-management/experiments/wait4_usage
~~~

Representative output:

~~~text
exit_status=9 user_us=4211 system_us=0 minor_faults=11
usage_collected=yes
~~~

Timing and fault counts vary. Use them as measurements, not constants. `wait4()`
is not the core portable POSIX interface, so decide whether its per-child data
justifies the portability cost.

## 20. Trace the Kernel Boundary

The chapter helper uses `strace -f`, because child syscalls would otherwise be
missed.

Trace the fork/exec/wait lifecycle:

~~~bash
chapters/05-process-management/scripts/trace.sh 04_fork_exec_wait
grep -E 'clone|execve|wait4|exit_group' \
  chapters/05-process-management/observations/strace/04_fork_exec_wait.strace
~~~

Important observations:

- glibc may implement `fork()` through `clone()` or `clone3()`;
- `execve()` appears in the child with the same trace PID before and after;
- the parent blocks in a wait operation;
- normal C `return` eventually reaches `exit_group()`.

Trace close-on-exec:

~~~bash
chapters/05-process-management/scripts/trace.sh 06_close_on_exec
grep -E 'openat|fcntl|execve|close' \
  chapters/05-process-management/observations/strace/06_close_on_exec.strace
~~~

Trace the modern orphan model:

~~~bash
chapters/05-process-management/scripts/trace.sh subreaper_adoption
grep -E 'prctl|clone|wait4' \
  chapters/05-process-management/observations/strace/subreaper_adoption.strace
~~~

The exact trace includes dynamic-loader activity. Filter for the calls that
answer your question instead of treating every line as equally important.

## 21. Common Mistakes and Corrections

### Mistake: saying exec creates a process

Correction: `fork()` creates; `exec()` replaces. The PID survives exec.

### Mistake: assuming the parent runs first

Correction: scheduling order is unspecified. Synchronize any required order.

### Mistake: calling exit() after exec fails in a child

Correction: use `_exit()` so the child does not flush inherited stdio buffers or
run copied parent cleanup handlers.

### Mistake: printing from every process to prove ordering

Correction: interleaved stdout is not reliable evidence. Send structured
records to one printing process.

### Mistake: reading WEXITSTATUS unconditionally

Correction: call it only if `WIFEXITED(status)` is true.

### Mistake: treating waitpid WNOHANG return zero as failure

Correction: zero means matching children exist but none has a reportable change.

### Mistake: waiting once after creating many children

Correction: loop until all expected children are reaped or until `ECHILD` in a
general reaper.

### Mistake: forgetting that stdio buffers are copied by fork

Correction: flush intentionally before fork or use `_exit()` in a failed child
launch path.

### Mistake: leaking descriptors into executed programs

Correction: define inheritance intentionally and use atomic close-on-exec flags.

### Mistake: assuming the book's PID maximum is universal

Correction: inspect `/proc/sys/kernel/pid_max` on the running system.

### Mistake: assuming every orphan goes directly to the host init process

Correction: account for PID namespaces and Linux child subreapers.

### Mistake: using system() with untrusted text

Correction: invoke an explicit executable and pass untrusted values as argument
elements, not shell source code.

### Mistake: experimenting with credential changes as root

Correction: inspect identities in this lab. Study privilege transitions in a
dedicated, isolated security exercise with a complete threat model.

### Mistake: always self-daemonizing

Correction: follow the contract of the deployment environment. Foreground
operation is usually correct under a modern service manager.

## 22. Guided Exercises for You

Complete these in order. Make one change at a time and keep the original tests
passing.

### Exercise A — Prove the PID survives exec

The existing `05_exec_arguments_environment.c` already reports `same_pid=yes`.
Add a second environment entry containing the pre-exec parent PID. In child
mode, verify that both PID relationships match your prediction.

Questions:

1. Which identity stayed constant?
2. Which memory containing the original local variables survived?
3. Why was an environment string available after the replacement?

### Exercise B — Trigger a controlled exec failure

Copy `04_fork_exec_wait.c` to a new exercise and exec a path that does not
exist. Send the saved `errno` value to the parent through a dedicated pipe, then
call `_exit(127)`.

Before running, predict:

- whether exec returns;
- which program image continues;
- what the parent sees in the wait status.

### Exercise C — Compare descriptor offsets after fork

Open a data file before `fork()`. Let the child read four bytes and report them.
After waiting, let the parent read the next four bytes from its descriptor.

Explain why distinct descriptor-table entries in two processes can share one
open-file offset.

### Exercise D — Make WNOWAIT visible

Extend `13_waitid_wnowait.c` to inspect `/proc/CHILD/stat` after `waitid()` and
before `waitpid()`. Record the state. Then confirm that `/proc/CHILD` disappears
after the final reap.

Be prepared for a container whose procfs mount hides the child; document that
environment instead of forcing the expected result.

### Exercise E — Reap children in a different order

Modify `11_waitpid_specific_child.c` so both children wait on independent gate
pipes. Release them in one order and reap them in the opposite order.

Explain the difference between termination order and collection order.

### Exercise F — Build a tiny compiler-driver launcher

Write a program that launches `/usr/bin/cc --version` or the compiler path
provided as its first argument. Capture stdout and stderr with pipes, wait for
the child, and report a decoded status.

Rules:

- no `system()`;
- no shell command string;
- set close-on-exec on every descriptor the compiler should not inherit;
- cap captured output so a child cannot exhaust parent memory.

### Exercise G — Process-group signal preparation

Create two children in one new process group and verify both group IDs. Do not
send a signal yet; Chapter 10 will add safe signal handling. Explain how one
negative PID passed to `kill()` can later address the group.

### Exercise H — Supervisor design on paper

Design a supervisor for three worker processes. Specify:

- which descriptors each child inherits;
- what happens when exec fails;
- how all child statuses are reaped;
- whether workers share a process group;
- when a failed worker restarts;
- how shutdown avoids zombies;
- whether systemd already provides any of these responsibilities.

## 23. Review Questions

Answer aloud before checking the guide:

1. What is the difference between a program and a process?
2. Why can one program have many PIDs over time?
3. What are the three classes of `fork()` return value?
4. What does copy-on-write postpone?
5. Why does a successful exec not return?
6. Which important process identity does exec preserve?
7. How can a descriptor be intentionally removed during exec?
8. Why is `_exit()` safer after a failed child exec?
9. Why does a zombie exist?
10. What must be true before reading `WEXITSTATUS`?
11. What does `waitpid()` return with `WNOHANG` when a child is still running?
12. What special promise does `WNOWAIT` make?
13. Why can a subreaper wait for a grandchild?
14. How are user groups different from process groups?
15. What relationship holds among PID, PGID, and SID after successful
    `setsid()` by a forked child?
16. Why can `system()` turn data into commands?
17. Why might a service managed by systemd remain in the foreground?
18. What changes when fork is called in a multithreaded process?

## 24. Mastery Checklist

Chapter 5 is ready for review when you can do all of the following without
copying code blindly:

- [ ] explain program, process, and thread in your own words;
- [ ] read PID, PPID, PGID, and SID from a running program;
- [ ] draw both control-flow paths after `fork()`;
- [ ] explain copy-on-write at page-fault level;
- [ ] launch a program with fork, descriptor setup, exec, and wait;
- [ ] define an explicit argument vector and environment;
- [ ] control descriptor inheritance with close-on-exec;
- [ ] choose correctly among `exit()`, `_Exit()`, and `_exit()`;
- [ ] decode normal and signal termination correctly;
- [ ] use `waitpid()` with an exact PID and `WNOHANG`;
- [ ] explain `waitid(..., WNOWAIT)`;
- [ ] reap multiple children without assuming their order;
- [ ] explain zombies, namespace PID 1, and subreapers;
- [ ] distinguish real, effective, and supplementary identities;
- [ ] create and inspect process groups and sessions;
- [ ] describe both traditional daemonization and service-manager operation;
- [ ] trace creation, exec, and waiting with `strace -f`;
- [ ] pass `make test CHAPTER=05`.

## 25. Connection to Compilers and Runtimes

Process management is not separate from compiler engineering.

A compiler driver may:

- fork and exec a preprocessor, compiler, assembler, and linker;
- connect stages with pipes;
- redirect diagnostic streams;
- prevent internal descriptors from leaking into tools;
- collect and translate exit status;
- terminate a process group when one stage fails.

A language runtime may:

- expose process-spawn APIs;
- manage environment and descriptor inheritance;
- avoid unsafe post-fork work in a multithreaded VM;
- use `posix_spawn()` to reduce fork-related hazards;
- act as a supervisor and reap child processes;
- integrate process events with an event loop.

The durable mental model is:

~~~text
fork creates an execution container
exec replaces its program
exit leaves a status
wait transfers that status to the parent
groups and sessions connect the process to job control and supervision
~~~
