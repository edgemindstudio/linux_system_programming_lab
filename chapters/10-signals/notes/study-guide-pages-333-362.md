# Chapter 10 Study Guide - Signals

Book: Robert Love, *Linux System Programming*, second edition

Scope: Chapter 10, printed pages 333-362

Repository: `linux_system_programming_lab`

This guide is an original explanation and laboratory companion. It summarizes
the chapter's ideas in new language and connects them to runnable C programs;
it does not reproduce the book.

## How to Use This Chapter

Signals are easy to demonstrate incorrectly. A program can appear to work
while containing a lost-wakeup race, an unsafe handler call, or an assumption
about scheduling order. For each exercise:

1. identify who generates the signal;
2. identify the target process or process group;
3. state the disposition before generation;
4. state whether the signal is blocked;
5. predict whether it becomes pending or is delivered immediately;
6. list every operation performed by the handler;
7. decide whether the interrupted operation can report `EINTR`;
8. run and trace the program;
9. separate guaranteed behavior from one observed ordering.

Build and test from the repository root:

~~~bash
make chapter CHAPTER=10
make test CHAPTER=10
make check-structure
~~~

Sources and executables live in matching trees:

~~~text
chapters/10-signals/exercises/
chapters/10-signals/experiments/

build/chapters/10-signals/exercises/
build/chapters/10-signals/experiments/
~~~

## Chapter Summary

A signal is a kernel-mediated asynchronous notification. It can originate
from a terminal action, a kernel-detected condition, a timer, a child state
change, another process, or the receiving process itself. The notification is
identified by a signal number and may include limited metadata.

The lifecycle has three separate stages:

1. **generation:** an event or sender creates the signal;
2. **pending state:** the kernel records a signal that cannot yet be delivered;
3. **delivery:** the kernel applies the target's disposition when an eligible
   execution context can receive it.

The process chooses one of three broad dispositions where the signal permits
that choice:

- retain the default action;
- ignore the signal;
- catch it with a handler.

Signals interrupt ordinary execution, so a handler runs under severe
constraints. It cannot safely call arbitrary library functions or manipulate
complex program state. The central engineering pattern is to let the handler
record a minimal fact and let normal code perform the real work.

## 1. Signal Concepts

### 1.1 Software interrupt as a mental model

The phrase "software interrupt" is useful because delivery can divert control
at a point the application did not explicitly call into a handler. It is not a
perfect hardware description; the kernel arranges user-space handler execution
as part of returning to the process.

The important consequence is that normal code may be interrupted between
operations. A handler cannot assume that libc, the allocator, a stream, or an
application invariant is in a stable state.

### 1.2 Generation is not delivery

Suppose SIGUSR1 is blocked:

~~~text
sender calls kill()
        |
        v
kernel marks SIGUSR1 pending
        |
        v
target continues without running handler
        |
target unblocks SIGUSR1
        |
        v
kernel may deliver SIGUSR1
~~~

The send operation can succeed even though the target's handler has not run.
Likewise, observing a pending signal proves generation but not the number of
ordinary-signal occurrences that led to the pending bit.

### 1.3 Default, ignored, and caught

`SIG_DFL` means the kernel performs the documented default action. Depending
on the signal, that action can:

- terminate;
- terminate and request a core dump;
- stop the process;
- continue a stopped process;
- ignore the notification.

`SIG_IGN` asks the kernel to ignore the signal. A handler address asks the
kernel to run application code on delivery.

`SIGKILL` and `SIGSTOP` cannot be caught, blocked, or ignored. That is a
deliberate control boundary, not an API omission.

## 2. Signal Identifiers and Meanings

### 2.1 Use names, not numbers

Programs include `<signal.h>` and use names such as `SIGINT`, `SIGTERM`, and
`SIGCHLD`. The numerical mapping can vary by platform. Signal zero is not an
ordinary signal; `kill()` assigns it probe semantics.

Inspect the local platform with:

~~~bash
kill -l
man 7 signal
~~~

The shell command is useful for observation, but source code should continue
to use symbolic constants.

