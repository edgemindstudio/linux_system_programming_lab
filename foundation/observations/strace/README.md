# Smoke-Test System-Call Observation

## Program

`foundation/smoke_test.c`

## Purpose

The smoke test verifies that the repository can compile and execute a small C program. It also provides an introductory demonstration of how a user-space program requests services from the Linux kernel.

## Source-Level Operations

The program performs three visible actions:

1. obtains its process identifier using `getpid()`;
2. formats output using `printf()`;
3. terminates successfully by returning `EXIT_SUCCESS`.

## Full Trace

The full trace is stored in:

```text
foundation/observations/strace/smoke_test.strace
```

The full trace includes work performed before `main()` begins, including:

* loading the executable;
* locating and loading the C library;
* creating memory mappings;
* configuring process and thread state;
* preparing the runtime environment.

## Focused Trace

The focused trace is stored in:

```text
foundation/observations/strace/smoke_test_focused.strace
```

Observed calls:

```text
getpid()                                = 29056
write(1, "Linux System Programming Laborat"..., 36) = 36
write(1, "Smoke test completed successfull"..., 35) = 35
write(1, "Process ID: 29056\n", 18) = 18
exit_group(0)                           = ?
+++ exited with 0 +++
```

Process identifiers vary between executions.

## Interpretation

### `getpid()`

The program requests its process identifier from the kernel.

### `write(1, ...)`

The program sends bytes to file descriptor `1`, which represents standard output.

The source code calls `printf()`, but `printf()` is a C library function, not a Linux system call. The library formats the text and ultimately uses `write()` to request output from the kernel.

### `exit_group(0)`

The process terminates with status code `0`, indicating successful completion.

## Mental Model

```text
C source program
    ↓ compilation
ELF executable
    ↓ execution
dynamic loader and C library
    ↓
system-call interface
    ↓
Linux kernel
    ↓
terminal and process resources
```

## Key Findings

* A program performs significant initialization before `main()` begins.
* Library functions and system calls are not the same thing.
* `printf()` eventually caused `write()` system calls.
* Standard output is represented by file descriptor `1`.
* The kernel manages process identity, output, and process termination.
* `strace` makes the user-space-to-kernel interaction visible.

## Compiler and Runtime Relevance

Compilers and runtimes use the same mechanisms when they:

* retrieve process information;
* print warnings and diagnostics;
* write logs;
* communicate through standard streams;
* load shared libraries;
* allocate and map memory;
* terminate with success or failure status.
