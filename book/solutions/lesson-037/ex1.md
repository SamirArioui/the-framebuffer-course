# Solution: exercise 1 — Read into your own memory

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 037 — whole-file reads](../../lessons/part-1/lesson-037-file-read.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

`ReadFile` hands out bytes the *platform* allocated and the caller must
release — fine for a one-off read, awkward when the engine already knows
where its memory should come from. Part 4's asset loading will read into an
arena; today's version reads into any buffer the caller owns.

`ReadFileInto(path, into, capacity)` is the same contract pointed at
someone else's memory: the file's complete bytes land in `into`, or the
answer is a typed failure — `FILE_UNREADABLE` when the file does not fit.
Nothing is allocated and nothing needs releasing: the bytes are yours
because the memory was always yours. (The `FileData.data` field points at
the caller's buffer — or is 0 on failure, so the same checks work for both
functions.)

Verified both ways in one run:

```
$ DISPLAY=:99 ./build/game README.md /tmp/opencode/xcheck/empty.txt
engine: read /tmp/opencode/xcheck/empty.txt into my own memory: 0 bytes
engine: read README.md: 6113 bytes, 143 lines
```

and the failure path — a file too big for the buffer is a typed answer, not
a truncated read. A 100,000-byte file against the 64 KiB buffer:

```
$ DISPLAY=:99 ./build/game README.md /tmp/opencode/xcheck/big.bin
engine: /tmp/opencode/xcheck/big.bin: does not fit in 64 KiB
engine: read README.md: 6113 bytes, 143 lines
```

The two functions are one design: **the caller decides where bytes live**.
Whether the platform lends you memory for a moment or you hand it your own,
the read is all-of-the-file or it is a failure — and when the caller is an
arena in Part 4, the second form is the one that fits.