### 2.2 Operational categories

It is easier to remember signals by reason than by number:

| Category | Examples | Typical meaning |
|---|---|---|
| terminal/job control | `SIGINT`, `SIGQUIT`, `SIGTSTP`, `SIGCONT` | interactive control |
| termination | `SIGTERM`, `SIGKILL`, `SIGHUP` | shutdown or loss of terminal |
| child lifecycle | `SIGCHLD` | child stopped, continued, or exited |
| programming faults | `SIGSEGV`, `SIGBUS`, `SIGILL`, `SIGFPE` | invalid execution condition |
| IPC/application | `SIGUSR1`, `SIGUSR2` | application-defined notification |
| I/O/resource events | `SIGPIPE`, `SIGXCPU`, `SIGXFSZ` | exceptional operating condition |
| time/profiling | `SIGALRM`, `SIGVTALRM`, `SIGPROF` | timer expiration |

The table is a map, not permission to recover from every fault. Continuing
after synchronous faults such as an invalid memory access is generally unsafe
unless a very carefully designed runtime owns the entire recovery mechanism.

### 2.3 SIGTERM versus SIGKILL

`SIGTERM` is the normal request for termination. A process can catch it, set a
shutdown flag, stop accepting work, flush durable state from normal code, and
exit cleanly.

`SIGKILL` is unconditional. No handler runs, so the application cannot perform
cleanup. Operational tools should normally try `SIGTERM`, wait according to a
policy, and escalate only if required.

## 3. Installing a Disposition with sigaction()

Prefer `sigaction()` to the historical `signal()` interface:

~~~c
struct sigaction action = {0};
action.sa_handler = handle_signal;
sigemptyset(&action.sa_mask);
action.sa_flags = 0;

if (sigaction(SIGUSR1, &action, NULL) == -1) {
    /* handle error */
}
~~~

The structure describes:

- the one-argument handler or `SA_SIGINFO` handler;
- additional signals blocked while the handler runs;
- flags that modify delivery behavior.

The signal currently being handled is normally added to the handler-time mask
automatically. `sa_mask` extends that protection to other signals. This can
protect a tiny handler protocol, but it does not make arbitrary unsafe code
safe.

Exercise 10.04 generates SIGUSR2 inside a SIGUSR1 handler while SIGUSR2 is in
the first handler's mask. The expected sequence is:

~~~text
enter SIGUSR1 handler
generate SIGUSR2
observe SIGUSR2 pending
return from SIGUSR1 handler
deliver SIGUSR2
return to main
~~~

## 4. Waiting for Any Caught Signal

`pause()` suspends until a caught signal causes a handler to run. After the
handler returns, `pause()` returns `-1` and sets `errno` to `EINTR`.

~~~c
errno = 0;
int result = pause();
if (result == -1 && errno == EINTR) {
    /* a caught signal interrupted the wait */
}
~~~

This interface alone does not solve the check-then-sleep race:

~~~text
if (!ready)        signal can arrive here
    pause();       and this can then sleep indefinitely
~~~

Use `sigsuspend()` when a state check and the transition into waiting must be
atomic with respect to signal delivery.

## 5. Fork, Exec, and Signal State

### 5.1 Across fork()

The child inherits dispositions and its caller's signal mask. It does not
inherit the parent's pending signals. Parent and child have distinct address
spaces after `fork()`, so a handler's write to a normal child-side flag does
not change the parent's flag.

This distinction connects Chapter 5 and Chapter 10:

~~~text
parent before fork
  disposition: handler
  mask: SIGUSR2 blocked
  pending: SIGUSR1

child after fork
  disposition: handler        inherited
  mask: SIGUSR2 blocked       inherited
  pending: empty              not inherited
~~~

### 5.2 Across exec()

An ignored disposition survives `exec()`. A caught disposition resets to
default because the handler address referred to code in the replaced image.
The signal mask survives.

This creates a real launcher responsibility. A shell, supervisor, or compiler
driver that blocks signals temporarily must restore the intended mask before
executing another program, unless the inherited mask is an explicit part of
the interface.

