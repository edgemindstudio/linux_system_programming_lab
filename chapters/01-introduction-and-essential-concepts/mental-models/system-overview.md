# Linux System Programming Overview

## High-Level System Map

```text
┌─────────────────────────────────────────────────────────────┐
│                    USER SPACE                               │
│                                                             │
│  ┌───────────────┐     ┌─────────────────────────────────┐  │
│  │ Your C Program│ ──▶ │ C Library / Runtime             │  │
│  │               │     │                                 │  │
│  │ main()        │     │ printf(), malloc(), fopen()     │  │
│  └───────────────┘     └─────────────────────────────────┘  │
│             │                         │                      │
│             │ direct system call      │ library eventually   │
│             │                         │ requests kernel work  │
└─────────────┼─────────────────────────┼──────────────────────┘
              │                         │
══════════════╪═════════════════════════╪═══════════════════════
              │      SYSTEM-CALL INTERFACE
              │
              ▼
┌─────────────────────────────────────────────────────────────┐
│                    KERNEL SPACE                             │
│                                                             │
│  ┌────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ Processes  │  │ Virtual     │  │ Files and VFS       │  │
│  │ Scheduler  │  │ Memory      │  │                     │  │
│  └────────────┘  └─────────────┘  └─────────────────────┘  │
│                                                             │
│  ┌────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ Networking │  │ Security    │  │ Device Drivers      │  │
│  │ and IPC    │  │ Permissions │  │                     │  │
│  └────────────┘  └─────────────┘  └─────────────────────┘  │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                       HARDWARE                              │
│                                                             │
│        CPU      RAM      Disk      Network      Devices     │
└─────────────────────────────────────────────────────────────┘
```

## Request and Return Flow

```text
User program
    │
    │ request
    ▼
C library or direct system-call wrapper
    │
    ▼
System-call interface
    │
    ▼
Linux kernel
    │
    ▼
Kernel subsystem or device driver
    │
    ▼
Hardware or protected resource

Hardware/resource result
    │
    ▼
Linux kernel
    │
    ▼
Return value, data, or error
    │
    ▼
User program
```

## Smoke-Test Example

```text
getpid()
    ↓
system-call interface
    ↓
kernel process-management subsystem
    ↓
process ID returned to the program
```

```text
printf()
    ↓
C library formatting and buffering
    ↓
write(1, ...)
    ↓
kernel file and terminal handling
    ↓
text appears in the terminal
```

## Core Principle

User-space programs cannot directly control protected hardware and kernel resources.

They request controlled services through the system-call interface. The kernel validates the request, manages the resource, performs or coordinates the operation, and returns a result or error.
