# Chapter 9 - Memory Management

**Status:** Lab prepared; study in progress

## Purpose

Understand a process address space as a collection of page-backed regions and
learn how the C allocator, the program break, anonymous mappings, stack
storage, memory operations, residency, locking, and Linux overcommit fit into
that model.

## Book Scope

Robert Love, *Linux System Programming*, second edition, Chapter 9, printed
pages 293-331.

## Study Material

- `notes/study-guide-pages-293-331.md`
- `mental-models/memory-management.md`

## Exercises

1. Query the runtime page size
2. Locate representative address-space regions
3. Allocate and initialize an array with `malloc()`
4. Request zero-filled storage with `calloc()`
5. Grow an allocation safely with `realloc()`
6. Reject an allocation-size overflow
7. Request explicit alignment with `posix_memalign()`
8. Inspect the program break without changing it
9. Create and release an anonymous mapping
10. Share an anonymous mapping across `fork()`
11. Inspect allocator-specific usable size safely
12. Ask the allocator to trim unused heap space
13. Use a small, bounded `alloca()` allocation
14. Use a bounded variable-length array
15. Set and compare raw bytes
16. Move overlapping bytes correctly
17. Search a bounded byte region
18. Query page residency with `mincore()`
19. Lock one page while respecting `RLIMIT_MEMLOCK`

## Experiments

- inventory `/proc/self/maps` without assuming addresses;
- correlate first page touches with minor page faults;
- observe that allocator strategy is not an API contract;
- allow `realloc()` to move storage while preserving bytes;
- discard anonymous pages with `MADV_DONTNEED`;
- show that `memfrob()` is reversible obfuscation, not encryption;
- inspect Linux overcommit policy without changing it.

## Commands

~~~bash
make chapter CHAPTER=09
make test CHAPTER=09
make tidy-chapter CHAPTER=09
chapters/09-memory-management/scripts/trace.sh 09_anonymous_mmap
chapters/09-memory-management/scripts/trace.sh demand_paging_faults
~~~

Generated files are placed under:

~~~text
build/chapters/09-memory-management/
~~~

## Safety Choices

The lab uses bounded allocations and never attempts to trigger the OOM killer.
It queries `sbrk(0)` but does not change the program break, locks at most one
page, treats lock failure caused by policy or resource limits as an observable
boundary, and reads overcommit policy without modifying sysctls. Programs do
not read uninitialized or freed storage.

## Completion Rule

Building the lab is not the same as mastering it. Before moving to Chapter 10,
explain virtual versus physical memory, pages and regions, allocator ownership,
checked sizing, alignment, `realloc()` invalidation, program-break versus
mapping mechanisms, stack-allocation risks, byte-operation contracts, demand
paging, residency versus locking, overcommit, and why successful allocation
does not prove that future memory access cannot fail.
