# Chapter 9 Study Guide - Memory Management

Book: Robert Love, *Linux System Programming*, second edition

Scope: Chapter 9, printed pages 293-331

Repository: `linux_system_programming_lab`

This guide is an original explanation and laboratory companion. It summarizes
the chapter's ideas in new language and connects them to runnable C programs;
it does not reproduce the book.

## How to Use This Chapter

Do not run every program mechanically. For each exercise:

1. read the purpose and Linux-behavior comment at the top of the source;
2. predict what is guaranteed and what may vary;
3. compile and run it;
4. compare the output with the prediction;
5. trace an important program when a kernel transition matters;
6. explain the result without relying only on the printed `yes` fields.

Build and test the chapter from the repository root:

~~~bash
make chapter CHAPTER=09
make test CHAPTER=09
make check-structure
~~~

The complete source programs live in:

~~~text
chapters/09-memory-management/exercises/
chapters/09-memory-management/experiments/
~~~

The executables appear in the matching directories under `build/`.

## Chapter Summary

Every Linux process runs inside a virtual address space. The addresses stored
in C pointers identify locations in that virtual space; they are not normally
physical RAM addresses. Linux divides the address space into regions and pages,
gives regions permissions and backing, and resolves many pages only when the
process first accesses them.

The familiar C allocation functions sit above this virtual-memory machinery.
`malloc()` and friends manage blocks for the program. The allocator may obtain
larger areas from Linux by changing the data segment, creating anonymous
mappings, or reusing memory it already owns. A correct program must rely on the
allocator's documented contract, not on a guessed internal strategy.

Memory has several separate properties:

- **addressability:** does a virtual region exist?
- **permission:** may this process read, write, or execute it?
- **initialization:** have the bytes acquired a defined value?
- **residency:** is the page currently present in physical memory?
- **ownership:** which component must release it, and how?
- **visibility:** which threads or processes can observe updates?
- **lifetime:** when does every pointer to it become invalid?

Confusing these properties produces classic systems bugs. A successful
allocation is not the same as initialized storage. Freeing a block does not
guarantee an immediate kernel unmapping. A resident page is not automatically
locked. A valid pointer does not prove its target will remain valid after
`realloc()`.

The chapter's larger lesson is to reason in layers: program objects, allocator
blocks, virtual-memory regions, pages, and physical backing are connected, but
they are not interchangeable concepts.

## 1. The Process Address Space

### 1.1 Virtual addresses

The program sees a private virtual address space. The CPU's memory-management
unit translates virtual page numbers according to page tables maintained by
the kernel. This gives Linux several useful powers:

- isolate one process from another;
- map the same physical page into more than one address space;
- map files as memory;
- delay physical allocation until first access;
- protect code, data, and guard regions differently;
- relocate programs and libraries through ASLR.

Therefore, never infer physical placement from a pointer's numeric value.

### 1.2 Pages

A page is the normal unit of virtual-memory mapping and protection. Query the
runtime size instead of hard-coding 4096:

~~~c
long page_size = sysconf(_SC_PAGESIZE);
~~~

4096 bytes is common on x86-64 Linux, but it is not a universal C or POSIX
constant. Alignment, mapping length, residency vectors, and locking limits are
all naturally page-oriented.

### 1.3 Regions

A memory region is a contiguous virtual-address range with common properties.
A typical process contains executable code, read-only data, initialized data,
zero-initialized data, a heap, file and anonymous mappings, shared libraries,
and one or more thread stacks.

That description is a mental model, not a promise that every platform places
regions in a fixed order. ASLR deliberately changes addresses. Compare objects
for identity only where C permits it; do not rank unrelated pointers with `<`
or `>`.

Inspect the live mapping table with:

~~~bash
cat /proc/self/maps
./build/chapters/09-memory-management/experiments/proc_maps_inventory
~~~

Expected shape:

~~~text
mapping_count=<positive number> heap_mapping=yes stack_mapping=yes
count_positive=yes addresses_not_assumed=yes procfs_snapshot=yes
~~~

The exact count and every hexadecimal address may change between runs.

### 1.4 Page faults are not all errors

