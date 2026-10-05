# Solution: exercise 2 — The quoted line lies

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 003 — char buffers: strings by hand](../../lessons/part-0/lesson-003-char-buffers.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The attempt goes wrong four ways, and the symptom of the first two is the
`"hi thererld"` you saw: the copy runs for *every* line instead of only when
a new longest is found, and it writes `len` characters but never the NUL.
After `story.txt`'s second line the buffer reads `hi there` from the new
copy followed by `rld` — leftovers of the earlier, longer `hello world` —
and `%s` walks straight through them. Third, nothing records the final line
when the file lacks a trailing newline, so `partial.txt` quotes whatever
garbage the stack holds (on a quiet build, often nothing at all). Fourth,
`longest_text[128]` cannot hold a line the `line` buffer accepted — the
copy writes up to 256 bytes into 128, a stack overflow that happens to be
harmless on this build; lessons 005 and 006 teach the tools that make such
things scream.

The fix does the obvious right things: a buffer the size of `line`,
`longest_text[0] = '\0'` up front so an empty file quotes empty, the copy
under the new-longest guard and copying `i <= len` so the terminator comes
along, and the same record in the trailing-line branch. Verified on all the
tricky inputs at once:

```
$ ./wordcount story.txt
longest line: "hello world"
2 4 21 11 story.txt
$ ./wordcount partial.txt empty.txt
longest line: "no newline"
0 2 10 10 partial.txt
longest line: ""
0 0 0 0 empty.txt
```
