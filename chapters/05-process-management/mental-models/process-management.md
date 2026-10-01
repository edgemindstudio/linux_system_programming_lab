# Chapter 5 Mental Model — Process Management

## One Sentence

A Linux process is a kernel-managed execution container whose program image can
be copied with `fork()`, replaced with `exec()`, and eventually reduced to an
exit status that its parent must collect.

## Program, Process, and Thread

| Term | Plain-language meaning | Important ownership |
|---|---|---|
| Program | Executable instructions stored in a file | Persistent file bytes |
| Process | One running instance with kernel state | Address space, descriptors, credentials, PID |
| Thread | One execution path inside a process | Registers, instruction pointer, stack |

Threads in one process share the process address space and most process
resources. Different processes normally have separate address spaces.

## The Core Lifecycle

~~~text
existing process
      |
      | fork()
      v
parent + near-duplicate child
      |              |
      |              | execve()
      |              v
      |        same PID, new program image
      |              |
      |              | exit(), _exit(), return, or signal
      |              v
      |           zombie status
      |              |
      +---- wait()/waitpid()/waitid()
                     |
                     v
                   reaped
~~~

`fork()` creates another process. `exec()` does not create a process; it
replaces the calling process's program image.

## What fork() Produces

Immediately after a successful `fork()`:

- parent receives the child's PID;
- child receives zero;
- both resume after the same call;
- each has a distinct PID and virtual address space;
- memory initially has the same bytes and is optimized with copy-on-write;
- descriptors refer to the same underlying open-file descriptions;
- buffered `FILE` state is copied in user space;
- scheduler order is unspecified.

Equal virtual addresses in parent and child do not mean that later writes
change one shared C variable. A write normally triggers a page fault and a
private physical copy.

## What exec() Does

`execve()` keeps the process but replaces the program:

| Usually preserved | Replaced or reset |
|---|---|
| PID and PPID | Code, data, heap, and stack |
| Real/effective credentials, subject to exec rules | Argument and environment arrays |
| Working directory | C-library state and `atexit()` registrations |
| Open descriptors without `FD_CLOEXEC` | Mappings and most address-space state |
| Process group and session | Caught signal handlers reset to defaults |

A successful exec never returns. A return value of `-1` means the old program
is still running and must handle the error.

## Descriptor View Across fork() and exec()

~~~text
parent descriptor 3 ----+
                        +--> one kernel open-file description
child descriptor 3  ----+    (offset and status flags)

exec without FD_CLOEXEC: descriptor remains open
exec with FD_CLOEXEC:    kernel closes it during exec
~~~

Use `O_CLOEXEC` while opening when possible. It avoids the race between
`open()` and a later `fcntl(F_SETFD)` in a multithreaded process.

## Termination Has Two Layers

`exit()` is a C-library path:

1. run normal-exit handlers in reverse registration order;
2. flush standard I/O streams;
3. perform other library cleanup;
4. enter the kernel termination path.

`_exit()` skips user-space cleanup and terminates through the kernel. This is
why a child commonly calls `_exit(127)` when `exec()` fails: it must not flush a
second copy of buffers inherited from the parent.

## Waiting and Zombies

A terminated child cannot vanish before the parent has a chance to read its
status. The kernel retains a small zombie record containing information such as
the PID and wait status.

- `wait()` selects any terminated child;
- `waitpid()` can select one PID, a group, or any child;
- `WNOHANG` turns a blocking wait into a poll;
- `waitid()` returns a `siginfo_t` record;
- `WNOWAIT` observes a status without consuming it.

Always test the status category before reading category-specific data:

~~~text
WIFEXITED(status)   -> WEXITSTATUS(status)
WIFSIGNALED(status) -> WTERMSIG(status)
WIFSTOPPED(status)  -> WSTOPSIG(status)
~~~

## Parentage in Modern Linux

The simple historical rule says an orphan is adopted by PID 1. Modern Linux
adds two important contexts:

- inside a PID namespace, PID 1 is the namespace's init process;
- a process marked as a child subreaper can adopt orphaned descendants before
  they reach that PID 1.

Supervisors use subreapers so they can collect grandchildren reliably.

## Credentials

A process has several security identities:

- real user/group IDs: where it came from;
- effective user/group IDs: used by many access checks;
- saved IDs: allow controlled privilege transitions;
- supplementary groups: additional memberships;
- Linux capabilities and other security state: finer-grained authority.

Printing IDs is safe. Changing them correctly is security-sensitive and should
not be learned by experimenting as root in a general-purpose lab.

## Session and Process-Group Hierarchy

~~~text
session
├── foreground process group
│   ├── process
│   └── process
└── background process group
    └── process
~~~

A pipeline normally forms one process group. The terminal directs job-control
signals to the foreground group. A session collects process groups and may own
a controlling terminal.

`setsid()` makes a non-group-leader caller:

- leader of a new session;
- leader of a new process group;
- detached from a controlling terminal.

## Daemons: Historical and Current Model

Traditional self-daemonization uses `fork()`, `setsid()`, a directory and umask
policy, descriptor cleanup, and standard-stream redirection. A service manager
such as systemd already provides supervision, logging, restart policy,
credentials, working-directory selection, and descriptor passing. Under such a
manager, a service usually stays in the foreground.

## The Five Questions to Ask

Whenever process code behaves unexpectedly, ask:

1. Which process is executing this line?
2. Which memory is copied, shared, or already replaced?
3. Which descriptors survived, and what kernel objects do they reference?
4. Who will collect each child's state?
5. Which process group, session, namespace, and credential set apply?
