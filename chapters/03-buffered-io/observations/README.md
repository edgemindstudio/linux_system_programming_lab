# Chapter 3 Observations

Store deliberately retained evidence from Chapter 3 here.

The tracing helper writes system-call traces to `observations/strace/`:

~~~bash
chapters/03-buffered-io/scripts/trace.sh 09_buffered_copy \
    /etc/hosts build/chapters/03-buffered-io/data/hosts.copy
~~~

Particularly useful evidence includes:

- `strace -c` comparisons of buffered and unbuffered operation counts;
- traces showing when `fflush()` or `fclose()` causes `write()`;
- predictions and results for full, line, and unbuffered modes;
- observations about stream position when standard I/O and descriptor I/O mix;
- thread-output records before and after manual stream locking.

Generated traces are ignored by default. Commit only evidence that supports a
written prediction, corrected assumption, or chapter conclusion.
