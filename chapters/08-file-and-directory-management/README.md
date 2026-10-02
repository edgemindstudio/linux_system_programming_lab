# Chapter 8 - File and Directory Management

**Status:** Lab prepared; study in progress

## Purpose

Understand that Linux pathnames are names in directories, while inodes hold
the identity and metadata of filesystem objects. The chapter connects that
model to permissions, ownership, extended attributes, directory traversal,
links, unlinking, copying, renaming, device nodes, ioctl, and inotify.

## Book Scope

Robert Love, *Linux System Programming*, second edition, Chapter 8, printed
pages 241-292.

## Study Material

- `notes/study-guide-pages-241-292.md`
- `mental-models/file-and-directory-management.md`

## Exercises

1. Read inode metadata with `stat()`
2. Compare `stat()` with `lstat()`
3. Decode file type bits in `st_mode`
4. Change permissions through an open descriptor
5. Inspect ownership and access identities
6. Complete an extended-attribute lifecycle
7. Change and restore the current working directory
8. Apply `umask` while creating a directory
9. Observe `rmdir()` on a nonempty directory
10. Read directory entries without assuming order
11. Create two names for one inode
12. Store and inspect a symbolic-link pathname
13. Keep using an inode after unlinking its name
14. Rename a directory entry without copying data
15. Copy data into a distinct inode
16. Read randomness through a character device
17. Receive an inotify create event
18. Remove a watch and inspect the pending queue

## Experiments

- follow an open descriptor across `rename()`;
- observe hard-link counts fall from three to one;
- use a directory descriptor after its pathname changes;
- distinguish atime, mtime, and ctime;
- reject final symbolic-link traversal with `O_NOFOLLOW`;
- correlate both halves of an inotify move with a cookie;
- demonstrate that an inotify directory watch is not recursive.

## Commands

~~~bash
make chapter CHAPTER=08
make test CHAPTER=08
make tidy-chapter CHAPTER=08
chapters/08-file-and-directory-management/scripts/trace.sh 01_stat_metadata
chapters/08-file-and-directory-management/scripts/trace.sh inotify_rename_cookie
~~~

Generated files are placed under:

~~~text
build/chapters/08-file-and-directory-management/
~~~

## Safety Choices

All writable paths stay under the disposable Chapter 8 build-data directory.
The lab never changes ownership, permissions, links, or names outside that
directory. It reads `/dev/urandom` without printing random material, treats
extended attributes as an optional filesystem capability, does not require
root, and gives every blocking inotify operation a deterministic triggering
event before the read.

## Completion Rule

Building the lab is not the same as mastering it. Before moving to Chapter 9,
explain directory entries versus inodes, `stat()` versus `lstat()`, file type
and permission bits, real versus effective credentials, xattr namespaces,
directory-stream limitations, hard versus symbolic links, unlink lifetime,
copy versus rename semantics, device-node dispatch, ioctl's purpose, and the
full inotify instance/watch/event lifecycle.
