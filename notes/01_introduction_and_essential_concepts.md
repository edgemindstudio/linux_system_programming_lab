# Introduction and Essential Concepts

These notes summarize the foundational ideas needed before moving deeper into Linux system programming.

The goal of this chapter is not to memorize every Linux detail. The goal is to build a mental model of how user programs, libraries, system calls, the kernel, and system resources fit together.

---

## 1. What Systems Programming Means

Systems programming is the development of software that works closely with the operating system and the resources that the operating system manages.

Examples include:

* shells;
* compilers;
* linkers;
* loaders;
* debuggers;
* language runtimes;
* databases;
* servers;
* system utilities.

Systems programming does not necessarily mean writing kernel code.

Most systems programs run in **user space** and communicate with the kernel through defined operating-system interfaces.

A useful mental model is:

```text
Application software
        ↓
Systems software
        ↓
Operating-system interfaces
        ↓
Kernel
        ↓
Hardware
```

A compiler is a good example of systems software. It runs in user space, but it interacts closely with files, processes, memory, binaries, libraries, and other operating-system resources.

---

## 2. User Space Versus Kernel Space

Linux separates execution into two important environments:

* user space;
* kernel space.

### User space

User space is where ordinary programs normally execute.

Examples include:

* my C programs;
* shells;
* compilers;
* text editors;
* browsers;
* runtimes;
* databases.

Programs in user space operate with restricted privileges.

They cannot directly manipulate protected hardware or kernel resources.

### Kernel space

Kernel space is where the Linux kernel executes.

The kernel manages resources such as:

* CPU scheduling;
* processes;
* virtual memory;
* filesystems;
* networking;
* device access;
* security;
* permissions;
* signals;
* interprocess communication.

The basic relationship is:

```text
USER SPACE

My program
    │
    │ requests a protected service
    ▼

==============================
   System-call boundary
==============================

    ▼

KERNEL SPACE

Linux kernel
    │
    ├── processes
    ├── memory
    ├── filesystems
    ├── networking
    ├── permissions
    └── devices
```

The separation protects the computer from programs accidentally or maliciously interfering with system-wide resources.

---

## 3. System Calls Versus Library Functions

A **system call** is a controlled request from a user-space program to the Linux kernel.

Examples include:

```text
read()
write()
open()
close()
mmap()
fork()
execve()
```

A system call allows a program to ask the kernel to perform an operation that the program cannot safely perform directly.

The basic flow is:

```text
User program
    ↓
System-call interface
    ↓
Linux kernel
    ↓
Kernel performs or rejects operation
    ↓
Result or error returned
    ↓
User program continues
```

A **library function** is different.

A library function executes as part of user-space software.

Examples include:

```text
printf()
strlen()
fopen()
malloc()
```

A library function may:

1. perform all of its work in user space;
2. wrap a system call;
3. perform user-space processing and then invoke one or more system calls.

For example:

```text
printf()
    ↓
C library formats the output
    ↓
C library eventually requests output
    ↓
write()
    ↓
Linux kernel
```

Therefore:

```text
Library function ≠ system call
```

A library function can eventually cause a system call, but they are conceptually different layers.

---

## 4. API Versus ABI

### API — Application Programming Interface

An API defines how source code interacts with a service or library.

An API describes things such as:

* function names;
* parameter types;
* return values;
* constants;
* structures;
* expected behavior.

For example:

```c
ssize_t read(int fd, void *buf, size_t count);
```

This is part of the interface visible to a C programmer.

The API exists primarily at the **source-code level**.

### ABI — Application Binary Interface

An ABI defines how compiled binary components interact.

It includes rules for:

* calling conventions;
* register usage;
* argument passing;
* return values;
* stack organization;
* data-type sizes;
* memory alignment;
* object-file formats;
* symbol representation.

The relationship can be summarized as:

```text
API
    Source-code agreement

ABI
    Binary-level agreement
```

This distinction is especially important in compiler engineering.

A compiler must generate machine code that follows the platform ABI so that separately compiled functions, libraries, linkers, loaders, and operating-system components can work together correctly.

---

## 5. C Library and Compiler Roles

### The compiler

The compiler translates source code into lower-level representations and eventually machine-executable code.

A simplified compilation pipeline is:

