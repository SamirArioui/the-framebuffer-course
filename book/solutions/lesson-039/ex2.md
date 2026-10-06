# Solution: exercise 2 — The file that lies about its size

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 039 — the virtual-memory deep dive](../../lessons/part-1/lesson-039-virtual-memory.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The bug first. `ReadFile` trusts `fstat`: it takes the reported size,
allocates exactly that, and reads exactly that. For an ordinary file the
size is the truth. For a **virtual file** — `/proc/self/maps` is one, and
it is the same file the deep dive reads — the size is a *lie*: the kernel
reports 0 bytes and then produces kilobytes when you read it. Our
whole-file reader therefore returns **0 bytes, presented as success** —
exactly the outcome lesson 037's contract forbids: partial data dressed as
success.

The fix is to stop treating the size as the destination and start treating
it as a hint. Read until the file says it is done — `read()` returning 0 —
and grow the buffer whenever it fills (lesson 008's `realloc`, doing the
job it was taught to do). The old "one byte past the end" check goes away:
end-of-file *is* the end of the file, whatever the size claimed.

With the fix and a probe reading the second command-line argument:

```
$ DISPLAY=:99 ./build/game rt.bin /proc/self/maps
engine: page size 4096 bytes
engine: /proc/self/maps: 5766 bytes read
```

5,766 bytes — the map file's real content (its size varies per run: the
process's mappings decide). Before the fix the same call reported 0.

What does the contract mean now? "Whole file" was never "the number in the
stat" — it is *everything the file gives*, and a file that gives more than
its size claimed gets read completely. Regular files still land in one
allocation of exactly the right size; virtual files grow from a first
chunk. The reader is honest in both cases, and "never partial data
presented as success" now holds for the files that lie.

(The same lesson applies to `/proc` generally, sysfs, and anything else
the kernel synthesizes on read — which is worth remembering long before
Part 4 starts reading real assets: know which kind of file you are
holding.)
