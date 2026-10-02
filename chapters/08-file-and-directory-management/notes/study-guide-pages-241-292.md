# Chapter 8 Study Guide - File and Directory Management

Book: Robert Love, *Linux System Programming*, second edition
Scope: Chapter 8, printed pages 241-292
Repository: `linux_system_programming_lab`

This is an original learning companion. It explains the chapter through the
complete programs in this repository and does not reproduce the book.

## 1. Chapter Goal

Chapter 8 moves from reading and writing file contents to managing the names,
metadata, and relationships that make up a filesystem namespace.

The most important distinction is:

~~~text
pathname -> directory entry -> inode -> file data or device implementation
~~~

A pathname is how a process asks Linux to find an object. The inode is the
filesystem object's identity and stores most of its metadata. A directory
connects a name to an inode. A file descriptor refers to an opened object; it
does not depend on that object's pathname continuing to exist.

Once that model is clear, several Linux behaviors stop being surprising:

- two hard-link names can identify the same inode;
- a symbolic link has its own inode and stores another pathname;
- renaming a file changes the namespace without changing an open descriptor;
- unlinking the last name does not destroy an inode that is still open;
- copying creates a different inode;
- directory iteration returns names, not a guaranteed order;
- inotify reports namespace activity through a file descriptor.

## 2. Chapter Summary

Robert Love highlights a progression from metadata to event observation:

- `stat()`, `fstat()`, and `lstat()` obtain a snapshot of inode metadata.
- The `st_mode` field combines the file type with permission and special bits.
- Real and effective credentials help Linux decide whether an operation is
  permitted.
- Ownership, mode bits, and extended attributes are separate metadata layers.
- Every process has a current working directory that affects relative paths.
- Directory streams let programs enumerate entries, but their ordering and
  optional type hints must not be treated as guarantees.
- Hard links add names for one inode; symbolic links contain pathnames.
- `unlink()` removes a name, while the kernel keeps an open inode alive.
- `rename()` changes names within a filesystem; a copy creates new data and a
  new inode and therefore needs an explicit metadata policy.
- Device nodes use normal filesystem names to reach kernel device drivers.
- `ioctl()` carries device- or object-specific control requests outside the
  ordinary byte-stream model.
- inotify exposes filesystem events as variable-length records read from a
  descriptor.

## 3. The Core Mental Model

### 3.1 Pathname

A pathname is a sequence of directory-entry names. It can be absolute, such
as `/etc/hosts`, or relative to the process's current working directory.

The pathname is not the file object. It is an instruction for finding one.

### 3.2 Directory entry

A directory stores mappings from names to inode numbers. The name belongs to
the containing directory. Renaming a file therefore changes directory data.

### 3.3 Inode

An inode is the filesystem's identity and metadata record for an object. It
normally records information such as:

- object type;
- permission and special bits;
- owner and group IDs;
- link count;
- byte size;
- timestamps;
- device information;
- references to the object's data.

The filename is not stored as the inode's identity. One inode may have several
hard-link names.

### 3.4 Open file description and file descriptor

Opening a pathname creates or reuses kernel state that identifies the opened
object, its access mode, and its current file offset. The process receives a
small integer file descriptor referring to that state.

After `open()` succeeds, later namespace changes do not silently retarget that
descriptor. It keeps referring to the object that was opened.

### 3.5 A useful comparison

| Concept | What it represents | Can change independently? |
|---|---|---|
| Pathname | Route through directory names | Yes; entries can be renamed or removed |
| Directory entry | One name-to-inode association | Yes; links and renames change it |
| Inode | Filesystem object identity and metadata | Metadata and link count can change |
| File descriptor | Process-local handle to an opened object | Remains valid across rename/unlink |

## 4. Reading Metadata

### 4.1 `stat()`

~~~c
int stat(const char *path, struct stat *result);
~~~

`stat()` resolves a pathname and follows a final symbolic link. On success it
fills a `struct stat` with a metadata snapshot.

### 4.2 `fstat()`

~~~c
int fstat(int fd, struct stat *result);
~~~

`fstat()` asks about the object already referenced by a descriptor. It avoids
resolving the pathname again and is usually the stronger choice after a file
has been opened.

### 4.3 `lstat()`

~~~c
int lstat(const char *path, struct stat *result);
~~~

