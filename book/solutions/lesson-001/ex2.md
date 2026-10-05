# Solution: exercise 2 — The silent directory

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 001 — argv and file input: your first `gcc` command](../../lessons/part-0/lesson-001-first-program.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

Opening a directory with `fopen` succeeds on this system — the failure
arrives later, at the first read. The read loop cannot see it: `fgetc`
returns `EOF` both for "the file ended" and for "the read failed", and those
two endings look identical in the loop condition. The stream knows the
difference, and `ferror(f)` is how you ask: it reports whether a read or
write error has occurred on the stream.

The fix tests `ferror` after the loop, before anything is printed. A file
that ended normally gets its count line as before; a file that failed to
read gets a `cannot read` message on `stderr` instead, and the loop continues
with the next file. The `fclose(f)` before `continue` is the same resource
discipline as everywhere else: a file that failed to read is still an open
file.

Run `./wordcount .` with the fix and the directory complaint appears; run
`./wordcount wordcount.c .` and one line of count and one line of complaint
come out together. The companion function `feof(f)` answers the other
question — "did the loop stop because the file ended?" — which is the one
you want when a truncated read would be a bug.
