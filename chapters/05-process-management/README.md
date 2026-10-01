# Chapter 5 — Process Management

**Status:** Lab prepared; study in progress

## Purpose

Understand a Linux process as a running program with an identity, address
space, open resources, credentials, parent/child relationships, and a place in
the session and process-group hierarchy. The chapter follows a process from
creation through program replacement, termination, status collection, and
supervision.

## Book Scope

Robert Love, *Linux System Programming*, second edition, Chapter 5, printed
pages 137–175.

## Study Material

- `notes/study-guide-pages-137-175.md`
- `mental-models/process-management.md`

## Exercises

1. Inspect PID, PPID, process group, and session
2. Observe the two return values from `fork()`
3. Demonstrate copy-on-write state separation
4. Combine `fork()`, `exec()`, a pipe, and `waitpid()`
5. Supply a new argument vector and environment to `execve()`
6. Compare inherited descriptors with `FD_CLOEXEC`
7. Contrast stdio cleanup in `exit()` and `_exit()`
8. Observe reverse-order `atexit()` handlers
9. Decode a normal child exit
10. Decode signal termination
11. Wait for a specific child
12. Poll a running child with `WNOHANG`
13. Observe without reaping through `waitid()` and `WNOWAIT`
14. Inspect real/effective credentials and supplementary groups
15. Place a child in a new process group
16. Create a new session with `setsid()`
17. Pass shell metacharacters safely to an explicitly named executable
18. Verify the safe core of traditional daemon setup
19. Reap every child without assuming scheduler order

## Experiments

- construct and verify a three-generation process tree;
- expose duplicated C-library output buffers after `fork()`;
- measure copy-on-write minor page faults;
- observe a zombie before and after reaping;
- demonstrate Linux child-subreaper adoption;
- inspect the runtime PID limit, PID 1, and nested PID information;
- collect per-child resource usage with `wait4()`.

## Commands

~~~bash
make chapter CHAPTER=05
make test CHAPTER=05
make tidy-chapter CHAPTER=05
chapters/05-process-management/scripts/trace.sh 04_fork_exec_wait
chapters/05-process-management/scripts/trace.sh subreaper_adoption
~~~

Generated files are placed under:

~~~text
build/chapters/05-process-management/
~~~

## Safety Choices

The lab does not execute `vfork()`, change credentials, install set-user-ID
binaries, or leave background processes running. Those operations add risk
without improving the central mental model. Credential calls are observational,
daemon setup is short-lived, and every created child is reaped.

## Completion Rule

Building the lab is not the same as mastering it. Before moving to Chapter 6,
explain what `fork()` copies and shares, what `exec()` preserves and replaces,
why `_exit()` belongs in a failed post-fork exec path, how wait status is
decoded, why zombies exist, how subreapers update the old orphan model, and how
sessions and process groups support shells, supervisors, and services.