`lstat()` behaves like `stat()` except that, when the final pathname component
is a symbolic link, it reports the link itself.

### 4.4 Important `struct stat` fields

| Field | Meaning |
|---|---|
| `st_dev` | Device containing the inode |
| `st_ino` | Inode number on that device |
| `st_mode` | File type, permissions, and special bits |
| `st_nlink` | Number of hard links |
| `st_uid` | Owner user ID |
| `st_gid` | Owner group ID |
| `st_size` | Logical size in bytes for a regular file |
| `st_atim` | Last access timestamp on Linux/POSIX systems exposing it |
| `st_mtim` | Last content-modification timestamp |
| `st_ctim` | Last inode-status-change timestamp |

An inode number is unique only within its filesystem device. A robust identity
comparison therefore uses both `st_dev` and `st_ino`.

### 4.5 Metadata is a snapshot

The kernel can change an object's metadata immediately after `stat()` returns.
Do not treat a pathname check followed by a pathname operation as one atomic
decision. That pattern can create a time-of-check/time-of-use race.

When possible:

1. open the object with the required safety flags;
2. use `fstat()` on the resulting descriptor;
3. continue operating through that descriptor.

## 5. File Types and Mode Bits

`st_mode` contains two categories of information:

~~~text
file-type bits | permission and special bits
~~~

Use the standard macros rather than comparing raw numeric constants:

- `S_ISREG(mode)` - regular file;
- `S_ISDIR(mode)` - directory;
- `S_ISLNK(mode)` - symbolic link;
- `S_ISFIFO(mode)` - FIFO;
- `S_ISCHR(mode)` - character device;
- `S_ISBLK(mode)` - block device;
- `S_ISSOCK(mode)` - socket.

Extract ordinary permissions with:

~~~c
mode_t permissions = information.st_mode & 07777;
~~~

The leading zero makes the literal octal. Filesystem permissions are commonly
written in octal because each group of three bits maps naturally to a digit.

## 6. Permissions

### 6.1 Regular-file permissions

For a regular file:

- read permits reading its data;
- write permits changing its data;
- execute permits attempting to execute it as a program.

### 6.2 Directory permissions

Directory permissions mean something different:

- read permits listing directory-entry names;
- write permits adding or removing entries, subject to other rules;
- execute permits searching or traversing the directory.

A process may know a filename but still be unable to reach it when it lacks
search permission on a directory in the path.

### 6.3 Changing mode

~~~c
int chmod(const char *path, mode_t mode);
int fchmod(int fd, mode_t mode);
~~~

`fchmod()` changes the opened inode and avoids looking the name up again. It is
especially useful when the program has already created the file safely.

### 6.4 Special bits

The mode also has set-user-ID, set-group-ID, and sticky bits. Their effect
depends on the object type and mount policy. The sticky bit is especially
important on shared writable directories such as `/tmp`: it restricts who may
remove or rename entries.

### 6.5 `umask`

The process file-creation mask removes permission bits requested during
creation:

~~~text
effective mode = requested mode & ~umask
~~~

For example:

~~~text
requested directory mode 0777
umask                    0027
result                    0750
~~~

`umask()` changes process state and returns the previous mask. In a
multithreaded program, changing it temporarily can race with file creation in
another thread.

## 7. Ownership and Access Decisions

An inode records a numeric owner user ID and group ID. Processes also have
real, effective, and saved credential IDs.

In simplified terms:

- the real ID describes who started the process;
- the effective ID normally drives permission checks;
- the saved ID supports controlled privilege transitions.

Relevant interfaces include:

~~~c
uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int chown(const char *path, uid_t owner, gid_t group);
int fchown(int fd, uid_t owner, gid_t group);
int lchown(const char *path, uid_t owner, gid_t group);
~~~

Changing ownership is privilege-sensitive and can clear set-ID bits. The lab
therefore observes ownership without trying to acquire or mutate privileges.

`access()` answers a different question from simply attempting `open()`: it
checks access using the process's real IDs. Security-sensitive programs should
usually attempt the required operation directly with the intended effective
credentials instead of checking and then using the path later.

## 8. Extended Attributes

Extended attributes attach named byte strings to an inode. The common Linux
operations are:

- `setxattr()` or `fsetxattr()` - create or replace a value;
- `getxattr()` or `fgetxattr()` - obtain a value;
- `listxattr()` or `flistxattr()` - enumerate names;
- `removexattr()` or `fremovexattr()` - remove a name/value pair.

