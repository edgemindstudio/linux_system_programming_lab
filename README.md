# Linux System Programming Laboratory

A chapter-oriented laboratory for studying Robert Love's *Linux System
Programming: Talking Directly to the Kernel and C Library*, second edition.

The repository develops practical understanding of how Linux user-space
programs interact with the C library, system-call interface, kernel, and
hardware. It supports long-term preparation for systems, compiler, and runtime
engineering.

The repository contains original notes, programs, predictions, observations,
and explanations. It does not reproduce the book's copyrighted text.

## Current Status

| Area | Status |
|---|---|
| Repository foundation | Complete |
| Chapter 1 — Introduction and Essential Concepts | In progress |
| Chapter 2 — File I/O | Lab prepared; study in progress |
| Chapters 3–11 | Scaffolded; not started |
| Cross-chapter projects | Not started |

See [ROADMAP.md](ROADMAP.md) for the complete learning sequence.

## Central Mental Model

~~~text
User program
    ↓
C library and runtime
    ↓
System-call interface
    ↓
Linux kernel
    ↓
Device drivers
    ↓
Hardware
~~~

Programs normally execute in user space. They request protected services from
the kernel through system calls.

## Learning Method

Each important topic follows the same evidence-driven cycle:

~~~text
Read
  ↓
Build a mental model
  ↓
Explain the concept in my own words
  ↓
Predict Linux behavior
  ↓
Write pseudocode and a focused C program
  ↓
Compile and run
  ↓
Observe with Linux tools
  ↓
Explain the result
  ↓
Connect it to compiler/runtime engineering
~~~

Code existing in the repository does not mean a chapter is mastered. A chapter
is complete only after its programs have been studied, run, observed, explained,
and reviewed.

## Repository Structure

~~~text
linux_system_programming_lab/
├── chapters/
│   ├── 01-introduction-and-essential-concepts/
│   ├── 02-file-io/
│   ├── 03-buffered-io/
│   ├── 04-advanced-file-io/
│   ├── 05-process-management/
│   ├── 06-advanced-process-management/
│   ├── 07-threading/
│   ├── 08-file-and-directory-management/
│   ├── 09-memory-management/
│   ├── 10-signals/
│   └── 11-time/
├── foundation/
├── projects/
├── scripts/
├── docs/
├── Makefile
├── ROADMAP.md
└── README.md
~~~

Each chapter owns its learning assets. As a chapter is implemented, it may
contain:

~~~text
NN-chapter-title/
├── README.md
├── notes/
├── mental-models/
├── exercises/
├── experiments/
├── tests/
├── scripts/
└── observations/
~~~

Only directories that contain real work are committed. Future chapter
directories begin with a README and gain subdirectories as study starts. This
keeps the repository explicit without filling it with empty placeholders.

Detailed conventions are documented in
[docs/REPOSITORY_STRUCTURE.md](docs/REPOSITORY_STRUCTURE.md).

## Build and Test

The default compiler is Clang with C17 and strict warnings.

Build everything currently implemented:

~~~bash
make
~~~

Build one chapter:

~~~bash
make chapter CHAPTER=02
~~~

The convenience form also works:

~~~bash
make chapter02
~~~

Test one chapter:

~~~bash
make test CHAPTER=02
~~~

Test every implemented chapter:

~~~bash
make test-all
~~~

Run the repository-foundation smoke test:

~~~bash
make run
~~~

List every generated executable:

~~~bash
make list
~~~

See all supported commands:

~~~bash
make help
~~~

## Build Output

Generated files are kept outside the source directories. The build tree mirrors
the chapter tree:

~~~text
build/
├── foundation/
│   └── smoke_test
└── chapters/
    └── 02-file-io/
        ├── data/
        ├── exercises/
        └── experiments/
~~~

The entire build/ directory is ignored by Git and can be recreated at any time:

~~~bash
make clean
make
~~~

## Chapter Completion Criteria

A chapter is complete when I can:

- explain why its major mechanisms exist;
- identify what user space controls and what the kernel controls;
- use the important APIs and check their return values;
- predict the behavior of focused experiments;
- write and debug representative C programs;
- collect evidence with tools such as strace, /proc, gdb, readelf, or
  sanitizers;
- explain common mistakes and corrected assumptions;
- connect the chapter to compiler and runtime engineering;
- pass the chapter tests;
- commit the completed learning record.

## Compiler and Runtime Connection

The book's mechanisms support future work involving:

- compiler source, object, and diagnostic I/O;
- toolchain process orchestration;
- object files, linking, and loading;
- virtual memory and memory mapping;
- runtime heaps and stacks;
- threading and synchronization;
- signals, faults, and diagnostics;
- clocks, timers, profiling, and performance;
- virtual machines, garbage collectors, and JIT compilers.
