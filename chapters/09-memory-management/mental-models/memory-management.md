# Chapter 9 Mental Model - Memory Management

## One Process, One Virtual Address Space

A pointer used by a C program is normally a virtual address. The CPU and Linux
translate virtual pages to physical memory or another backing store. Different
regions can have different permissions, lifetimes, sharing rules, and backing.

~~~text
high addresses
+-----------------------------+
| thread stacks               | grows as calls need frames
+-----------------------------+
| shared libraries/mappings   | file-backed or anonymous
+-----------------------------+
| heap                        | allocator-managed dynamic storage
+-----------------------------+
| BSS                         | zero-initialized static storage
+-----------------------------+
| initialized data            | initialized static storage
+-----------------------------+
| executable code/read-only   | instructions and constants
+-----------------------------+
low addresses
~~~

This is a useful conceptual picture, not a portable address-order promise.
ASLR, the loader, the ABI, and the allocator determine actual placement.

## The Layers

| Layer | Responsibility |
|---|---|
| C program | asks for objects and obeys their bounds and lifetimes |
| allocator | manages reusable blocks and implements `malloc()` family rules |
| C library | may use the program break, anonymous mappings, or both |
| Linux VM | manages regions, permissions, faults, sharing, and reclamation |
| MMU/CPU | translates virtual pages and enforces access permissions |
| physical memory/storage | supplies frames and backing when needed |

The allocator and kernel solve different problems. `free()` returns a block to
the allocator. That does not necessarily mean the kernel immediately receives
physical pages or a smaller address space.

## Page Lifecycle

~~~text
reserve virtual range
        |
        v
first access causes a page fault
        |
        v
Linux supplies/maps a page and resumes the instruction
        |
        v
page may remain resident, be reclaimed, locked, or unmapped
~~~

A page fault is not automatically a bug. Demand paging relies on recoverable
minor faults. Invalid permissions or absent regions instead lead to a fault
that Linux cannot satisfy for the process.

## Allocation Contracts

| Operation | Main guarantee | Main responsibility |
|---|---|---|
| `malloc(n)` | at least `n` bytes or `NULL` | initialize before reading; `free()` once |
| `calloc(c, n)` | zeroed bytes for `c` elements | still check failure and later free |
| `realloc(p, n)` | preserved prefix on success | use a temporary pointer; old `p` becomes invalid on success |
| `posix_memalign()` | requested valid alignment | inspect return code; release with `free()` |
| `mmap()` | page-granular region | check `MAP_FAILED`; later `munmap()` |
| `alloca()`/VLA | automatic stack lifetime | keep size small and bounded; never `free()` |

## Ownership and Lifetime

The critical question is not merely “where is the memory?” but also:

1. Who owns it?
2. Which operation releases it?
3. When do pointers to it become invalid?
4. Which threads or processes can see it?
5. What synchronization makes writes visible?

For example, a successful `realloc()` invalidates the old pointer even if the
new numeric address is identical. A `MAP_SHARED` region can transmit updates
between related processes, but synchronization is still needed to define when
the other process may safely consume them.

## Residency Is Not Allocation

- An address range can exist without every page being resident.
- `mincore()` reports a residency snapshot, not a future guarantee.
- `mlock()` asks Linux to keep pages resident, subject to privilege and limits.
- Overcommit can allow a reservation before all backing resources are assured.
- Successful `malloc()` is therefore not proof that every later page touch is
  guaranteed to succeed under system-wide memory pressure.

## Safe Rules to Carry Forward

- Check `count * element_size` before allocating.
- Never read bytes before initializing them.
- Never use a pointer after `free()` or successful `realloc()`.
- Use `memmove()`, not `memcpy()`, for overlapping ranges.
- Compare `memcmp()` results by sign, not by expecting exactly `-1` or `1`.
- Do not mix direct program-break manipulation with the normal allocator.
- Keep stack allocations small and derived only from trusted bounds.
- Treat allocator internals, mapping addresses, and residency as observations,
  not stable API promises.