Attribute names have namespaces. The lab uses `user.lsp_lab`, which belongs to
the user namespace. Filesystems, mount options, permissions, or container
policy may disable user extended attributes, so a portable learning test must
distinguish “unsupported here” from “program logic failed.”

Never assume an extended attribute is a NUL-terminated string. The interface
returns a byte count.

## 9. Current Working Directory

Relative pathnames are resolved from the process's current working directory.

~~~c
char *getcwd(char *buffer, size_t size);
int chdir(const char *path);
int fchdir(int fd);
~~~

`fchdir()` makes a directory descriptor useful as a stable return point. The
lab opens the original directory, changes elsewhere, and restores the original
location through the descriptor.

The working directory is process-wide state. Library code that changes it can
surprise unrelated callers, and concurrent changes are particularly risky.
Prefer descriptor-relative interfaces such as `openat()` when practical.

## 10. Creating and Removing Directories

~~~c
int mkdir(const char *path, mode_t mode);
int rmdir(const char *path);
~~~

`mkdir()` applies the process umask to the requested permissions. `rmdir()`
removes an empty directory. A directory containing entries other than `.` and
`..` is not empty, so removal fails, commonly with `ENOTEMPTY`.

Do not respond to every failure by recursively deleting content. First decide
whether the program really owns that directory tree and whether destructive
removal is within its authorization.

## 11. Reading a Directory

The C library exposes directory streams:

~~~c
DIR *opendir(const char *path);
struct dirent *readdir(DIR *directory);
int closedir(DIR *directory);
~~~

Each successful `readdir()` call returns a pointer to library-managed storage.
Copy any name that must survive a later call.

Important rules:

- do not assume alphabetical or creation order;
- expect `.` and `..` unless explicitly handled;
- set `errno = 0` before a read loop if end-of-directory must be distinguished
  from error;
- do not require `d_type` to be informative, because it may be `DT_UNKNOWN`;
- join paths with bounds-aware code;
- close the directory stream.

Directory contents can change while they are being enumerated. A listing is
not an atomic snapshot unless a stronger application-level mechanism provides
that guarantee.

## 12. Hard Links

~~~c
int link(const char *oldpath, const char *newpath);
~~~

A hard link adds another directory-entry name for the same inode. Therefore:

- both names have the same `(st_dev, st_ino)` identity;
- both names expose the same file data;
- modifying through either name changes the same inode;
- `st_nlink` increases;
- removing one name leaves the other name intact.

Ordinary users cannot hard-link directories, and hard links cannot cross
filesystem boundaries because an inode number belongs to one filesystem.

## 13. Symbolic Links

~~~c
int symlink(const char *target, const char *linkpath);
ssize_t readlink(const char *path, char *buffer, size_t size);
~~~

A symbolic link is a separate inode whose contents are a pathname. The stored
pathname may be absolute or relative, may name a nonexistent target, and is
interpreted when the link is followed.

Key distinctions:

- `stat(link)` normally reports the target;
- `lstat(link)` reports the link inode;
- `readlink()` returns stored bytes and does not append a NUL byte;
- deleting the target can leave a dangling symbolic link;
- `O_NOFOLLOW` rejects a final symbolic-link component during `open()`.

Avoid reasoning about a symbolic link by checking it first and opening it
later. An attacker or another process may replace a path component between the
operations.

## 14. Unlinking and Object Lifetime

~~~c
int unlink(const char *path);
~~~

`unlink()` removes a directory entry and decrements the inode's link count. It
does not mean “immediately erase every byte associated with this pathname.”

Linux can reclaim the inode only after both conditions are true:

~~~text
hard-link count == 0
and
open-reference count == 0
~~~

This is why a process can continue reading and writing an open file after its
last pathname has been removed. The descriptor is still an active reference.

This behavior supports safe temporary files and log rotation, but it can also
explain apparently missing disk space: a large deleted file may remain open in
a process.

## 15. Renaming, Moving, and Copying

### 15.1 Rename

~~~c
int rename(const char *oldpath, const char *newpath);
~~~

Within one filesystem, `rename()` changes namespace associations rather than
copying file data. Open descriptors continue to identify the same inode.