When a process first touches a valid demand-paged address, the CPU can trap
into the kernel. Linux finds that the access is permitted, supplies or maps the
page, updates the page table, and resumes the instruction. That recoverable
event is a normal page fault.

- A **minor fault** is resolved without reading the page from storage.
- A **major fault** requires storage I/O.
- An invalid or forbidden access cannot be resolved for the process and may
  produce `SIGSEGV` or `SIGBUS`.

Do not equate “page fault” with “segmentation fault.”

## 2. Dynamic Allocation

### 2.1 `malloc()`

~~~c
void *malloc(size_t size);
~~~

On success, `malloc()` returns storage suitably aligned for ordinary object
types. The bytes are uninitialized. Reading them before the program writes a
defined representation is a bug. On failure, it returns `NULL`.

A complete ownership sequence is:

1. calculate the byte count safely;
2. call `malloc()`;
3. check for `NULL`;
4. initialize before reading;
5. remain within the requested bounds;
6. call `free()` exactly once when ownership ends;
7. stop using every pointer to that allocation.

`free(NULL)` is permitted. Freeing an interior pointer, stack address, static
object, or already freed block is invalid.

### 2.2 Checked size calculations

For `count` objects of `element_size` bytes, multiplication can overflow before
the allocator sees it. The safe precondition is:

~~~c
if (count != 0 && element_size > SIZE_MAX / count) {
    /* reject the request */
}
~~~

If overflow wraps a huge request to a small byte count, later indexing can
write beyond the actual allocation. This is a correctness and security issue.

### 2.3 `calloc()`

~~~c
void *calloc(size_t count, size_t size);
~~~

`calloc()` allocates an array and initializes every byte to zero. It also
receives the count and element size separately, allowing conforming
implementations to detect a multiplication overflow. Zero bytes represent
integer zero and null pointers on the Linux targets used by this lab, but C
programs should still think about the representation required by each type.

### 2.4 `realloc()`

~~~c
void *realloc(void *pointer, size_t new_size);
~~~

`realloc()` may resize in place or move the data. On successful growth, the
prefix up to the old size is preserved and new tail bytes are uninitialized.
The old pointer becomes invalid on success even when the numeric address did
not change.

Use a temporary pointer:

~~~c
void *temporary = realloc(block, new_size);
if (temporary == NULL) {
    /* block still owns the original allocation */
} else {
    block = temporary;
}
~~~

Assigning directly to `block` loses the only pointer to the original storage
when the operation fails.

### 2.5 Alignment

Some data structures, device interfaces, vector instructions, and direct-I/O
paths require stronger alignment than ordinary `malloc()` promises.

~~~c
int result = posix_memalign(&pointer, alignment, size);
~~~

The alignment must be a power of two and a multiple of `sizeof(void *)`.
Unlike many functions, `posix_memalign()` returns an error number directly; it
does not report failure through `errno`. A successful pointer is released with
ordinary `free()`.

### 2.6 Allocator implementation details

The allocator commonly manages a heap and can also use anonymous mappings.
Its thresholds, arenas, caches, metadata, and reuse rules may change. Do not
write correctness logic that assumes:

- every small allocation changes the program break;
- every large allocation creates one visible `mmap()`;
- `free()` immediately shrinks the process;
- the requested size equals the allocator's internal usable size;
- a later allocation receives or does not receive a previous address.

GNU interfaces such as `malloc_usable_size()` and `malloc_trim()` are useful
for observation or specialized tuning, but they are not portable C APIs.

## 3. The Data Segment and Program Break

Historically, a process grows its data segment by moving the program break.
`brk()` sets it and `sbrk()` adjusts or queries it. The Chapter 9 exercise uses
only `sbrk(0)` to inspect the current break.

Directly changing the break in a program that also uses `malloc()` risks
corrupting the allocator's view of its heap. The safe rule for ordinary
programs is simple: let the allocator own this mechanism.

The program break is also not synonymous with all dynamic memory. Modern
allocators may use anonymous mappings and other policies.

## 4. Anonymous Memory Mappings

An anonymous mapping creates a virtual region not backed by an ordinary file:

~~~c
void *region = mmap(NULL,
                    length,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1,
                    0);