## 6. Descriptions Are for People

`strsignal()` maps a signal number to a readable description. The exact string
can depend on locale and libc. It belongs in diagnostics, not protocol logic.

Correct reasoning:

~~~text
signal identity -> compare against SIGTERM
display text     -> obtain from strsignal(SIGTERM)
~~~

Incorrect reasoning parses an English description to determine behavior.

## 7. Sending Signals

### 7.1 kill()

Despite its name, `kill()` sends any signal:

~~~c
int kill(pid_t target, int signal_number);
~~~

Target interpretation matters:

| Target | Meaning |
|---|---|
| positive PID | one process |
| zero | caller's process group |
| negative process-group ID | every permitted process in that group |
| `-1` | broad permitted set with defined exclusions |

The receiver's disposition determines the outcome. `kill(child, SIGUSR1)` does
not mean "kill the child" when SIGUSR1 is caught or ignored.

### 7.2 Permissions

Sending is subject to identity and capability rules. Root-like capabilities
can broaden authority, while ordinary processes generally signal processes
with compatible real or saved credentials. `SIGCONT` also has session-related
rules.

Treat `EPERM` and `ESRCH` distinctly:

- `EPERM`: a target exists, but the sender lacks permission;
- `ESRCH`: the target was not found in the relevant namespace, or is already
  gone.

A PID is reusable. A successful probe is not a durable reference to one
process lifetime.

### 7.3 The null signal

`kill(pid, 0)` performs target and permission checks without delivering a
signal. It answers a narrow question at one instant. It cannot guarantee that
the process remains alive after the call returns.

### 7.4 Sending to yourself

`raise(sig)` targets the calling process. With the signal unblocked, the
handler completes before `raise()` returns successfully.

### 7.5 Process groups

Group signaling is valuable for job control and supervisors, but unsafe test
code can terminate its own shell or test harness. Exercise 10.11 first places
the child in a new group and only then uses `kill(-child_pid, SIGUSR1)`.

## 8. Reentrancy and Async-Signal Safety

### 8.1 Why ordinary functions can fail inside handlers

Imagine main is interrupted while `malloc()` updates allocator metadata. A
handler calls `malloc()` again. The second call observes an internal structure
halfway through a mutation. Possible results include deadlock, corruption, or
apparently correct behavior that later fails.

The same pattern applies to buffered stdio and many library services. A
function is **reentrant** when another invocation can safely overlap it.
POSIX separately specifies a set of **async-signal-safe** functions allowed in
handlers.

Do not infer safety merely because a function worked during a test.

### 8.2 Minimal flag pattern

~~~c
static volatile sig_atomic_t stop_requested;

static void handle_term(int signal_number)
{
    if (signal_number == SIGTERM) {
        stop_requested = 1;
    }
}
~~~

Main checks the flag and performs complex shutdown normally. The flag carries
only a small state transition; it is not a general synchronization primitive.

### 8.3 Preserve errno when necessary

A handler that calls an async-signal-safe function may still change `errno`.
If interrupted code relies on its current value, save and restore it:

~~~c
static void handler(int signal_number)
{
    int saved_errno = errno;
    /* async-signal-safe work */
    errno = saved_errno;
}
~~~

The lab's simplest handlers avoid needing `errno`, but production libraries
often preserve it defensively.

## 9. Signal Sets and Masks

`sigset_t` is opaque. Initialize and modify it with its API:

~~~c
sigset_t set;
sigemptyset(&set);
sigaddset(&set, SIGUSR1);
sigaddset(&set, SIGTERM);
~~~

Important functions include:

- `sigemptyset()` - start with no members;
- `sigfillset()` - start with the supported signal universe;
- `sigaddset()` - add one member;
- `sigdelset()` - remove one member;
- `sigismember()` - query membership.

Creating a set does not change process behavior. `sigprocmask()` applies it to
the calling thread's mask in this single-threaded lab:

~~~c
sigprocmask(SIG_BLOCK, &set, &previous);
/* protected work */
sigprocmask(SIG_SETMASK, &previous, NULL);
~~~