A successful rename is atomic with respect to observers of the source and
destination names: programs do not see a partially copied file. Details such
as replacement rules depend on object types and directory permissions.

Across filesystems, `rename()` fails with `EXDEV`. A user-level “move” command
may then copy and unlink, which has different atomicity and failure behavior.

### 15.2 Copy

A copy normally:

1. opens the source;
2. creates a distinct destination inode;
3. transfers bytes with robust partial-read/partial-write loops;
4. applies an explicit metadata policy;
5. closes both descriptors and checks errors.

The destination does not automatically inherit every property. A real copy
tool must decide what to do with:

- mode bits;
- ownership;
- timestamps;
- extended attributes;
- access-control lists;
- sparse regions;
- hard-link relationships;
- special file types.

The lab preserves ordinary permission bits while deliberately creating a
distinct inode. That limited policy is stated rather than hidden.

## 16. Device Nodes and Randomness

A device node is a filesystem object that connects ordinary file operations to
a kernel driver. Its metadata includes a device type and major/minor numbers.

- the major number selects a driver or driver family;
- the minor number selects a particular device or instance.

Examples include:

- `/dev/null` - discards writes and immediately reaches EOF on reads;
- `/dev/zero` - supplies zero bytes;
- `/dev/full` - fails writes with `ENOSPC`;
- `/dev/urandom` - supplies kernel-generated random bytes.

The Chapter 8 lab reads 32 bytes from `/dev/urandom` and intentionally does
not print them. Random material can be sensitive, and raw values are not a
useful deterministic test result.

For cryptographic applications on current Linux systems, prefer a well-vetted
library or the modern `getrandom()` interface when appropriate. Avoid building
security decisions from simplified historical claims about entropy devices.

## 17. `ioctl()` and Out-of-Band Control

`read()` and `write()` transfer bytes. Some kernel objects need operations
that do not fit that model, such as querying terminal size or inspecting bytes
queued for reading.

~~~c
int ioctl(int fd, unsigned long request, ...);
~~~

The meaning and argument type depend on the request and object. This makes
`ioctl()` powerful but less uniform than ordinary I/O. Consult the request's
specific manual page and never guess the argument layout.

The inotify lifecycle exercise uses `FIONREAD` to ask how many event bytes are
currently queued on the inotify descriptor.

## 18. inotify

inotify is Linux's descriptor-based filesystem event facility.

### 18.1 Complete lifecycle

1. Create an instance with `inotify_init1()`.
2. Add one or more watches with `inotify_add_watch()`.
3. Cause or wait for relevant filesystem activity.
4. Read one or more `struct inotify_event` records.
5. Interpret each mask, watch descriptor, name, and cookie.
6. Remove watches with `inotify_rm_watch()` when appropriate.
7. Close the inotify descriptor.

### 18.2 Event layout

~~~c
struct inotify_event {
    int      wd;
    uint32_t mask;
    uint32_t cookie;
    uint32_t len;
    char     name[];
};
~~~

Records are variable length because `name` follows the fixed header. Iterate
by adding `sizeof(struct inotify_event) + event->len`, not merely the header
size.

### 18.3 Watches are not recursive

Watching a directory reports selected events directly associated with that
directory. It does not automatically watch every existing or future
subdirectory. A recursive watcher must discover subdirectories, add watches,
and handle races while the tree changes.

### 18.4 Move cookies

A rename within watched locations can produce `IN_MOVED_FROM` and
`IN_MOVED_TO`. A nonzero cookie can correlate the two halves. Event consumers
must still handle unmatched events caused by watch scope, queue timing, and
movement into or out of the observed set.

### 18.5 Watch removal

Removing a watch queues an `IN_IGNORED` event. A watch can also disappear when
the watched object is deleted or a filesystem is unmounted. Treat watch
descriptors as lifecycle-bound identifiers, not permanent object IDs.

### 18.6 Queue overflow

If the consumer cannot keep up, the queue may report `IN_Q_OVERFLOW`. At that
point events were lost. A robust program must rescan or otherwise rebuild its
state rather than pretending the event history is complete.

### 18.7 What inotify does not guarantee

inotify is a notification system, not a transaction log. An event says that
something happened. By the time the consumer responds, the name or object may
have changed again.

## 19. Exercise Walkthrough

Build all Chapter 8 programs first:

~~~bash
make chapter CHAPTER=08
~~~