~~~

Check against `MAP_FAILED`, not `NULL`. New anonymous bytes read as zero. The
mapping is released with `munmap(region, length)`, not `free()`.

### Private versus shared

- `MAP_PRIVATE` gives copy-on-write behavior. Changes are private to the
  process's mapping.
- `MAP_SHARED` makes updates visible through other mappings of the same shared
  object. An anonymous shared mapping created before `fork()` can communicate
  between parent and child.

Visibility does not replace synchronization. The Chapter 9 shared-mapping
exercise uses `waitpid()` so the parent reads only after the child has written
and exited.

### Advice

`madvise()` tells Linux how the program expects to use a mapping or that
contents are no longer needed. Advice can affect performance and reclamation;
most advice does not change the C-level ownership rule.

On Linux, `MADV_DONTNEED` applied to a private anonymous page discards its
current contents. A later read observes the mapping's zero-fill behavior.

## 5. Stack Allocation

Automatic local objects normally live in a thread's stack frame. `alloca()`
and variable-length arrays extend that idea to runtime-sized storage.

Advantages:

- allocation is very fast;
- lifetime ends automatically when the function or block returns;
- no explicit `free()` is required.

Risks:

- stack capacity is limited;
- allocation failure is not reported like `malloc()` failure;
- a large or attacker-controlled size can overflow the stack;
- `alloca()` is not ISO C, and VLA support is optional in later C standards;
- pointers must not escape the storage's lifetime.

Use stack allocation only for small, tightly bounded sizes. Use heap or mapped
storage when the size is large, externally controlled, or must outlive the
current scope.

## 6. Byte-Oriented Memory Operations

These functions operate on raw byte ranges. They do not know the logical type,
capacity, ownership, or lifetime of the objects passed to them.

### `memset()`

~~~c
void *memset(void *destination, int byte_value, size_t length);
~~~

`memset()` repeats the low unsigned-byte value. It does not assign a multi-byte
integer value to every array element. For example, filling an `int` array with
byte value 1 does not normally create integer value 1 in each element.

### `memcmp()`

~~~c
int memcmp(const void *left, const void *right, size_t length);
~~~

The result is negative, zero, or positive. Only the sign is specified. Do not
require exactly `-1` or `1`. Comparing structures byte-for-byte can be wrong
when padding bytes are indeterminate or when multiple representations express
the same logical value.

### `memcpy()` versus `memmove()`

`memcpy()` requires nonoverlapping ranges. If source and destination overlap,
use `memmove()`, which behaves as though it first preserved the source bytes.

### `memchr()`

`memchr()` searches exactly the supplied number of bytes. It can search binary
data containing zero bytes; it is not a string operation.

### `memfrob()`

GNU `memfrob()` applies a reversible fixed XOR to each byte. Applying it twice
restores the original. That makes it an illustration of byte transformation,
not cryptography. Never use it to protect secrets.

## 7. Residency and Locking

### `mincore()`

`mincore()` reports which pages in a mapping are resident at the instant of the
query. The address must be page aligned. One low bit per page in the result
vector indicates residency.

Residency is a snapshot. Unless a page is locked, Linux may reclaim it after
the call. Even before first access, a page's initial state can differ because
of kernel decisions, so the lab asserts only that the page is resident after a
deliberate touch.

### `mlock()` and `munlock()`

`mlock()` requests that pages stay resident. Uses include latency-sensitive
code and reducing the chance that sensitive material reaches swap. Locking is
limited by `RLIMIT_MEMLOCK` and privilege; containers can impose additional
policy.

The lab locks only one page. `EPERM`, `ENOMEM`, or `EAGAIN` is reported as an
environmental boundary instead of prompting for privilege or bypassing policy.

Locking memory is not a complete secret-handling strategy. Sensitive programs
must also consider core dumps, copies, registers, logs, compiler optimization
of erasure, hibernation, and access by privileged software.

## 8. Overcommit and the OOM Boundary

Linux may allow processes to reserve more virtual memory than the machine can
back simultaneously. This is overcommit. The system policy is exposed through
`/proc/sys/vm/overcommit_memory`:

