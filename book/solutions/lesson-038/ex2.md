# Solution: exercise 2 — The device that is always full

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 038 — whole-file writes and round-trips](../../lessons/part-1/lesson-038-file-write.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

Three paths, three predictions to write down first: what does `WriteFile`
answer for `/dev/full`, for a path in a directory that does not exist, and
for a path in a directory the OS will not let you write to?

The instrument narrates the OS's own answer on the way to each typed
failure:

```
$ DISPLAY=:99 ./build/game /dev/full
platform: write failed, errno=28 (No space left on device)
engine: /dev/full: could not write

$ DISPLAY=:99 ./build/game /no-such-dir/out.bin
platform: write open failed, errno=2 (No such file or directory)
engine: /no-such-dir/out.bin: could not write

$ DISPLAY=:99 ./build/game /usr/out.bin
platform: write open failed, errno=13 (Permission denied)
engine: /usr/out.bin: could not write
```

`/dev/full` is the interesting one: the file *opens* fine — it is the
write that fails, with `ENOSPC` on every byte. That is the case the write
loop exists for: `write()` returning an error is not "wrote some of it",
and the contract's answer is a typed failure, not a shorter file.

The other two fail at `open`: `errno=2` because the parent directory does
not exist (the OS cannot create a file in nothing), and `errno=13` because
the directory exists and says no. Same typed answer from the seam
(`could not write`), three different OS reasons underneath.

Notice where the vocabulary boundary sits: `errno=28`, `errno=2`,
`errno=13` are the OS's numbers and they never cross the seam — the engine
prints `could not write` and exits, exactly as it does for every write
failure. If the engine ever needs to distinguish "disk full" from "no
permission", that is a new *typed* failure to design — not an errno to
forward. (And one honest footnote: a failed write may leave a partial file
behind on disk. The engine's answer is correct — it knows the write failed
— but the bytes on disk are the OS's leftovers. Exercise material for a
future lesson: make writes all-or-nothing.)