### Exercise 08.01 - Read inode metadata

~~~bash
./build/chapters/08-file-and-directory-management/exercises/01_stat_metadata
~~~

Expected shape:

~~~text
regular=yes size=9 ...
inode_positive=yes size_matches=yes metadata_is_snapshot=yes
~~~

Connection: `stat()` gives a snapshot. The program validates properties rather
than expecting a particular inode number or timestamp.

### Exercise 08.02 - Compare `stat()` and `lstat()`

~~~bash
./build/chapters/08-file-and-directory-management/exercises/02_stat_vs_lstat
~~~

Expected:

~~~text
stat_follows_link=yes lstat_reports_link=yes
target_and_link_inodes_distinct=yes
~~~

Connection: one pathname operation follows the final link; the other inspects
the link inode.

### Exercise 08.03 - Decode file types

~~~bash
./build/chapters/08-file-and-directory-management/exercises/03_file_type_mode
~~~

Expected:

~~~text
regular_detected=yes directory_detected=yes fifo_detected=yes
~~~

Connection: use `S_IS*` macros on `st_mode`; do not infer type from filename
extensions.

### Exercise 08.04 - Change permissions through a descriptor

~~~bash
./build/chapters/08-file-and-directory-management/exercises/04_fchmod_permissions
~~~

Expected:

~~~text
requested=0640 observed=0640
permissions_changed=yes descriptor_target_stable=yes
~~~

Connection: the descriptor remains attached to the opened inode.

### Exercise 08.05 - Observe ownership and access identities

~~~bash
./build/chapters/08-file-and-directory-management/exercises/05_ownership_and_access
~~~

Expected properties include `owned_by_real_user=yes` and
`readable_writable=yes` in the lab's private data directory.

Connection: inode ownership and process credentials are numeric kernel state.

### Exercise 08.06 - Extended-attribute lifecycle

~~~bash
./build/chapters/08-file-and-directory-management/exercises/06_extended_attributes
~~~

Supported filesystems report:

~~~text
xattr_supported=yes value=metadata-lab listed=yes
removed=yes inode_metadata_extended=yes
~~~

An environment without user-xattr support reports `xattr_supported=no` and
exits successfully. That is a capability observation, not a false failure.

### Exercise 08.07 - Change and restore the working directory

~~~bash
./build/chapters/08-file-and-directory-management/exercises/07_current_working_directory
~~~

Expected:

~~~text
directory_changed=yes directory_restored=yes
descriptor_based_restore=yes
~~~

Connection: relative pathname resolution uses process state, while `fchdir()`
uses a previously opened directory.

### Exercise 08.08 - Apply `umask` to `mkdir()`

~~~bash
./build/chapters/08-file-and-directory-management/exercises/08_mkdir_umask
~~~

Expected:

~~~text
requested=0777 umask=0027 observed=0750
effective_mode_correct=yes
~~~

Connection: creation requests a maximum; the mask removes bits.

### Exercise 08.09 - Reject removal of a nonempty directory

~~~bash
./build/chapters/08-file-and-directory-management/exercises/09_rmdir_nonempty
~~~

Expected:

~~~text
nonempty_removal_rejected=yes empty_removal_succeeded=yes
~~~

Connection: removing the file entry first makes the directory empty.

### Exercise 08.10 - Read directory entries

~~~bash
./build/chapters/08-file-and-directory-management/exercises/10_read_directory
~~~

Expected:

~~~text
alpha=found beta=found gamma=found
all_expected_entries=yes order_assumed=no dtype_required=no
~~~

Connection: a correct scanner tests membership, not enumeration order.

### Exercise 08.11 - Create a hard link

~~~bash
./build/chapters/08-file-and-directory-management/exercises/11_hard_link
~~~

Expected:

~~~text
same_inode=yes first_links=2 second_links=2
two_names_one_inode=yes hard_link_not_copy=yes
~~~

Connection: the link count belongs to the shared inode.

### Exercise 08.12 - Create and inspect a symbolic link

~~~bash
./build/chapters/08-file-and-directory-management/exercises/12_symbolic_link
~~~

Expected:

~~~text
stored_path=symlink_target.txt path_matches=yes
link_is_separate_inode=yes stat_followed_target=yes
~~~

Connection: `readlink()` reads the stored pathname; `stat()` follows it.

