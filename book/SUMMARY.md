# Summary

[Course home](index.md)

<!-- The curriculum parts (Part 0 through Part 5) land here as lessons are
     published, in curriculum order. plan/ holds authoring artifacts and is
     never part of this site: the book builds only from book/. -->

# Part 0 — C foundations

- [Lesson 001 — argv and file input: your first `gcc` command](lessons/part-0/lesson-001-first-program.md)
  - [Solution: ex1 — standard input](solutions/lesson-001/ex1.md)
  - [Solution: ex2 — the silent directory](solutions/lesson-001/ex2.md)
  - [Solution: ex3 — one character at a time](solutions/lesson-001/ex3.md)
  - [Solution: ex4 — bytes, not characters](solutions/lesson-001/ex4.md)

- [Lesson 002 — gdb: breakpoints, stepping, stack frames](lessons/part-0/lesson-002-gdb.md)
  - [Solution: ex1 — the second hit looks the same](solutions/lesson-002/ex1.md)
  - [Solution: ex2 — the file that closes twice](solutions/lesson-002/ex2.md)
  - [Solution: ex3 — conditions see one frame](solutions/lesson-002/ex3.md)
  - [Solution: ex4 — a counter in two scopes](solutions/lesson-002/ex4.md)

- [Lesson 003 — char buffers: strings by hand](lessons/part-0/lesson-003-char-buffers.md)
  - [Solution: ex1 — three hundred characters](solutions/lesson-003/ex1.md)
  - [Solution: ex2 — the quoted line lies](solutions/lesson-003/ex2.md)
  - [Solution: ex3 — the longest word](solutions/lesson-003/ex3.md)
  - [Solution: ex4 — the debugger on your machine](solutions/lesson-003/ex4.md)

- [Lesson 004 — malloc and free: growing buffers on the heap](lessons/part-0/lesson-004-heap-buffers.md)
  - [Solution: ex1 — the doubling sequence](solutions/lesson-004/ex1.md)
  - [Solution: ex2 — one byte at a time](solutions/lesson-004/ex2.md)
  - [Solution: ex3 — who owns the bytes](solutions/lesson-004/ex3.md)
  - [Solution: ex4 — standard input, revisited](solutions/lesson-004/ex4.md)

- [Lesson 005 — leaks made visible with sanitizers](lessons/part-0/lesson-005-leaks.md)
  - [Solution: ex1 — three verdicts](solutions/lesson-005/ex1.md)
  - [Solution: ex2 — what the certainty costs](solutions/lesson-005/ex2.md)
  - [Solution: ex3 — how the sanitizer knows](solutions/lesson-005/ex3.md)

- [Lesson 006 — undefined behavior and buffer overflows](lessons/part-0/lesson-006-undefined-behavior.md)
  - [Solution: ex1 — the value that is not promised](solutions/lesson-006/ex1.md)
  - [Solution: ex2 — the empty line that reads backwards](solutions/lesson-006/ex2.md)
  - [Solution: ex3 — three fates](solutions/lesson-006/ex3.md)
  - [Solution: ex4 — the price of the check](solutions/lesson-006/ex4.md)

