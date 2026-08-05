# Linux System Programming Laboratory Roadmap

This roadmap organizes the study of Robert Love's *Linux System Programming* into practical learning milestones.

The roadmap is not based only on page or chapter completion. Progress is measured by demonstrated understanding, implementation, observation, and explanation.

## Progress States

Each milestone or topic may have one of these states:

* Not started
* Reading
* Implementing
* Experimenting
* Reviewing
* Complete
* Revisit later

A topic is complete only when its completion criteria have been satisfied.

---

## Milestone 0 — Laboratory Foundation

**Status:** Complete

### Objective

Establish a clean, repeatable environment for studying Linux systems programming.

### Deliverables

* [x] Create the repository.
* [x] Initialize Git on the `main` branch.
* [x] Create the repository directory structure.
* [x] Write the repository purpose and learning method in `README.md`.
* [x] Define the study roadmap.
* [x] Add a suitable `.gitignore`.
* [x] Establish a strict C build configuration.
* [x] Add an introductory smoke-test program.
* [x] Compile and run the smoke test.
* [x] Inspect the smoke test using at least one Linux tool.
* [x] Add the first high-level mental map.
* [x] Review the repository foundation.
* [x] Create the initial Git commit.

### Completion Criteria

Milestone 0 is complete when:

* the repository builds successfully;
* the smoke-test program runs successfully;
* generated files are excluded from Git;
* the repository purpose and study process are documented;
* the first mental map is stored in the repository;
* the working tree is clean after the initial commit.

No major chapter exercise belongs in this milestone.

---

## Milestone 1 — Linux Systems Programming Foundations

**Status:** Not started

### Core Questions

* What is systems programming?
* What is the difference between user space and kernel space?
* What is an operating system?
* What is the Linux kernel?
* What is a system call?
* What is the role of the C library?
* What is POSIX?
* How do system calls differ from ordinary function calls?
* How are errors reported?
* What are `errno` and `perror()`?
* How are Linux manual pages organized?

### Planned Outputs

* introductory systems-programming notes;
* user-space and kernel-space mental map;
* system-call path mental map;
* `errno` exercise;
* manual-page navigation exercise;
* introductory `strace` observation;
* compiler/runtime relevance summary.

### Completion Criteria

I can:

* explain the user-space-to-kernel path;
* distinguish a library call from a system call;
* explain how failures are reported;
* use the relevant manual-page sections;
* trace a simple program and identify important system calls;
* connect these foundations to compilers and runtimes.

---

## Milestone 2 — File Descriptors and Basic File I/O

**Status:** Not started

### Core Topics

* file descriptors;
* standard input, output, and error;
* `open()`;
* `read()`;
* `write()`;
* `close()`;
* file offsets;
* partial reads and writes;
* interruption by signals;
* open flags;
* creation modes;
* error handling.

### Planned Exercises

* print descriptor values;
* open and read a file;
* write to a file;
* implement a basic file-copy utility;
* redirect output to a file;
* intentionally trigger and inspect I/O errors.

### Planned Experiments

* file-descriptor allocation and reuse;
* behavior after closing a descriptor;
* partial read behavior;
* standard descriptor behavior;
* file-offset changes;
* effects of `O_APPEND`.

### Compiler/Runtime Connection

* reading source files;
* writing object files and diagnostics;
* handling standard streams;
* build-tool pipelines;
* runtime resource and log I/O.

---

## Milestone 3 — Buffered I/O and the C Standard Library

**Status:** Not started

### Core Topics

* `FILE`;
* standard I/O streams;
* buffering modes;
* `fopen()`;
* `fread()`;
* `fwrite()`;
* `fgets()`;
* `fprintf()`;
* flushing;
* end-of-file and error states;
* interaction between buffered and unbuffered I/O.

### Planned Experiments

* terminal versus redirected-output buffering;
* explicit flushing;
* buffered versus unbuffered copying;
* mixing file-descriptor and stream operations.

### Compiler/Runtime Connection

* formatted diagnostics;
* source-text reading;
* buffering performance;
* deterministic output behavior.

---

## Milestone 4 — Advanced File I/O

**Status:** Not started

### Core Topics

* vectored I/O;
* positioned I/O;
* nonblocking I/O;
* synchronous I/O;
* direct I/O;
* I/O multiplexing;
* file metadata;
* advisory operations where relevant.

### Planned Experiments

* compare sequential and positioned I/O;
* inspect nonblocking behavior;
* compare multiple-buffer versus single-buffer operations;
* study readiness notification.

### Compiler/Runtime Connection

* scalable tool pipelines;
* language servers;
* build systems;
* runtime event processing;
* efficient binary and object-file access.

---

## Milestone 5 — Files, Directories, and File-System Metadata

**Status:** Not started

### Core Topics

* file types;
* metadata;
* permissions;
* ownership;
* links;
* directories;
* current working directory;
* path handling;
* timestamps;
* file-system limits.

### Planned Exercises

* inspect file metadata;
* list directory entries;
* classify file types;
* create and inspect links;
* resolve and manipulate paths safely.

### Compiler/Runtime Connection

* include-file discovery;
* module lookup;
* dependency scanning;
* build caching;
* installation layouts;
* executable and library discovery.

---

## Milestone 6 — Processes and Program Execution

**Status:** Not started