### Exercise 08.13 - Unlink an open file

~~~bash
./build/chapters/08-file-and-directory-management/exercises/13_unlink_open_file
~~~

Expected:

~~~text
name_gone=yes link_count=0 data=still-readable
descriptor_kept_inode_alive=yes
~~~

Connection: namespace lifetime and open-reference lifetime are separate.

### Exercise 08.14 - Rename without copying

~~~bash
./build/chapters/08-file-and-directory-management/exercises/14_rename_file
~~~

Expected:

~~~text
old_name_gone=yes new_name_exists=yes same_inode=yes
namespace_changed_without_copy=yes
~~~

Connection: the same inode is reachable under a new name.

### Exercise 08.15 - Copy into a distinct inode

~~~bash
./build/chapters/08-file-and-directory-management/exercises/15_copy_file
~~~

Expected:

~~~text
sizes_match=yes distinct_inodes=yes
mode_preserved=yes metadata_policy_explicit=yes
~~~

Connection: copying data and preserving selected metadata are separate tasks.

### Exercise 08.16 - Read a character device

~~~bash
./build/chapters/08-file-and-directory-management/exercises/16_random_device
~~~

Expected:

~~~text
character_device=yes bytes_read=32
random_values_not_printed=yes driver_served_read=yes
~~~

Connection: a normal `read()` crosses the device node into a kernel driver.

### Exercise 08.17 - Receive an inotify create event

~~~bash
./build/chapters/08-file-and-directory-management/exercises/17_inotify_create_event
~~~

Expected:

~~~text
watch_descriptor_nonnegative=yes create_event_found=yes
event_records_variable_length=yes inotify_is_fd_based=yes
~~~

Connection: event notification uses the same descriptor/read model as other
Linux I/O, but the records have structured, variable-length contents.

### Exercise 08.18 - Complete an inotify lifecycle

~~~bash
./build/chapters/08-file-and-directory-management/exercises/18_inotify_lifecycle
~~~

Expected:

~~~text
pending_bytes_positive=yes ignored_event_found=yes
watch_removed=yes instance_closed=yes
~~~

Connection: `FIONREAD` queries queued bytes, and watch removal produces
`IN_IGNORED` before the instance is closed.

## 20. Experiment Walkthrough

### Open descriptor across rename

~~~bash
./build/chapters/08-file-and-directory-management/experiments/open_fd_across_rename
~~~

Observe that the descriptor's inode matches the new name while the old name is
gone. `/proc/self/fd` also shows how Linux describes the live reference.

### Link-count lifecycle

~~~bash
./build/chapters/08-file-and-directory-management/experiments/link_count_lifecycle
~~~

Expected `link_counts=3,2,1`. Removing one name decrements the count without
changing the data reachable through remaining names.

### Directory descriptor stability

~~~bash
./build/chapters/08-file-and-directory-management/experiments/directory_fd_stability
~~~

The experiment renames a directory and then uses its open descriptor. The
descriptor follows the directory object, not its old pathname.

### Metadata timestamps

~~~bash
./build/chapters/08-file-and-directory-management/experiments/metadata_timestamps
~~~

The important result is:

~~~text
ctime_is_status_change=yes ctime_is_creation_time=no
~~~

On traditional Unix interfaces, ctime is inode-change time, not creation time.

### Refuse final symlink traversal

~~~bash
./build/chapters/08-file-and-directory-management/experiments/symlink_nofollow
~~~

The default open follows the link; `O_NOFOLLOW` rejects it. This flag protects
only the final component, so secure path traversal may require stronger
descriptor-relative techniques.

### Pair inotify rename events

~~~bash
./build/chapters/08-file-and-directory-management/experiments/inotify_rename_cookie
~~~

The `IN_MOVED_FROM` and `IN_MOVED_TO` events carry the same nonzero cookie in
this controlled rename.

### Demonstrate nonrecursive watches

~~~bash
./build/chapters/08-file-and-directory-management/experiments/inotify_nonrecursive
~~~

The watched parent reports creation of a child directory but not creation of a
file inside that unwatched child.

## 21. Tracing Real Linux Behavior

The trace helper runs one Chapter 8 binary under `strace` and stores the trace
under the chapter's ignored observation directory.

~~~bash
chapters/08-file-and-directory-management/scripts/trace.sh 13_unlink_open_file