- `0`: heuristic policy;
- `1`: always overcommit;
- `2`: strict accounting.

`overcommit_ratio` participates in strict commit-limit calculation. Container
limits, swap, workloads, and administrator policy also matter.

Consequences:

- a non-`NULL` allocation can reserve address space before all pages are
  physically committed;
- first access can be the moment when backing is needed;
- severe system pressure can lead Linux to invoke the OOM killer;
- an individual program must not test OOM behavior by irresponsibly exhausting
  a development machine.

This lab only reads the policy. It never attempts to trigger OOM.

## 9. Exercise Walkthroughs

Run these from the repository root after `make chapter CHAPTER=09`.
Variable values are shown with angle brackets.

### Exercise 09.01 - Page size

~~~bash
./build/chapters/09-memory-management/exercises/01_page_size
~~~

Expected shape:

~~~text
page_size=<positive power of two> pointer_size=<platform value>
page_size_positive=yes power_of_two=yes runtime_query_used=yes
~~~

Connection: mapping and protection interfaces operate on page boundaries, but
the program discovers the page size at runtime.

### Exercise 09.02 - Address-space regions

~~~bash
./build/chapters/09-memory-management/exercises/02_address_space_regions
~~~

Expected stable properties:

~~~text
initialized_global=17 zero_initialized_global=0 stack=29 heap=41
regions_distinct=yes address_order_not_assumed=yes one_virtual_address_space=yes
~~~

The source takes representative addresses but deliberately avoids printing or
ordering them. ASLR makes fixed addresses inappropriate test expectations.

### Exercise 09.03 - `malloc()` array

~~~bash
./build/chapters/09-memory-management/exercises/03_malloc_array
~~~

~~~text
elements=8 bytes=32 sum=36
every_element_initialized=yes ownership_released=yes sum_correct=yes
~~~

Notice the order: allocate, check, initialize, use, free.

### Exercise 09.04 - `calloc()`

~~~bash
./build/chapters/09-memory-management/exercises/04_calloc_zeroed
~~~

~~~text
elements=8 initially_zero=yes assigned_sum=28
count_and_element_size_supplied=yes allocation_released=yes
~~~

The initial zero check is valid because `calloc()` defines those bytes.

### Exercise 09.05 - Safe `realloc()`

~~~bash
./build/chapters/09-memory-management/exercises/05_realloc_preserves_data
~~~

~~~text
old_count=4 new_count=8 prefix_preserved=yes sum=36
temporary_pointer_used=yes new_tail_initialized=yes
~~~

The new tail is initialized explicitly before it is read.

### Exercise 09.06 - Checked allocation size

~~~bash
./build/chapters/09-memory-management/exercises/06_checked_allocation
~~~

~~~text
overflow_rejected=yes errno_is_enomem=yes safe_allocation=yes
multiplication_checked_before_allocator=yes
~~~

The deliberately impossible request is rejected mathematically; no enormous
allocation is attempted.

### Exercise 09.07 - Explicit alignment

~~~bash
./build/chapters/09-memory-management/exercises/07_posix_memalign
~~~

~~~text
alignment=64 size=256 aligned=yes
power_of_two_alignment=yes ordinary_free_used=yes
~~~

Observe that the return value, not `errno`, is checked.

### Exercise 09.08 - Program break

~~~bash
./build/chapters/09-memory-management/exercises/08_program_break
~~~

~~~text
program_break_query=yes address_nonnull=yes
legacy_interface=yes direct_break_change_avoided=yes allocator_should_manage_heap=yes
~~~

This is intentionally observational. It does not compete with `malloc()` for
control of the data segment.

### Exercise 09.09 - Anonymous mapping

~~~bash
./build/chapters/09-memory-management/exercises/09_anonymous_mmap
~~~

~~~text
page_bytes=<runtime page size> initially_zero=yes endpoints_writable=yes
file_backing=no release_with_munmap=yes
~~~

Trace the mapping lifecycle:

~~~bash
chapters/09-memory-management/scripts/trace.sh 09_anonymous_mmap
grep -E 'mmap|munmap|write' \
  chapters/09-memory-management/observations/strace/09_anonymous_mmap.strace
~~~