Saving and restoring the previous mask composes correctly with callers that
already blocked other signals.

In multithreaded programs, use `pthread_sigmask()` and design which thread will
accept process-directed signals.

## 10. Pending Signals

`sigpending()` returns signals that are both generated and currently blocked:

~~~c
sigset_t pending;
sigpending(&pending);
if (sigismember(&pending, SIGUSR1) == 1) {
    /* SIGUSR1 is pending */
}
~~~

For standard signals, one pending bit does not count occurrences. If SIGUSR1
is generated eight times while blocked, Linux can deliver it once when it is
unblocked. This is coalescing, not packet loss relative to the API contract.

If every event must be preserved, use a queue, pipe, eventfd, socket, shared
memory protocol, or a carefully justified real-time signal design.

## 11. Waiting for a Signal Set

### 11.1 sigsuspend()

The robust asynchronous pattern is:

1. block the target signal;
2. prepare the handler and shared state;
3. inspect the state while delivery remains blocked;
4. call `sigsuspend()` with a temporary mask that unblocks the signal;
5. recheck the condition after every return;
6. restore the original mask.

The loop matters because another caught signal can also wake the call.

### 11.2 sigwait()

`sigwait()` is synchronous. The signal must be blocked first. The function
removes one matching pending signal and returns its number through an output
parameter.

Like Pthreads functions, `sigwait()` returns an error number directly. Do not
write code that only checks for `-1` and reads `errno`.

Synchronous acceptance can be much easier to integrate into a runtime than
arbitrary asynchronous handlers.

## 12. Interrupted System Calls

A handler can interrupt a blocking operation. Without restart behavior, a
read may return:

~~~text
result = -1
errno  = EINTR
~~~

The program decides whether to retry, stop, recompute a timeout, or process a
shutdown request. Blind retries can be wrong when the signal communicates
cancellation.

`SA_RESTART` asks the kernel/libc boundary to restart selected interfaces.
It is not universal. The two experiments keep all other conditions the same:

- `interrupted_read_eintr` observes an application-visible `EINTR`;
- `restarted_read` observes the handler and then a successful `read()`.

Trace both and compare the syscall return path.

## 13. Advanced Information with SA_SIGINFO

Set `SA_SIGINFO` and use the three-argument handler member:

~~~c
static void handler(int sig, siginfo_t *info, void *context);
~~~

The `siginfo_t` object can describe:

- the signal number;
- the sender PID and credentials for user-generated signals;
- the origin classification in `si_code`;
- an address associated with certain hardware faults;
- child status and resource information for `SIGCHLD`;
- timer or message-queue context;
- a value supplied by `sigqueue()`.

Fields are meaningful according to the signal and `si_code`; not every field
is valid for every event. Code must branch on the documented origin before
interpreting union members.

The context pointer exposes machine execution state through platform-specific
types. General application handlers should leave it alone unless implementing
a low-level runtime facility with explicit architecture support.

## 14. si_code and Origin

`si_code` distinguishes broad origins. Examples include a user `kill()`, a
queued user signal, timer expiration, child status, polling events, and
signal-specific hardware causes.

The same numeric field can have signal-specific interpretations. Correct code
does not build a universal switch that assumes every code has one global
meaning.

Exercise 10.17 sends SIGUSR1 with `kill()` and expects a user-origin code and a
sender PID equal to the current process. Exact numeric values are deliberately
not printed or asserted.

## 15. Sending a Payload

`sigqueue()` sends a signal plus a `union sigval`:

~~~c
union sigval value = {.sival_int = 42};
sigqueue(target, SIGUSR1, value);
~~~

An `SA_SIGINFO` handler reads `info->si_value.sival_int`. Pointer payloads are
meaningful only when sender and receiver share a valid interpretation of the
address, which generally rules them out across unrelated processes.

Even with a payload, a signal is a constrained notification channel. A pipe or
socket provides clearer framing, capacity, backpressure, and error handling
for substantial data.

## 16. Standard Signals and Real-Time Signals

