# Solution: exercise 2 — The file that never ends

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 037 — whole-file reads](../../lessons/part-1/lesson-037-file-read.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

Three predictions to make before running anything — what does `ReadFile`
answer for a directory, for `/dev/zero`, and for a file that does not
exist?

With the instrument applied, the implementation narrates the OS's own
answer on the way to the typed failure:

```
$ DISPLAY=:99 ./build/game src
platform: not a regular file (directory)
engine: src: unreadable

$ DISPLAY=:99 ./build/game /dev/zero
platform: not a regular file (special)
engine: /dev/zero: unreadable

$ DISPLAY=:99 ./build/game no-such-file.txt
platform: open failed, errno=2 (No such file or directory)
engine: no-such-file.txt: file not found
```

The directory and the missing file are the ordinary cases: `open` fails
with `errno=2` (`ENOENT`) for the missing path — the one OS error that maps
to `FILE_NOT_FOUND` — and a directory *opens* fine but fails the
`S_ISREG` check, so it becomes `FILE_UNREADABLE`.

`/dev/zero` is the interesting prediction. It opens, it reads — forever. A
naive whole-file loop (`while (read(...) > 0) keep going`) never returns
from it; "all the bytes" is an infinite answer. The implementation never
asks the question that way: it takes the file's *size* first
(`fstat`), allocates exactly that, reads exactly that, and then requires
the next byte to not exist. `/dev/zero` is a character device with size 0 —
it fails the regular-file check before a single byte is read. The whole-file
contract is not "read until the file stops"; it is "the file has a size,
and you get exactly it, or you get a failure".

The instrument's lines are the OS's vocabulary (`errno`, file kinds); the
engine's lines are the seam's (`file not found`, `unreadable`). The seam
does not leak the first into the second — the whole point of the typed
failure is that the engine never needs to know what `errno=2` means.