```text
C source
    ↓
preprocessor
    ↓
compiler
    ↓
assembly
    ↓
assembler
    ↓
object file
    ↓
linker
    ↓
executable
```

The compiler is not the operating system.

Its main responsibility is program translation.

### The C library

The C library provides commonly used programming interfaces.

On many Linux systems, this library is GNU libc, or glibc.

It provides facilities such as:

* formatted I/O;
* memory allocation;
* string manipulation;
* file streams;
* process-related interfaces;
* system-call wrappers.

A useful relationship is:

```text
My C program
      ↓
C library
      ↓
system-call interface when necessary
      ↓
Linux kernel
```

The compiler and C library therefore solve different problems.

```text
Compiler
    translates the program

C library
    provides reusable runtime functionality and OS interfaces
```

---

## 6. POSIX and Portability

POSIX defines standardized operating-system interfaces used by Unix-like systems.

Its purpose is to make programs more portable between compatible operating systems.

Examples of systems influenced by POSIX include:

* Linux;
* macOS;
* BSD systems;
* other Unix-like operating systems.

A useful distinction is:

```text
ISO C
    standardizes the C language and C standard library

POSIX
    standardizes many operating-system programming interfaces

Linux
    implements many POSIX interfaces
    and also provides Linux-specific features
```

Code that relies only on portable standards can often be moved between systems more easily.

Linux-specific interfaces may provide additional capabilities but reduce portability.

As an engineer, I should recognize when I am writing:

```text
portable C
portable POSIX code
Linux-specific systems code
```

---

## 7. Files and File Descriptors

Unix and Linux expose many resources using file-like interfaces.

A file descriptor is a small integer maintained by a process that refers to an open resource managed by the kernel.

Conceptually:

```text
My process

fd 0 ───► standard input
fd 1 ───► standard output
fd 2 ───► standard error
fd 3 ───► some opened resource
fd 4 ───► another opened resource
```

A file descriptor can refer to resources such as:

* regular files;
* terminals;
* pipes;
* sockets;
* devices.

The file descriptor is not the file itself.

It is a process-local handle that allows the program to refer to a kernel-managed open resource.

This is why common operations such as:

```text
read()
write()
close()
```

can work with several different resource types.

### Filesystem concept

A pathname is also not exactly the same thing as a file.

A simplified model is:

```text
pathname
    ↓
directory lookup
    ↓
filesystem object
    ↓
data + metadata
```

This distinction becomes more important later when studying links, descriptors, and open files.

---

## 8. Processes and Threads

### Program

A program is a passive collection of executable instructions and data, usually stored in a file.

### Process

A process is a running instance of a program.

A process has resources such as:

* a virtual address space;
* executable code;
* global data;
* heap;
* stack;
* open file descriptors;
* process identifier;
* security credentials;
* execution state.

Therefore:

```text
Program
    passive executable

Process
    active execution of a program
```

Multiple processes can execute the same program.

### Thread

A thread is a path of execution inside a process.

Threads belonging to the same process normally share:

* code;
* global data;
* heap;
* address space;
* open file descriptors.

Each thread has its own execution state, including:

* registers;
* stack;
* instruction position.

A useful distinction is:

```text
Process
    resource container and execution environment

Thread
    execution path inside that environment
```

---

## 9. Users, Groups, and Permissions

Linux uses users and groups as part of its security model.

### User

A user represents an identity recognized by the operating system.

### Group

A group allows multiple users to share permissions and resource access.

### Permissions

Permissions define which operations are allowed on resources.

Common filesystem permissions include:

```text
read
write
execute
```

Processes also have security credentials.

When a process requests an operation, the kernel decides whether that operation is permitted.

Conceptually:

```text
Process requests operation
        ↓
Kernel identifies process credentials
        ↓
Kernel checks permissions
        ↓
Allowed
   or
Denied
```

Permissions are therefore enforced by the kernel.

They are not simply recommendations made by applications.

---

## 10. Signals and IPC

### Signals

A signal is a mechanism used to notify a process that some event has occurred.

Examples include:

* a user interrupting a program;
* a termination request;
* an invalid memory access;
* a timer event;
* a child process changing state.

A simplified signal model is:

```text
Event occurs
    ↓
Kernel generates/delivers signal
    ↓
Process receives signal
    ↓
Default action
    or
Installed signal handler
```

