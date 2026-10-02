# Mental Model: File and Directory Management

## The central chain

~~~text
pathname
   |
   v
directory entry (name -> inode reference)
   |
   v
inode identity and metadata
   |
   +--> type and permissions
   +--> owner and group
   +--> timestamps
   +--> hard-link count
   +--> extended attributes
   +--> data or device identity
~~~

The pathname is not stored in the inode. A directory gives a name to an inode.
That one distinction explains hard links, rename behavior, unlink lifetime,
and why an open descriptor survives a pathname change.

## Three ways to reach an object

~~~text
path lookup                 open descriptor              directory stream
stat(path)                  fstat(fd)                    readdir(DIR *)
chmod(path)                 fchmod(fd)                   name enumeration
chown(path)                 fchown(fd)
~~~

A path-based operation performs name resolution at the time of the call. A
descriptor-based operation acts on the already opened object. Descriptor-based
operations avoid repeating pathname resolution and can reduce races.

## stat(), fstat(), and lstat()

| Interface | Starting point | Final symbolic link |
|---|---|---|
| `stat(path, &st)` | pathname | followed |
| `fstat(fd, &st)` | open descriptor | already resolved |
| `lstat(path, &st)` | pathname | reported as the link itself |

`struct stat` is a snapshot. The filesystem may change immediately after the
call returns. Do not treat a successful `stat()` followed by a separate `open()`
as one atomic decision.

## st_mode contains two categories

~~~text
st_mode
   +--> object type: regular, directory, link, FIFO, socket, block, character
   +--> permission and special bits: rwx, set-user-ID, set-group-ID, sticky
~~~

Use `S_ISREG`, `S_ISDIR`, `S_ISLNK`, and related macros for type tests. Mask
permission bits deliberately rather than printing all implementation bits.

## Directory permissions are operations

For regular files:

- read means reading contents;
- write means changing contents;
- execute means attempting execution.

For directories:

- read means listing names;
- write means creating or removing entries;
- execute means searching/traversing the directory.

Directory write permission does not by itself grant access to file contents.

## Creation mode and umask

~~~text
requested mode: 0777
umask:          0027
effective:      0750
~~~

The kernel clears masked permission bits during creation. Later `chmod()` or
`fchmod()` changes the inode mode directly and is not another creation event.

## Ownership and credentials

~~~text
inode                 process
st_uid <-----------> real UID / effective UID
st_gid <-----------> real GID / effective GID
~~~

Most access attempts use effective credentials. `access()` answers for real
credentials and is not a substitute for attempting the actual operation when
making a security decision.

## Extended attributes

~~~text
inode
  user.chapter08 = "metadata-lab"
  security.*     = security framework data
  system.*       = kernel/filesystem-managed data
  trusted.*      = privileged metadata
~~~

An xattr key and value are metadata outside the primary byte stream. Support
depends on filesystem and mount configuration.

## Directory state

~~~text
process filesystem context
   +--> current working directory
   +--> root directory
   +--> umask
~~~

`chdir()` changes the pathname base used by relative lookups. `fchdir()` uses
an open directory descriptor. `opendir()` creates a library-level directory
stream; `readdir()` returns entries in unspecified order.

## Hard links

~~~text
name A ----+
           +----> inode (st_nlink = 2)
name B ----+
~~~

Both names are peers. Removing one name decrements the link count. The inode
remains reachable through the other name.

## Symbolic links

~~~text
link inode --stores pathname text--> another lookup --> target inode
~~~

A symbolic link is a distinct inode. It may cross filesystems and may dangle.
`readlink()` reads the stored pathname and does not append a null terminator.

## Unlink lifetime

~~~text
directory entry removed
          |
          v
st_nlink becomes zero
          |
          +--> open descriptor still references inode
                     |
                     v
               final close releases last reference
~~~

Removing a name and destroying file data are not necessarily the same instant.

## Copy versus rename

| Operation | Namespace | Data | Inode |
|---|---|---|---|
| Hard link | adds a name | shared | same |
| Rename | changes a name | not copied | same within filesystem |
| Copy | adds destination name | transferred | distinct |

`rename()` is atomic within one filesystem but fails with `EXDEV` across
filesystems. A portable move utility then performs copy plus removal.

## Device nodes and ioctl

~~~text
open/read/write/ioctl
         |
         v
device node (major, minor)
         |
         v
kernel device driver
~~~

The familiar file API becomes a dispatch interface to a driver. `ioctl()`
handles controls that do not naturally fit the main byte stream.

## inotify lifecycle

~~~text
inotify_init1() -> instance fd
        |
inotify_add_watch() -> watch descriptor
        |
filesystem operation -> queued event records
        |
read(instance fd) -> parse struct inotify_event + name bytes
        |
inotify_rm_watch() -> IN_IGNORED
        |
close(instance fd)
~~~

The instance descriptor belongs to the process. A watch descriptor is only
meaningful inside that instance. Move cookies correlate paired events.
Directory watches are not recursive, and queue overflow must be treated as a
loss of information requiring reconciliation.

## Compiler and runtime connection

Compilers, linkers, package managers, language servers, and build systems all
depend on these mechanisms:

- source discovery uses directory traversal;
- incremental builds compare metadata but must respect timestamp limits;
- atomic output replacement commonly uses temporary file plus `rename()`;
- caches rely on inode identity, permissions, and ownership;
- file watchers drive rebuilds and editor indexes;
- secure runtimes avoid unsafe symbolic-link traversal;
- diagnostic tools inspect `/proc`, which presents kernel state through a
  filesystem interface.
