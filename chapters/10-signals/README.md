# Chapter 10 - Signals

**Status:** Lab prepared; study in progress

## Purpose

Understand signals as kernel-mediated asynchronous notifications and learn to
control their dispositions, masks, pending state, delivery, waiting, metadata,
and payloads without introducing races or performing unsafe work in handlers.

## Book Scope

Robert Love, *Linux System Programming*, second edition, Chapter 10, printed
pages 333-362.

## Study Material

- `notes/study-guide-pages-333-362.md`
- `mental-models/signals.md`

## Exercises

1. Inspect portable signal identifiers
2. Install a handler with `sigaction()`
3. Suspend until a signal is handled
4. Add a signal to a handler's temporary mask
5. Observe disposition inheritance across `fork()`
6. Observe disposition changes across `exec()`
7. Map signal numbers to descriptions
8. Probe a process with the null signal
9. Terminate and reap a child with `kill()`
10. Send a signal to the current process with `raise()`
11. Signal an isolated process group safely
12. Communicate through `volatile sig_atomic_t`
13. Construct and inspect signal sets
14. Block a signal and inspect pending state
15. Wait atomically with `sigsuspend()`
16. Accept a blocked signal synchronously with `sigwait()`
17. Inspect sender metadata with `SA_SIGINFO`
18. Send an integer payload with `sigqueue()`

## Experiments

- demonstrate standard-signal coalescing while delivery is blocked;
- demonstrate ordered real-time signal queueing;
- observe `EINTR` when a handler interrupts a blocking read;
- compare the same read when `SA_RESTART` is enabled;
- bridge a minimal handler into a normal event loop with a self-pipe;
- consume a blocked signal through Linux `signalfd()`;
- compare configured dispositions and masks with `/proc/self/status`.

## Commands

~~~bash
make chapter CHAPTER=10
make test CHAPTER=10
make tidy-chapter CHAPTER=10
chapters/10-signals/scripts/trace.sh 02_sigaction_handler
chapters/10-signals/scripts/trace.sh standard_signal_coalescing
~~~

Generated files are placed under:

~~~text
build/chapters/10-signals/
~~~

## Safety Choices

The lab never signals unrelated processes. Destructive default actions are
tested only in child processes created by the exercise. The process-group
exercise places its child into a new group before signaling that group. Every
handler is intentionally tiny and uses only `volatile sig_atomic_t` assignment
or an async-signal-safe `write()`. Timeouts in the smoke test prevent an
unexpected waiting bug from hanging the test suite.

## Completion Rule

Building the lab is not the same as mastering it. Before moving to Chapter 11,
explain generation versus pending state versus delivery, dispositions, default
actions, masks, standard-signal coalescing, real-time queueing, handler
reentrancy, async-signal-safe operations, `EINTR`, `SA_RESTART`, race-free
waiting, `siginfo_t`, payloads, and why signals are notifications rather than a
general-purpose data transport.