grep -E 'openat|unlink|fstat|read|close' \
  chapters/08-file-and-directory-management/observations/strace/13_unlink_open_file.strace
~~~

For inotify:

~~~bash
chapters/08-file-and-directory-management/scripts/trace.sh 17_inotify_create_event

grep -E 'inotify|openat|read|close' \
  chapters/08-file-and-directory-management/observations/strace/17_inotify_create_event.strace
~~~

The trace shows system calls, not C-library intent. A library function may
perform several calls, and some wrapper operations may be optimized or use a
different but equivalent syscall on a particular architecture.

## 22. Common Mistakes

1. **Treating a pathname as permanent identity.**
   Names can be renamed, removed, or replaced. Use a descriptor when the
   operation must remain attached to the opened object.

2. **Comparing only inode numbers.**
   Include `st_dev`; inode numbers repeat across filesystems.

3. **Using `stat()` when the link itself matters.**
   Use `lstat()` or an appropriate `*at()` interface and flags.

4. **Calling ctime “creation time.”**
   It records inode status change in these interfaces.

5. **Writing decimal permission literals.**
   Use octal, such as `0640`.

6. **Forgetting that directory execute means search.**
   Read permission alone does not guarantee pathname traversal.

7. **Expecting requested creation mode exactly.**
   The umask removes permission bits.

8. **Using `access()` and then `open()` as a security check.**
   The pathname can change between calls. Attempt the real operation safely.

9. **Assuming extended attributes always work.**
   Filesystem and policy support vary.

10. **Assuming `readdir()` order.**
    Treat a directory as a set of entries unless the application sorts a copy.

11. **Keeping a `readdir()` pointer indefinitely.**
    The storage may be reused by the next call.

12. **Requiring `dirent.d_type`.**
    It can be `DT_UNKNOWN`; call a metadata interface when type is required.

13. **Expecting `readlink()` to append `\0`.**
    It returns a byte count. Add termination only when buffer space permits.

14. **Calling a hard link a copy.**
    Both names reference one inode.

15. **Believing `unlink()` invalidates open descriptors.**
    The open reference keeps the inode alive.

16. **Treating cross-filesystem move as atomic rename.**
    `rename()` returns `EXDEV`; copy-and-delete has a larger failure window.

17. **Ignoring partial writes in a copy loop.**
    Continue until every byte returned by `read()` has been written.

18. **Assuming an inotify watch is recursive.**
    Add and manage watches for subdirectories explicitly.

19. **Advancing through events by only `sizeof(struct inotify_event)`.**
    Include `event->len`.

20. **Ignoring `IN_Q_OVERFLOW` and `IN_IGNORED`.**
    Both change what the consumer can safely believe about its state.

## 23. Guided Exercises

Perform these in order. Keep all new writable paths under
`build/chapters/08-file-and-directory-management/data/`.

### Exercise A - Compare identity before and after rename

Modify a copy of `14_rename_file.c` to print `st_dev`, `st_ino`, and
`st_nlink` before and after the rename.

Question: Which fields must remain equal for the object to be the same inode?

### Exercise B - Open, unlink, and write

Extend `13_unlink_open_file.c` so it writes additional data through the open
descriptor after unlinking, seeks back, and reads the combined content.

Question: Why can the write succeed when `access(path, F_OK)` fails?

### Exercise C - Directory permission experiment

Create a private directory and file, then test controlled combinations of read
and execute permission on the directory. Restore permissions before cleanup.

Question: Which permission is required to open a known child pathname?

### Exercise D - Preserve one more metadata property during copy

Extend `15_copy_file.c` to preserve timestamps with `futimens()`.

Question: Why might changing timestamps also affect ctime?

### Exercise E - Descriptor-relative traversal

Open the lab data directory and use `openat()` to create a child without
changing the current working directory.

Question: What process-global state did you avoid changing?

### Exercise F - Recursive-watch design on paper

Write pseudocode for discovering subdirectories, adding watches, processing
new directory events, handling deletion, and recovering from queue overflow.

Question: Where can a new subdirectory change before its watch is installed?

### Exercise G - Observe deleted-but-open storage

Run a program that opens and unlinks a lab file, pauses briefly, and inspect
`/proc/<pid>/fd` from another terminal.

Question: How does the descriptor link describe the deleted name?

## 24. Review Questions and Answers

