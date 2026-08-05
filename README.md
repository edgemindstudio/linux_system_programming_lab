# Linux System Programming Laboratory

Status: In progress

The objective of Milestone 0 is to establish:

* the repository structure;
* the learning philosophy;
* the build foundation;
* strict compiler warnings;
* an introductory smoke-test program;
* a roadmap for studying the book;
* the first mental map;
* the initial Git commit.

No major chapter implementation belongs in Milestone 0.

## Book

Robert Love, *Linux System Programming: Talking Directly to the Kernel and C Library*.

This repository contains original learning notes, exercises, experiments, and observations. It does not reproduce the book's copyrighted text.


A structured learning laboratory based on Robert Love's *Linux System Programming*.

This repository supports my long-term development as a systems engineer, with a primary career direction toward compiler and runtime engineering.

## Purpose

The purpose of this repository is not merely to record that I read the book. It is to develop practical understanding of how Linux user-space programs interact with the operating system, kernel services, and hardware.

The repository will combine:

* concise notes written in my own words;
* high-level mental models;
* focused C programming exercises;
* experiments based on predictions;
* observations collected with Linux tools;
* tests and debugging practice;
* larger projects integrating multiple concepts;
* explicit connections to compiler and runtime engineering.

## Central Mental Model

```text
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
```

Programs normally execute in user space and cannot directly control protected hardware resources. They request services from the Linux kernel through system calls.

The kernel manages protected resources such as:

* processes and CPU scheduling;
* virtual memory;
* files and file systems;
* networking;
* interprocess communication;
* security and permissions;
* hardware devices.

## Learning Method

Each important topic follows this learning cycle:

```text
Read
  ↓
Build a mental model
  ↓
Explain the concept in my own words
  ↓
Predict Linux behavior
  ↓
Write a focused C program
  ↓
Compile and run it
  ↓
Observe the system
  ↓
Explain the result
  ↓
Connect it to compiler/runtime engineering
```

## Learning Principles

### Understanding before implementation

Before writing code, I should understand:

* what problem the mechanism solves;
* where it lives in the system;
* what user space controls;
* what the kernel controls;
* what can fail;
* why the mechanism matters.

### Prediction before observation

For experiments, I should state what I expect Linux to do before running the program.

The comparison between prediction and observation is used to improve my mental model.

### First attempt before assistance

Whenever practical, I should personally produce:

1. my initial explanation;
2. my prediction;
3. my pseudocode;
4. my first implementation;
5. my first debugging attempt.

External assistance should be used for clarification, review, diagnosis, and deeper understanding rather than replacing the learning process.

### Evidence over assumption

Important conclusions should be supported with evidence from tools such as:

* `strace`;
* `gdb` or `lldb`;
* `/proc`;
* `readelf`;
* `objdump`;
* `nm`;
* `lsof`;
* sanitizers;
* tests;
* timing and profiling tools.

## Repository Structure

```text
linux_system_programming_lab/
├── README.md
├── ROADMAP.md
├── Makefile
├── notes/
├── exercises/
├── experiments/
├── mental_maps/
├── observations/
│   ├── strace/
│   ├── procfs/
│   ├── debugger/
│   └── binaries/
├── projects/
├── scripts/
└── tests/
```

### `notes/`

Concise explanations of important concepts written in my own words.

Notes should focus on:

* the problem being solved;
* the high-level mechanism;
* important abstractions;
* user-space responsibilities;
* kernel responsibilities;
* failure conditions;
* compiler/runtime relevance.

### `exercises/`

Small programs that practice one API, mechanism, or programming technique.

An exercise demonstrates how to use something.

### `experiments/`

Programs designed to answer a specific systems question.

An experiment begins with a prediction and records:

* the question;
* the prediction;
* the method;
* the observation;
* the explanation;
* the conclusion.

### `mental_maps/`

Diagrams and high-level representations showing how concepts fit together.

### `observations/`

Evidence collected from Linux inspection and debugging tools.

Examples include:

* system-call traces;
* process information from `/proc`;
* debugger sessions;
* executable and object-file inspection;
* performance measurements.

### `projects/`

Larger programs that combine several mechanisms learned from the book.

Projects will begin only after the relevant foundational topics have been studied.

### `tests/`

Automated checks for exercises, experiments, and projects where appropriate.

### `scripts/`

Small helper scripts for building, testing, tracing, inspecting, or cleaning repository artifacts.

## Topic Completion Criteria

A topic is considered sufficiently complete when I can:

* explain why the mechanism exists;
* place it within the larger Linux system;
* distinguish user-space and kernel responsibilities;
* identify its important APIs;
* write a small working example;
* handle basic errors correctly;
* observe its behavior with an appropriate tool;
* explain at least one common misconception;
* connect it to compiler or runtime engineering.

Perfect memorization of every flag, constant, and error code is not required. Reference material may be consulted when necessary.

## Compiler and Runtime Engineering Connection

This repository supports future work involving:

* compiler file and diagnostic I/O;
* process creation and toolchain orchestration;
* object files, linking, and loading;
* virtual memory and memory mapping;
* runtime heaps and stacks;
* concurrency and synchronization;
* signals, faults, and diagnostics;
* dynamic loading;
* virtual machines;
* garbage collection;
* JIT compilation;
* performance analysis.

## Current Milestone

### Milestone 0 — Laboratory Foundation

Status: In progress

The objective of Milestone 0 is to establish:

* the repository structure;
* the learning philosophy;
* the build foundation;
* strict compiler warnings;
* an introductory smoke-test program;
* a roadmap for studying the book;
* the first mental map;
* the initial Git commit.

No major chapter implementation belongs in Milestone 0.

## Book

Robert Love, *Linux System Programming: Talking Directly to the Kernel and C Library*.

This repository contains original learning notes, exercises, experiments, and observations. It does not reproduce the book's copyrighted text.
