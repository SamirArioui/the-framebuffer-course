# Solution: exercise 1 — The second OS

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 042 — the interface as a contract](../../lessons/part-1/lesson-042-contract.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The proof that the seam is a contract is a second implementation of it —
and this one is the smallest that can exist: every function the header
declares, defined by refusal. No window can open (`OPEN_NO_DISPLAY`), no
file can be read (`FILE_NOT_FOUND`), no memory can be reserved
(`MEMORY_NO_MEMORY`). A stub OS that does nothing, honestly.

The interesting part is not the stub. It is the **link line**:

```
$ g++ -std=c++17 -O0 -g -Wall -Wextra -Isrc \
      src/main.cpp src/framebuffer.cpp src/frame.cpp src/arena.cpp \
      tools/platform_stub.cpp -o build/game-stub
$ ./build/game-stub
engine: no display to open a window on
$ echo $?
1
```

The engine — every engine file, unchanged since the lesson's end state —
compiled and linked against the second implementation. No X11 library on
the line at all: `tools/platform_stub.cpp` is the whole OS, and the engine
does not notice the difference. It starts, asks for its window, receives
the typed failure the stub reports, and exits through the same error path
it would use for any display-less machine.

That is the spec's scenario rendered executable: *when an implementation
for a different OS is added behind the interface, only platform-layer
files change*. The changed file here is the platform implementation (its
own file, not even beside the engine's); `main.cpp`, `framebuffer.*`,
`frame.*`, and `arena.*` are byte-for-byte the lesson's end state.

One honest note, the same one the audit recorded: the stub lives in
`tools/` for this exercise because `build.sh` compiles *every* source in
`src/` — two implementations in there would collide at link, each
defining the same contract functions. The rule the audit writes down: **one
implementation per build**. If a port wants both files in the tree at once,
the build's file selection is the one line that knows — and it is not
engine code.

(The stub also happens to prove a smaller thing: the seam pulls in no OS
types. `struct Window` in the stub is a different type with a different
definition than X11's, and nothing outside cared.)