### Q1. Where is a filename stored?

In a directory entry. The inode holds object identity and most metadata.

### Q2. Why can two names have the same inode number?

They can be hard links to the same inode.

### Q3. When is an inode number sufficient as a global identity?

Never by itself. Combine it with the containing device ID, and remember that
identifiers can be reused after an inode is reclaimed.

### Q4. What is the difference between `stat()` and `lstat()` on a symlink?

`stat()` follows the final link and reports the target; `lstat()` reports the
link inode.

### Q5. Does `chmod(path, 0777)` guarantee mode 0777?

For an existing object, `chmod()` requests those mode bits subject to
filesystem and privilege rules. The umask applies during creation, not to a
later `chmod()` call.

### Q6. Why is ctime not creation time?

The traditional field records changes to inode status such as mode, owner,
link count, and often content-related metadata changes.

### Q7. What survives when the last pathname is unlinked?

Any active open references. The inode is reclaimed only after its link and
open-reference counts both reach zero.

### Q8. Why can a hard link not cross filesystems?

The directory entry must refer to an inode in its own filesystem.

### Q9. Why can a symbolic link cross filesystems?

It stores a pathname that normal lookup resolves later; it does not directly
hold the target inode number.

### Q10. Is rename the same as copy plus delete?

No. Same-filesystem rename changes namespace entries atomically and preserves
the inode. Copy plus delete creates another inode and has a larger failure
window.

### Q11. Why is `d_type` optional in practice?

Some filesystems do not provide the type in directory entries, so Linux may
return `DT_UNKNOWN`.

### Q12. What does an inotify watch descriptor identify?

A watch within one inotify instance. It is not a process-wide or permanent
filesystem object ID.

### Q13. Why does inotify need an overflow event?

Its queue is finite. If producers outrun the consumer, events are discarded,
and the consumer must learn that its reconstructed state may be incomplete.

## 25. Compiler and Runtime Connections

The chapter reinforces several C and Linux programming rules:

- check every syscall result before using output data;
- preserve `errno` when another call could overwrite it;
- use `ssize_t` for byte counts returned by `read()` and `readlink()`;
- use `off_t` for file sizes and offsets;
- use standard `S_IS*` macros and masks instead of layout assumptions;
- handle partial writes and `EINTR` in transfer loops;
- use bounded pathname construction and check truncation;
- distinguish library-owned and caller-owned buffers;
- close descriptors and directory streams along every completed path;
- avoid printing nondeterministic identifiers as test expectations;
- design tests around stable properties rather than incidental values.

The warning flags used by this repository make suspicious conversions,
shadowed variables, format mismatches, and portability assumptions visible
while the examples are still small.

## 26. Verification Commands

Run the complete chapter checks:

~~~bash
make clean
make chapter CHAPTER=08
make test CHAPTER=08
make check-structure
~~~

Expected final lines:

~~~text
Built Chapter 08 from chapters/08-file-and-directory-management
PASS: all deterministic Chapter 8 checks completed
PASS: chapter-oriented repository structure is valid
~~~

## 27. Mastery Checklist

Before Chapter 9, make sure you can explain without looking up the answer:

- [ ] pathname, directory entry, inode, open file description, and descriptor;
- [ ] why `(st_dev, st_ino)` is stronger than inode number alone;
- [ ] `stat()` versus `fstat()` versus `lstat()`;
- [ ] file-type macros and octal permission bits;
- [ ] regular-file versus directory permission meaning;
- [ ] how umask changes creation mode;
- [ ] real versus effective credentials at a conceptual level;
- [ ] the four extended-attribute operations and support caveat;
- [ ] current-working-directory risks and `fchdir()`;
- [ ] safe `readdir()` assumptions;
- [ ] hard-link versus symbolic-link identity;
- [ ] why an open unlinked file remains usable;
- [ ] same-filesystem rename versus copy-and-delete;
- [ ] why copying requires a metadata policy;
- [ ] how device nodes connect file operations to drivers;
- [ ] why `ioctl()` exists;
- [ ] the inotify instance, watch, event, and cleanup lifecycle;
- [ ] variable event lengths, move cookies, nonrecursive watches, overflow,
  and `IN_IGNORED`.

If you can predict the Chapter 8 programs before running them and explain the
relevant trace afterward, you have the filesystem model this chapter is meant
to build.