Expect loader mappings as well as the mapping created by the exercise.

### Exercise 09.10 - Shared mapping across `fork()`

~~~bash
./build/chapters/09-memory-management/exercises/10_shared_anonymous_mapping
~~~

~~~text
child_exit_ok=yes parent_observed=42 shared_update_visible=yes
mapping_shared=yes wait_provided_process_synchronization=yes
~~~

Compare this with Chapter 5 copy-on-write memory. The mapping flag deliberately
changes the visibility rule.

### Exercises 09.11 and 09.12 - Allocator extensions

~~~bash
./build/chapters/09-memory-management/exercises/11_malloc_usable_size
./build/chapters/09-memory-management/exercises/12_malloc_trim
~~~

Expected invariants:

~~~text
requested=37 usable_at_least_requested=yes
only_requested_bytes_used=yes usable_size_is_allocator_specific=yes

allocation_bytes=4194304 malloc_trim_result=<0 or 1>
trim_called=yes zero_is_not_failure=yes allocator_decides_release=yes
~~~

The first program does not treat allocator padding as application-owned
capacity. The second does not treat a zero trim result as an error.

### Exercises 09.13 and 09.14 - Bounded stack storage

~~~bash
./build/chapters/09-memory-management/exercises/13_alloca_scope
./build/chapters/09-memory-management/exercises/14_variable_length_array
~~~

~~~text
stack_bytes=64 contents_ok=yes
scope_bound=yes explicit_free_required=no large_sizes_avoided=yes

runtime_count=8 sum=36 sum_correct=yes
bounded_input=yes block_lifetime=yes portability_considered=yes
~~~

Both sizes are deliberately small and fixed by the demonstration. Do not
replace them with an unchecked command-line value.

### Exercises 09.15-09.17 - Raw byte operations

~~~bash
./build/chapters/09-memory-management/exercises/15_memset_memcmp
./build/chapters/09-memory-management/exercises/16_memmove_overlap
./build/chapters/09-memory-management/exercises/17_memchr_search
~~~

~~~text
equal_before=yes different_after=yes compared_bytes=16
comparison_sign_contract_used=yes raw_bytes_compared=yes

result=ababcd expected=ababcd
overlap_handled=yes memmove_required=yes

target_found=yes index=3 missing_found=no
embedded_zero_safe=yes bounded_search=yes
~~~

These programs demonstrate three distinct contracts: byte fill/ordering,
overlap-safe movement, and bounded binary search.

### Exercise 09.18 - Residency

~~~bash
./build/chapters/09-memory-management/exercises/18_mincore_residency
~~~

~~~text
resident_before=<yes or no> resident_after_touch=yes
initial_state_not_assumed=yes page_granular_query=yes
~~~

Only the post-touch property is deterministic enough for the smoke test.

### Exercise 09.19 - One-page locking

~~~bash
./build/chapters/09-memory-management/exercises/19_mlock_page
~~~

Typical allowed case:

~~~text
mlock_supported=yes page_locked=yes page_unlocked=yes
soft_limit_infinite=<yes or no> soft_limit_bytes=<value> page_bytes=<value>
secret_data_not_used=yes
~~~

A restricted environment can instead report:

~~~text
mlock_supported=no constraint_errno=<value>
environmental_boundary=yes
...
secret_data_not_used=yes
~~~

Both cases teach something real about the current Linux environment.

## 10. Experiments

### 10.1 Demand paging

~~~bash
./build/chapters/09-memory-management/experiments/demand_paging_faults
~~~

Expected shape:

~~~text
pages_touched=128 minor_fault_delta=<positive> major_fault_delta=<value>
minor_faults_increased=yes major_fault_count_not_assumed=yes
~~~

The exact delta can vary. The experiment prevents transparent huge pages when
that advice is available, making the page-by-page effect easier to observe.

### 10.2 Allocator strategy

~~~bash
./build/chapters/09-memory-management/experiments/malloc_heap_vs_mmap
~~~

The two `break_..._changed` fields can be either `yes` or `no`. The stable
conclusion is:

~~~text
allocations_succeeded=yes
allocator_strategy_is_not_an_api_contract=yes
~~~