### Core Topics

* process identity;
* process creation;
* `fork()`;
* process termination;
* waiting;
* zombies;
* `exec` family;
* environment variables;
* process relationships.

### Planned Exercises

* create a child process;
* inspect parent and child identifiers;
* execute another program;
* collect child exit status;
* build a minimal process launcher.

### Planned Experiments

* memory behavior after `fork()`;
* shared file offsets after `fork()`;
* environment inheritance;
* open descriptors across `exec`;
* zombie-process observation.

### Compiler/Runtime Connection

* invoking assemblers and linkers;
* build-system process orchestration;
* test runners;
* worker processes;
* runtime process management.

---

## Milestone 7 — Signals

**Status:** Not started

### Core Topics

* signal concepts;
* default actions;
* signal handlers;
* blocking;
* pending signals;
* interrupted system calls;
* signal-safe programming;
* signal delivery.

### Planned Exercises

* install a basic handler;
* block and unblock a signal;
* communicate signal occurrence safely;
* handle program interruption.

### Planned Experiments

* signal interruption of blocking calls;
* standard versus real-time behavior where appropriate;
* process termination and cleanup;
* signal delivery across parent and child processes.

### Compiler/Runtime Connection

* interruption and cancellation;
* crash reporting;
* fault handling;
* profiling;
* runtime safepoint concepts;
* process supervision.

---

## Milestone 8 — Memory Management and Memory Mapping

**Status:** Not started

### Core Topics

* process address space;
* stack and heap;
* page-based virtual memory;
* `malloc()` and allocator behavior;
* `brk()` and `sbrk()` as historical mechanisms;
* `mmap()`;
* `munmap()`;
* protection;
* anonymous mappings;
* file-backed mappings;
* page faults.

### Planned Exercises

* inspect process memory mappings;
* create anonymous mappings;
* map a file;
* modify a mapped region;
* change memory protections.

### Planned Experiments

* mapping visibility;
* private versus shared mappings;
* page-aligned behavior;
* access violations;
* memory growth and release.

### Compiler/Runtime Connection

* compiler arenas;
* runtime heaps;
* stacks;
* garbage collection;
* loaders;
* JIT code memory;
* executable permissions;
* memory-mapped object files.

---

## Milestone 9 — Threads and Synchronization

**Status:** Not started

### Core Topics

* threads;
* shared address spaces;
* thread creation;
* thread lifecycle;
* mutexes;
* condition variables;
* race conditions;
* deadlocks;
* thread-local storage;
* synchronization costs.

### Planned Exercises

* create and join threads;
* protect shared state;
* implement producer-consumer coordination;
* detect and correct a race.

### Planned Experiments

* unsynchronized counter behavior;
* mutex protection;
* condition-variable wakeups;
* deadlock construction and diagnosis;
* sanitizer-assisted race detection.

### Compiler/Runtime Connection

* parallel compilation;
* optimization pipelines;
* thread pools;
* work-stealing runtimes;
* concurrent garbage collection;
* runtime schedulers.

---

## Milestone 10 — Time, Timers, and Performance Observation

**Status:** Not started

### Core Topics

* system time;
* monotonic time;
* clocks;
* sleeping;
* timers;
* resource usage;
* measurement pitfalls.

### Planned Exercises

* measure elapsed time correctly;
* compare clocks;
* implement a basic timeout;
* inspect process resource usage.

### Compiler/Runtime Connection

* compiler pass timing;
* runtime scheduling;
* profiling;
* benchmark design;
* timeout management;
* latency analysis.

---

## Milestone 11 — Integrated Systems Project

**Status:** Not started

### Objective

Build one meaningful project that combines several mechanisms studied in the repository.

The exact project will be selected only after the supporting milestones are complete.

### Candidate Projects

* robust file-copy and inspection utility;
* minimal command shell;
* process supervisor;
* memory-mapped file inspector;
* concurrent log-processing tool;
* simple event-driven service;
* compiler-driver prototype that launches toolchain stages.

### Required Engineering Qualities

The project should include:

* documented requirements;
* clear architecture;
* careful error handling;
* tests;
* reproducible builds;
* system observations;
* debugging evidence;
* performance considerations;
* a compiler/runtime relevance analysis.

---

## Final Book Retrospective

**Status:** Not started

At the end of the book, document:

* what I can now explain;
* what I can now implement;
* what I can now debug;
* which assumptions were corrected;
* which topics require deeper study;
* how the material improved my compiler/runtime preparation;
* what project or book should follow;
* a complete mental map drawn from memory.

---

## Standard Topic Workflow

Every important topic should follow this sequence:

1. Identify the problem.
2. Build the high-level mental model.
3. Read the relevant material.
4. Explain the concept in my own words.
5. State a prediction.
6. Write pseudocode.
7. Implement the first version.
8. Compile with strict warnings.
9. Run and inspect the behavior.
10. Record observations.
11. Explain discrepancies.
12. Add tests where appropriate.
13. Connect the mechanism to compiler/runtime engineering.
14. Review before marking the topic complete.
15. Commit the completed learning unit.

---

## Current Next Step

Complete Milestone 0 by adding:

1. `.gitignore`;
2. the initial strict `Makefile`;
3. a small smoke-test source file;
4. the first repository mental map;
5. the initial review and Git commit.