Standard signals generally coalesce while pending. POSIX real-time signals are
queued, ordered, and can carry a payload. Linux exposes a runtime range from
`SIGRTMIN` through `SIGRTMAX`.

Do not hard-code the numeric endpoints because libc may reserve internal
real-time signals. Use the macros at runtime.

Queueing is bounded. `sigqueue()` can fail when the relevant resource limit is
reached. A correct design checks the result and defines what lost capacity
means for the application.

The experiments contrast the models:

~~~text
8 blocked SIGUSR1 sends   -> one pending standard signal, one delivery
3 blocked SIGRTMIN sends  -> three queued records, three synchronous receives
~~~

## 17. Event-Loop Integration

### 17.1 Self-pipe pattern

A handler performs one async-signal-safe `write()` to a prepared nonblocking
pipe. The normal event loop monitors the read end with `poll()` or `epoll()`.

Benefits:

- complex work stays out of the handler;
- signal activity joins the same readiness loop as files and sockets;
- ordinary code controls logging, allocation, and locking.

The pipe can fill. A robust design often treats a byte as "state may have
changed" rather than requiring one byte per occurrence.

### 17.2 Linux signalfd()

`signalfd()` converts selected blocked signals into structured readable
records. The discipline is essential:

1. block the selected signals;
2. create the descriptor for that same set;
3. keep the signals blocked in threads that must not receive them normally;
4. consume records through `read()` and readiness APIs.

This is elegant in Linux event loops but not portable to other POSIX systems.

## 18. Observing Linux

### 18.1 strace

Trace a focused program:

~~~bash
chapters/10-signals/scripts/trace.sh 15_sigsuspend_wait
~~~

Look for:

- `rt_sigaction` when dispositions are installed;
- `rt_sigprocmask` when masks change;
- `kill`, `tgkill`, or `rt_sigqueueinfo` when signals are generated;
- a signal-delivery marker in the trace;
- `rt_sigsuspend` or an interrupted `read()`;
- `wait4` when a child is reaped;
- `signalfd4` and a structured `read()` in the Linux-specific experiment.

The `rt_` prefix is a Linux syscall-interface detail. Application source still
uses the libc APIs such as `sigaction()` and `sigprocmask()`.

### 18.2 /proc/self/status

Linux exposes hexadecimal diagnostic masks including:

- `SigPnd` and `ShdPnd` for pending state;
- `SigBlk` for blocked signals;
- `SigIgn` for ignored signals;
- `SigCgt` for caught signals.

The experiment configures known states and verifies their bits. Use the
portable APIs for program behavior; use `/proc` for inspection and debugging.

## 19. Common Failure Modes

### Failure: printing from a handler

`printf()` can touch buffered stream state already being modified by
interrupted code. Set a flag or write to a prepared descriptor instead.

### Failure: believing volatile makes general data safe

`volatile sig_atomic_t` supports a narrow flag-like communication pattern. It
does not make structures, counters, malloc-managed objects, or multistep
invariants safe.

### Failure: checking a flag and then calling pause()

The signal can arrive between the check and the sleep. Block first and use
`sigsuspend()` for the atomic transition.

### Failure: expecting one handler call per standard-signal send

Standard signals can coalesce. The interface communicates a pending condition,
not an event count.

### Failure: assuming SA_RESTART fixes every EINTR

Restart behavior is call-specific. Design and test each blocking path.

### Failure: changing a mask without restoring it

Library or launcher code can accidentally leak blocked signals into callers or
executed programs. Save the previous mask and restore it.

### Failure: sending to the current process group in a test

`kill(0, sig)` can reach the shell and test harness. Create an isolated group
or target a specific child.

### Failure: treating PID probes as stable handles

The target can exit after `kill(pid, 0)`, and PIDs are reused. Modern Linux
supervisors may use pidfds when they require a stable process reference.

### Failure: doing shutdown inside the handler

Closing complex subsystems, joining threads, freeing memory, and flushing
stdio are not handler tasks. Record the request; let main coordinate shutdown.

## 20. Exercise Walkthrough

### Exercises 10.01-10.04: vocabulary and delivery