Use tracing to inspect one run, not to convert that run into a universal rule.

### 10.3 `realloc()` movement

~~~bash
./build/chapters/09-memory-management/experiments/realloc_movement
~~~

~~~text
grown_bytes=8388608 data_preserved=yes allocation_moved=<yes or no>
movement_is_allowed=yes old_pointer_not_reused=yes
~~~

Whether it moved is observational. Preserved bytes are the contract.

### 10.4 Discarding anonymous contents

~~~bash
./build/chapters/09-memory-management/experiments/madvise_dontneed
~~~

~~~text
page_bytes=<runtime page size> zero_after_dontneed=yes
linux_private_anonymous_semantics_observed=yes
~~~

This expected result is Linux-specific and applies to the mapping type used by
the program.

### 10.5 Reversible transformation

~~~bash
./build/chapters/09-memory-management/experiments/memfrob_roundtrip
~~~

~~~text
changed_after_once=yes restored_after_twice=yes
obfuscation_only=yes cryptography=no
~~~

The second line is the most important result.

### 10.6 Overcommit inventory

~~~bash
./build/chapters/09-memory-management/experiments/overcommit_inventory
~~~

~~~text
overcommit_mode=<0, 1, or 2> overcommit_ratio=<nonnegative value>
mode_valid=yes ratio_nonnegative=yes read_only_probe=yes
allocation_success_is_not_physical_memory_commitment=yes
~~~

Do not change system policy merely to make an example display another value.

## 11. Common Mistakes

### Mistake: reading freshly allocated `malloc()` bytes

Why it fails: their values are indeterminate. Initialize them first or use
`calloc()` when zero bytes are appropriate.

### Mistake: allocating `count * sizeof(element)` without overflow checking

Why it fails: unsigned `size_t` arithmetic can wrap, producing an allocation
smaller than the indexing logic expects.

### Mistake: overwriting the only pointer with `realloc()`

Why it fails: failure returns `NULL` while leaving the old block allocated. A
direct assignment leaks it.

### Mistake: continuing to use aliases after `realloc()` succeeds

Why it fails: every old pointer to that allocation becomes invalid, even when
the storage stayed at the same numeric address.

### Mistake: assuming `free()` immediately lowers RSS

Why it fails: the allocator can retain blocks for reuse; Linux can also account
and reclaim pages independently.

### Mistake: checking `mmap()` against `NULL`

Why it fails: the failure sentinel is `MAP_FAILED`.

### Mistake: releasing a mapping with `free()`

Why it fails: allocator blocks and mappings have different ownership APIs. Use
`munmap()` for `mmap()` regions.

### Mistake: mixing `brk()`/`sbrk()` mutations with `malloc()`

Why it fails: the allocator expects to control the heap layout and metadata.

### Mistake: using an unbounded VLA or `alloca()`

Why it fails: stack exhaustion may occur before the program can report a normal
allocation error.

### Mistake: using `memcpy()` for overlapping ranges

Why it fails: overlap violates its contract. Use `memmove()`.

### Mistake: expecting `memcmp()` to return exactly `-1` or `1`

Why it fails: only negative, zero, and positive are specified.

### Mistake: treating a `mincore()` result as permanent

Why it fails: residency can change immediately after the snapshot unless other
mechanisms constrain reclamation.

### Mistake: treating `mlock()` failure as a reason to run the whole lab as root

Why it fails: privilege and resource limits are part of the behavior being
studied. Record the boundary; do not bypass it.

### Mistake: trying to demonstrate OOM by consuming all memory

Why it fails: it can destabilize the machine and kill unrelated processes.
Study policy and bounded experiments instead.

## 12. Exercises for You

Complete these in order. Keep new work inside the Chapter 9 directories and
generated files under `build/chapters/09-memory-management/`.

### Exercise A - Predict region properties

Before running `02_address_space_regions`, write down which values should be
zero-initialized, which object requires `free()`, and why address ordering is
not tested. Then run it five times and compare only the guaranteed fields.

### Exercise B - Add a checked array helper

Write a function that allocates `count` objects of `element_size`, rejects
overflow, and clearly documents who owns the result. Test zero count, a small
valid request, and a deliberate overflow without attempting a huge allocation.

