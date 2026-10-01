# Chapter 7 Observations

Store deliberate Chapter 7 evidence here. Generated traces are ignored by Git
unless explicitly selected for long-term study.

Suggested observations:

- compare PID, kernel TID, and Pthread identity;
- locate `clone`/`clone3`, `futex`, and thread exits in `strace` output;
- inspect `/proc/self/task` while workers are alive;
- run the logical-race experiment repeatedly and explain why it remains
  defined despite losing updates;
- compare uncontended and contended mutex traces;
- observe that some mutex operations complete entirely in user space;
- use GDB's `info threads` on a program stopped at a worker breakpoint;
- use ThreadSanitizer only on deliberately prepared examples, never assume a
  normal run proves the absence of a race.

Do not commit enormous raw traces. Preserve short, explained evidence that
answers a specific prediction.