These establish symbolic identifiers, `sigaction()`, `pause()`, and the
handler-time mask. Explain exactly when control moves into each handler.

### Exercises 10.05-10.07: lifecycle across program transitions

Compare `fork()` inheritance with `exec()` reset rules. Then separate
machine-readable identifiers from locale-sensitive descriptions.

### Exercises 10.08-10.11: sending and targets

Probe without delivery, terminate only a controlled child, send to self, and
signal an isolated process group. These exercises emphasize target semantics
and cleanup through `waitpid()`.

### Exercises 10.12-10.16: safe state and waiting

Use a flag, construct sets, inspect pending state, remove the lost-wakeup race,
and compare asynchronous handling with synchronous `sigwait()`.

### Exercises 10.17-10.18: metadata and payloads

Use `SA_SIGINFO`, validate origin without hard-coded codes, and retrieve a
small integer payload. Keep handler work minimal even though more information
is available.

## 21. Compiler and Runtime Engineering Connections

Signals appear throughout low-level tools:

- a compiler driver forwards termination to child compiler and linker jobs;
- a test runner decodes signal-based child termination separately from exit
  codes;
- a runtime translates fatal faults into diagnostics or crash reports;
- a profiler samples execution with timer-generated signals;
- a debugger observes stop and trap signals;
- a JIT may coordinate guarded code regions and fault metadata;
- a server uses a self-pipe or `signalfd()` to integrate shutdown and reload
  requests into an event loop.

These uses demand explicit policies. Which signals are owned by the runtime?
Which dispositions can application code replace? Which thread receives them?
Which operations remain legal in the handler? How are children and process
groups cleaned up? A mature runtime documents those contracts.

## 22. Review Questions

1. What is the difference between generating, pending, and delivering a
   signal?
2. Why can SIGKILL and SIGSTOP not be caught or blocked?
3. Why should source code use `SIGTERM` rather than a remembered number?
4. What state crosses `fork()`, and what signal state does not?
5. Why does `exec()` reset caught handlers but preserve ignored signals?
6. Why is `printf()` unsafe in a handler even when it appears to work?
7. What can safely be communicated with `volatile sig_atomic_t`?
8. Why can repeated standard signals produce only one handler invocation?
9. How does a blocked signal become pending?
10. What race does `sigsuspend()` prevent?
11. How does `sigwait()` change the programming model?
12. What does `kill(pid, 0)` prove, and what does it not prove?
13. How does a negative PID change `kill()` targeting?
14. When can a blocking call report `EINTR`?
15. Why is `SA_RESTART` not a universal solution?
16. Which `siginfo_t` fields are valid for every signal?
17. What distinguishes `kill()` origin from `sigqueue()` origin?
18. Why are signal payloads unsuitable for bulk data transport?
19. How does the self-pipe pattern move work out of the handler?
20. What portability is lost by using `signalfd()`?

## 23. Mastery Checklist

Before moving to Chapter 11, be able to:

- explain the complete signal lifecycle;
- classify common default actions;
- install and inspect dispositions with `sigaction()`;
- explain fork and exec inheritance rules;
- send safely to a process, self, or controlled process group;
- construct and apply signal sets without assuming representation;
- distinguish blocking from ignoring;
- inspect pending signals;
- write a minimal async-signal-safe handler;
- explain `volatile sig_atomic_t` without overstating it;
- eliminate the check-then-sleep race with `sigsuspend()`;
- consume a signal synchronously with `sigwait()`;
- handle `EINTR` and reason about `SA_RESTART`;
- interpret `siginfo_t` only according to signal origin;
- contrast standard coalescing with real-time queueing;
- integrate signal notification with a self-pipe or Linux `signalfd()`;
- read signal evidence from `strace` and `/proc`;
- connect signal policy to supervisors, debuggers, compilers, and runtimes.

Passing the smoke test proves that the prepared demonstrations behaved as
expected on one run. Mastery requires explaining why each behavior is safe,
which behavior is portable, which is Linux-specific, and which observed order
could change on the next run.
