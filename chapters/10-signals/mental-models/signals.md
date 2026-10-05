# Chapter 10 Mental Model - Signals

## Signals Are Kernel-Mediated Notifications

A signal is a small notification identified primarily by a signal number. It
is not an arbitrary message buffer. One process, the kernel, or the same
process generates a signal; the kernel records it; and delivery occurs when the
target is eligible to receive it.

~~~text
event or sender
      |
      v
signal generated
      |
      v
kernel records pending state
      |
      +---- target blocks signal ----> remains pending
      |
      v
kernel delivers signal
      |
      +---- SIG_IGN ----> discard
      +---- SIG_DFL ----> default action
      +---- handler ----> interrupt normal flow, run handler, resume
~~~

Generation, pending state, and delivery are separate events. A program that
uses those words interchangeably will misreason about blocking and races.

## Per-Process Disposition, Per-Thread Mask

| Property | Meaning | Scope in a multithreaded process |
|---|---|---|
| Disposition | default, ignored, or caught | shared by the process |
| Signal mask | signals whose delivery is blocked | per thread |
| Pending state | generated but not yet delivered | process- or thread-directed |
| Handler execution | asynchronous control transfer | one eligible thread |

This chapter's focused programs are single-threaded unless they fork. In a
threaded runtime, masking discipline must be designed across all threads.

## Three Main Dispositions

~~~text
SIG_DFL  -> kernel performs the signal's documented default action
SIG_IGN  -> kernel discards the signal when appropriate
handler  -> kernel arranges a user-space handler invocation
~~~

Default actions include termination, termination with a core dump, stopping,
continuing, and ignoring. Never assume that every signal terminates.

`SIGKILL` and `SIGSTOP` are special: a process cannot catch, block, or ignore
them. That property preserves administrative control over a process.

## A Safe Handler Is Tiny

The handler can interrupt code while libc or the allocator holds internal
state. Calling an unsafe function from the handler can deadlock or corrupt that
state.

Safe pattern:

~~~text
handler:
    save errno if needed
    assign a volatile sig_atomic_t flag
    or write a small record to a prepared file descriptor
    restore errno if needed
    return

main loop:
    observe flag or descriptor
    perform logging, allocation, cleanup, and state transitions normally
~~~

`printf()`, `malloc()`, `free()`, and most application logic do not belong in a
signal handler. `write()` is async-signal-safe, which enables the self-pipe
pattern.

## Blocking Is Deferred Delivery

~~~text
block SIGUSR1
generate SIGUSR1
        |
        v
SIGUSR1 pending, handler not running
        |
unblock SIGUSR1
        |
        v
delivery becomes possible
~~~

For ordinary signals, pending state is generally one bit. Repeated generation
while blocked can coalesce. Real-time signals are queued and can preserve
multiple occurrences and payloads, subject to resource limits.

## Race-Free Waiting

The broken sequence is:

~~~text
check flag -> signal arrives -> call pause() -> sleep forever
~~~

The correct pattern blocks the signal before checking shared state and uses
`sigsuspend()` to replace the mask and sleep as one atomic operation:

~~~text
block target signal
prepare state
while condition is false:
    sigsuspend(mask that unblocks target)
restore original mask
~~~

When asynchronous execution is unnecessary, `sigwait()` can consume a blocked
signal synchronously. This often simplifies dedicated signal-management code.

## Sending Targets

| Call shape | Target |
|---|---|
| `raise(sig)` | calling process |
| `kill(pid, sig)` | one process |
| `kill(0, sig)` | caller's process group |
| `kill(-pgid, sig)` | selected process group |
| `kill(-1, sig)` | broad permitted set; dangerous outside controlled tools |
| `kill(pid, 0)` | existence/permission probe; no signal delivered |

The lab uses only the first, second, isolated-group, and null-signal forms. It
never broadcasts to an uncontrolled process set.

## Fork and Exec

After `fork()`:

- dispositions are inherited;
- the signal mask is inherited;
- pending signals are not copied into the child;
- parent and child then own separate user-space state.

After `exec()`:

- ignored dispositions remain ignored;
- caught dispositions reset to default because old handler code no longer
  belongs to the new process image;
- the signal mask remains in effect;
- the PID remains the same.

This is why a launcher must reason about the mask and dispositions it leaves
for a program it executes.

## Interrupted Operations

A caught signal can cause a blocking interface to fail with `EINTR`. The
handler may have run successfully even though the interrupted operation did
not complete.

`SA_RESTART` asks the implementation to restart certain compatible calls. It
does not restart everything. Robust systems code must document whether each
operation is retried, restarted automatically, converted into an event, or
treated as cancellation.

## Metadata and Payloads

With `SA_SIGINFO`, the handler receives:

- the signal number;
- a `siginfo_t` describing origin and context;
- an implementation-defined execution-context pointer.

`si_code` must be interpreted in the context of the signal. `sigqueue()` can
attach a small integer or pointer-sized value. The payload is suitable for
notification metadata, not bulk transport or a complex protocol.

## Linux Integration Patterns

The self-pipe pattern stays portable across POSIX systems: a minimal handler
writes to a pipe, and `poll()` or an event loop handles the notification.

Linux `signalfd()` offers another design: block selected signals and consume
structured records from a file descriptor. This integrates naturally with
`poll()` and `epoll()`, but it is not portable POSIX behavior.

## Rules to Carry Forward

- Use symbolic signal names, never memorized numbers.
- Prefer `sigaction()` over the historical `signal()` interface.
- Initialize every `sigset_t` through the signal-set API.
- Preserve and restore an existing mask instead of assuming it was empty.
- Keep handlers minimal and async-signal-safe.
- Use `volatile sig_atomic_t` only for simple handler-visible state.
- Block before checking state, then wait atomically with `sigsuspend()`.
- Use `sigwait()` or `signalfd()` when synchronous dispatch is a better fit.
- Treat `EINTR` and `SA_RESTART` as interface-specific design questions.
- Expect ordinary signals to coalesce; use real-time signals only when queueing
  semantics are genuinely required.
- Reap children whose signal termination you intentionally create.
- Never test process-group signaling against an uncontrolled group.