Signals can affect the normal control flow of a program.

### IPC — Interprocess Communication

Processes normally have separate address spaces.

One process cannot simply access another process's ordinary variables.

Linux therefore provides controlled interprocess communication mechanisms.

Examples include:

* pipes;
* sockets;
* shared memory;
* signals;
* message queues.

The problem IPC solves is:

```text
Process A                    Process B

private address space        private address space
       │                            │
       └──────── IPC ───────────────┘
```

IPC provides controlled ways for independent processes to exchange information or coordinate work.

---

## 11. Headers and Error Handling

### Header files

C header files provide declarations and definitions needed when using system and library interfaces.

They may define:

* function prototypes;
* types;
* constants;
* macros;
* structures.

Examples include:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
```

A program should include the appropriate header for the interfaces and types it uses.

For example:

```text
getpid()
    declared through <unistd.h>

pid_t
    defined through <sys/types.h>
```

### Error handling

Linux systems programming requires careful checking of return values.

A typical pattern is:

```c
int result = some_operation();

if (result == -1) {
    /* handle failure */
}
```

The exact failure value depends on the interface.

Possible failure indicators include:

```text
-1
NULL
MAP_FAILED
nonzero error codes
short results
```

After a function indicates failure, additional information may be available through:

```text
errno
```

A common helper is:

```c
perror("operation");
```

Important rule:

> Do not inspect `errno` simply because it contains a value.

The function's documented return value must first indicate failure.

A successful operation is not required to reset `errno` to zero.

Therefore the correct mental model is:

```text
Call function
    ↓
Check documented return value
    ↓
Did it indicate failure?
    ↓ yes
Inspect errno or error information
```

Manual pages are essential because each interface defines its own contract.

---

## 12. Compiler and Runtime Relevance

These introductory Linux concepts are directly connected to compiler and runtime engineering.

### Compiler relevance

A compiler may need to:

```text
read source files
    ↓
allocate memory
    ↓
create internal representations
    ↓
read libraries and metadata
    ↓
write assembly/object files
    ↓
invoke assemblers and linkers
    ↓
print diagnostics
```

Each of these activities depends on system interfaces.

Compilers also need to understand:

* APIs;
* ABIs;
* calling conventions;
* processes;
* object files;
* executable formats;
* operating-system conventions.

### Runtime relevance

A runtime may need to:

```text
allocate virtual memory
manage heaps and stacks
create and coordinate threads
handle signals
perform file and network I/O
load libraries
manage permissions
communicate with the operating system
```

A runtime operates between compiled application code and the underlying operating system.

A useful long-term mental model is:

```text
Application
    ↓
Language runtime
    ↓
C / system libraries
    ↓
Operating-system interfaces
    ↓
Linux kernel
    ↓
Hardware
```

Understanding Linux system programming therefore provides the operating-system foundation needed to reason about:

* compilers;
* loaders;
* virtual machines;
* garbage collectors;
* JIT compilers;
* thread runtimes;
* memory allocators;
* debuggers;
* performance tools.

---

## Chapter Mental Map

```text
                         USER SPACE

┌──────────────────────────────────────────────────────┐
│                                                      │
│  My C Program                                        │
│                                                      │
│  Uses:                                               │
│  • library functions                                 │
│  • POSIX/Linux APIs                                  │
│  • file descriptors                                  │
│  • processes and threads                             │
│  • signals and IPC                                   │
│                                                      │
└───────────────────────┬──────────────────────────────┘
                        │
                        ▼
                 C Library / APIs
                        │
                        ▼
========================================================
                SYSTEM-CALL BOUNDARY
========================================================
                        │
                        ▼

                       KERNEL

┌──────────────────────────────────────────────────────┐
│                                                      │
│  Linux Kernel                                        │
│                                                      │
│  Manages:                                            │
│  • processes and scheduling                          │
│  • virtual memory                                    │
│  • filesystems                                       │
│  • networking                                        │
│  • signals and IPC                                   │
│  • permissions                                       │
│  • devices                                           │
│                                                      │
└───────────────────────┬──────────────────────────────┘
                        │
                        ▼

                     HARDWARE
```

---

## 13. Questions I Still Have

Does Robert Love consider this book as a series?