### Exercise C - Observe `realloc()` without depending on movement

Run `realloc_movement` ten times. Record whether it moved each time. Explain why
the test still passes in either case and identify the only pointer used after
success.

### Exercise D - Inspect maps while paused

Add an optional pause to a copy of `09_anonymous_mmap`, run it in one terminal,
and inspect `/proc/PID/maps` in another. Locate the new writable anonymous
region. Remove the pause after recording the observation.

### Exercise E - Compare allocation and page touch

Trace `demand_paging_faults`, inspect `getrusage()` output, and explain why the
`mmap()` call and the first writes represent different stages of memory use.

### Exercise F - Build an overlap table

For several source/destination ranges in the same array, decide whether
`memcpy()` is permitted. Verify the overlapping cases with `memmove()` and
draw the final byte sequence by hand before running.

### Exercise G - Inspect resource limits

Run:

~~~bash
ulimit -l
cat /proc/self/limits | grep -i 'locked memory'
./build/chapters/09-memory-management/exercises/19_mlock_page
~~~

Relate the shell value, procfs entry, and program result. Do not raise the limit
for the exercise.

### Exercise H - Explain overcommit safely

Read the two sysctls used by `overcommit_inventory`. Explain each mode and why
the observed mode alone cannot predict whether a particular process will be
terminated under future memory pressure.

### Exercise I - Add sanitizer runs

Compile one ownership exercise manually with AddressSanitizer and Undefined
Behavior Sanitizer. Then create a temporary intentionally broken copy that
uses a block after `free()` and observe the diagnostic. Do not commit the
broken source.

Example compiler form:

~~~bash
clang -std=c17 -g3 -O1 -fsanitize=address,undefined \
  chapters/09-memory-management/exercises/03_malloc_array.c \
  -o build/chapters/09-memory-management/malloc_array_sanitized
~~~

### Exercise J - Connect to a runtime system

Write one page explaining how a language runtime could use:

- pages for heap organization;
- aligned blocks for object metadata;
- mappings for large objects;
- protection changes for JIT-generated code;
- residency and locking for latency-sensitive structures;
- checked arithmetic to defend allocator metadata.

## 13. Review Questions

1. Why is a C pointer normally not a physical address?
2. What separates a memory region from a page?
3. Why can a normal access cause a recoverable page fault?
4. What is the difference between allocation and initialization?
5. What must be checked before multiplying array dimensions?
6. Why is a temporary pointer required around `realloc()`?
7. What happens to old aliases after successful `realloc()`?
8. How does `calloc()` differ from `malloc()`?
9. Which alignment values are valid for `posix_memalign()`?
10. Why should ordinary programs not manipulate the break beside `malloc()`?
11. Why is `MAP_FAILED` different from `NULL`?
12. When is `MAP_SHARED` useful, and what does it not provide by itself?
13. When is stack allocation inappropriate?
14. Why is `memmove()` required for overlap?
15. What does the sign of `memcmp()` mean?
16. What does `mincore()` report, and for how long is that guaranteed?
17. Which policies can make `mlock()` fail?
18. Why can successful allocation precede real backing commitment?
19. Why does the lab inspect OOM policy instead of triggering OOM?
20. Which Chapter 9 facts are API guarantees and which are observations?

## 14. Completion Checklist

Before moving to Chapter 10, confirm that you can:

- explain virtual memory, regions, pages, translations, and faults;
- distinguish code, static data, heap, mappings, and stacks;
- use `malloc()`, `calloc()`, `realloc()`, and `free()` safely;
- prevent allocation-size overflow;
- request and validate stronger alignment;
- explain why allocator strategy is not an application contract;
- create private and shared anonymous mappings;
- choose between heap, mapping, and bounded stack storage;
- use raw byte operations according to their exact contracts;
- distinguish residency from locking;
- explain resource-limit and privilege boundaries around `mlock()`;
- explain overcommit without performing a destructive stress test;
- pass `make test CHAPTER=09`;
- collect and interpret at least two Chapter 9 traces;
- write your own explanation of what surprised you.

When all of those are true, the lab has become understanding rather than only
source code.