- [Lesson 007 — structs: sizeof, alignment, and padding](lessons/part-0/lesson-007-struct-layout.md)
  - [Solution: ex1 — padding in a fresh struct](solutions/lesson-007/ex1.md)
  - [Solution: ex2 — the padding is real memory](solutions/lesson-007/ex2.md)
  - [Solution: ex3 — why the machine insists](solutions/lesson-007/ex3.md)
  - [Solution: ex4 — the other machine's layout](solutions/lesson-007/ex4.md)

- [Lesson 008 — dynarray growth: realloc and capacity](lessons/part-0/lesson-008-dynarray.md)
  - [Solution: ex1 — the growth schedule](solutions/lesson-008/ex1.md)
  - [Solution: ex2 — the half-freed array](solutions/lesson-008/ex2.md)
  - [Solution: ex3 — doubling versus one-at-a-time](solutions/lesson-008/ex3.md)
  - [Solution: ex4 — what `realloc` really promises](solutions/lesson-008/ex4.md)

- [Lesson 009 — function pointers: comparators and hooks](lessons/part-0/lesson-009-function-pointers.md)
  - [Solution: ex1 — the reversed order](solutions/lesson-009/ex1.md)
  - [Solution: ex2 — searching by predicate](solutions/lesson-009/ex2.md)
  - [Solution: ex3 — why the bridge exists](solutions/lesson-009/ex3.md)

- [Lesson 010 — void*: genericity and its pain](lessons/part-0/lesson-010-void-pointer.md)
  - [Solution: ex1 — the same bytes, differently](solutions/lesson-010/ex1.md)
  - [Solution: ex2 — the wrong size](solutions/lesson-010/ex2.md)
  - [Solution: ex3 — removing in the middle](solutions/lesson-010/ex3.md)
  - [Solution: ex4 — the contract, written down](solutions/lesson-010/ex4.md)

- [Lesson 011 — the hashtable: hashing, buckets, lookup](lessons/part-0/lesson-011-hashtable.md)
  - [Solution: ex1 — count on it](solutions/lesson-011/ex1.md)
  - [Solution: ex2 — the leaderboard](solutions/lesson-011/ex2.md)
  - [Solution: ex3 — the walk's length](solutions/lesson-011/ex3.md)
  - [Solution: ex4 — one bucket](solutions/lesson-011/ex4.md)

- [Lesson 012 — multi-file builds: translation units and linking](lessons/part-0/lesson-012-multi-file.md)
  - [Solution: ex1 — the helper that clashed](solutions/lesson-012/ex1.md)
  - [Solution: ex2 — table statistics](solutions/lesson-012/ex2.md)
  - [Solution: ex3 — the lowercase letter](solutions/lesson-012/ex3.md)

- [Lesson 013 — raw bytes and pixel formats](lessons/part-0/lesson-013-raw-bytes.md)
  - [Solution: ex1 — three pixels, by hand](solutions/lesson-013/ex1.md)
  - [Solution: ex2 — the same pixel in packed 32-bit](solutions/lesson-013/ex2.md)
  - [Solution: ex3 — offsets, by hand](solutions/lesson-013/ex3.md)
  - [Solution: ex4 — the pixel that is not there](solutions/lesson-013/ex4.md)

- [Lesson 014 — endianness and image-header layout](lessons/part-0/lesson-014-image-headers.md)
  - [Solution: ex1 — width 258](solutions/lesson-014/ex1.md)
  - [Solution: ex2 — the wrong way around](solutions/lesson-014/ex2.md)
  - [Solution: ex3 — structs do not make files](solutions/lesson-014/ex3.md)
  - [Solution: ex4 — ask your own machine](solutions/lesson-014/ex4.md)

- [Lesson 015 — fill-rect onto a memory buffer](lessons/part-0/lesson-015-fill-rect.md)
  - [Solution: ex1 — the rectangle off the left](solutions/lesson-015/ex1.md)
  - [Solution: ex2 — counting what survived](solutions/lesson-015/ex2.md)
  - [Solution: ex3 — the addition that ate the clip](solutions/lesson-015/ex3.md)
  - [Solution: ex4 — why fold first](solutions/lesson-015/ex4.md)

- [Lesson 016 — drawing lines onto the buffer](lessons/part-0/lesson-016-lines.md)
  - [Solution: ex1 — the vertical line through everything](solutions/lesson-016/ex1.md)
  - [Solution: ex2 — rectangle outlines](solutions/lesson-016/ex2.md)
  - [Solution: ex3 — float versus integer](solutions/lesson-016/ex3.md)
  - [Solution: ex4 — the error term, watched](solutions/lesson-016/ex4.md)

- [Lesson 017 — writing a real image file by hand](lessons/part-0/lesson-017-image-file.md)
  - [Solution: ex1 — rows on disk](solutions/lesson-017/ex1.md)
  - [Solution: ex2 — round trip](solutions/lesson-017/ex2.md)
  - [Solution: ex3 — the flipped image](solutions/lesson-017/ex3.md)
  - [Solution: ex4 — per byte versus per row](solutions/lesson-017/ex4.md)

- [Lesson 018 — the optimizer and undefined behavior](lessons/part-0/lesson-018-optimizer-ub.md)
  - [Solution: ex1 — predict the wreckage](solutions/lesson-018/ex1.md)
  - [Solution: ex2 — the guard that gets deleted](solutions/lesson-018/ex2.md)
  - [Solution: ex3 — review a colleague's fill](solutions/lesson-018/ex3.md)
  - [Solution: ex4 — prove the fix](solutions/lesson-018/ex4.md)

- [Lesson 019 — the game loop](lessons/part-0/lesson-019-game-loop.md)
  - [Solution: ex1 — the life of `strtoul`](solutions/lesson-019/ex1.md)
  - [Solution: ex2 — the string that never ends](solutions/lesson-019/ex2.md)
  - [Solution: ex3 — pack the state](solutions/lesson-019/ex3.md)
  - [Solution: ex4 — why three phases](solutions/lesson-019/ex4.md)

- [Lesson 020 — timing with `clock_gettime`](lessons/part-0/lesson-020-timing.md)
  - [Solution: ex1 — predicting the ticks](solutions/lesson-020/ex1.md)
  - [Solution: ex2 — what the frame cap costs](solutions/lesson-020/ex2.md)
  - [Solution: ex3 — two clocks](solutions/lesson-020/ex3.md)
  - [Solution: ex4 — a thing that moves](solutions/lesson-020/ex4.md)

- [Lesson 021 — raw terminal input with escape codes](lessons/part-0/lesson-021-terminal-input.md)
  - [Solution: ex1 — bytes all the way down](solutions/lesson-021/ex1.md)
  - [Solution: ex2 — the key that vanished](solutions/lesson-021/ex2.md)
  - [Solution: ex3 — cSI, SS3, and your terminal](solutions/lesson-021/ex3.md)
  - [Solution: ex4 — wASD](solutions/lesson-021/ex4.md)

# Sample

- [Sample lesson — filling a rectangle](lessons/sample-lesson.md)
  - [Solution: ex1 — draw an outline](solutions/lesson-000/ex1.md)
  - [Solution: ex2 — the crashing rectangle](solutions/lesson-000/ex2.md)
