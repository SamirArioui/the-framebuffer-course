# Lesson 042 — the interface as a contract

{{#include ../../stability-horizon.md}}

## Prose

Sixteen lessons ago the seam was two functions and a promise. Today it is
audited: **what a second OS must implement**, checked against the
platform spec's single-boundary scenarios and against the code as it
stands. This is the lesson that turns "we wrote a platform layer" into
"the platform layer is a contract" — and the difference is that a contract
has claims you can *check*.

### What a second OS must implement

Everything the seam declares — nothing more, nothing less. From
`platform.h`, the complete list:

| The contract | What it means to the engine |
| ------------ | --------------------------- |
| `OpenWindow`, `CloseWindow` | a window of the requested size, resources released |
| `PumpEvents`, `CloseRequested` | news folded into state; every ending reports one way |
| `KeyDown`, `KeyPressed`, `HasFocus` | polled input: down now, went down, focus |
| `Present` | our bytes in the window, done when it returns |
| `Now`, `PageSize` | monotonic time; the page unit of memory |
| `ReserveMemory`, `ReleaseMemory` | whole pages, zeroed, owned until released |
| `ReadFile`, `ReleaseFile`, `WriteFile` | whole files or typed failures, both directions |

Fifteen functions, five plain structs and enums (`Window`, `Key`,
`OpenError`/`WindowResult`, `MemoryError`/`Reservation`,
`FileError`/`FileData`), and one rule that governs all of them: **no OS
type appears in the interface**. `Window` is declared and never defined in
the header; the keys are the seam's; the errors are the seam's. A second
OS implements the same functions and defines `struct Window` as whatever
its own window is made of. Exercise 1 does exactly that with a stub, and
the engine links against it without one changed line.

### Scenario one: engine code is OS-free

> **WHEN** a source file outside the platform layer is inspected,
> **THEN** it contains no OS-specific calls or headers.

Checked mechanically, and now *kept* checked — `tools/check-boundary.sh`
walks every engine file and fails on OS headers or OS calls:

```
$ ./tools/check-boundary.sh
boundary: engine code must not name an OS
boundary: OK — OS headers and OS calls appear only in src/platform_x11.cpp
boundary: the contract a second OS implements is declared in src/platform.h
boundary: 14 single-line declarations there (multi-line ones are in the header)
```

The objects agree at the link level. `main.o`'s undefined symbols are
engine functions, the language's own, and `platform::` calls — and
`platform_x11.o` defines exactly the contract's fifteen:

```
$ nm build/obj/platform_x11.o | grep " T " | c++filt | sed 's/^[0-9a-f]* T //' | sort
platform::CloseRequested(platform::Window const*)
platform::CloseWindow(platform::Window*)
platform::HasFocus(platform::Window const*)
platform::KeyDown(platform::Window const*, platform::Key)
platform::KeyPressed(platform::Window*, platform::Key)
platform::Now()
platform::OpenWindow(int, int)
platform::PageSize()
platform::Present(platform::Window*, unsigned char const*, int, int)
platform::PumpEvents(platform::Window*)
platform::ReadFile(char const*)
platform::ReleaseFile(platform::FileData&)
platform::ReleaseMemory(platform::Reservation&)
platform::ReserveMemory(unsigned long)
platform::WriteFile(char const*, unsigned char const*, unsigned long)
```

The boundary is not a diagram; it is the link step.

### Scenario two: a second OS slots in

> **WHEN** an implementation for a different OS is added behind the
> interface, **THEN** only platform-layer files change.

The changed-file list is exactly one: the implementation file. Exercise 1
proves it — a stub OS in its own file, the engine compiled and linked
against it unchanged, no X11 anywhere on the line:

```
$ ./build/game-stub
engine: no display to open a window on
```

The audit found **one wrinkle**, and honesty requires writing it down: the
build compiles every source under `src/`, so two implementations cannot
coexist there — each defines the same fifteen contract functions and the
linker would refuse. The contract therefore says **one implementation per
build**: the port's file replaces the first OS's file (or, if both must
live in the tree, the build's file selection is the one line that knows —
and it is not engine code). The scenario holds; the phrasing is "only
platform-layer files change", not "only new files appear".

### The audit's limits

The check passes today's tree — and a check is a *list of mistakes someone
thought of*. Exercise 2 opens that door deliberately: a boundary leak the
check never hears about, and the question of what review must do that
tooling cannot. The conventions already gave the answer for the language
law — *enforced by review, not tooling* — and the boundary is enforced the
same way. The check is the first, cheap filter. The review is the rest.

### Where this leaves the seam

The interface is **fixed** — the API was fixed before its first
implementation (design D1) and it has grown one contract at a time, each
lesson adding exactly one promise. Part 2 presents pixels through it, Part
3 mixes sound beside it, Part 4 loads assets across it, Part 5 measures
frames on it. None of those touch engine code's relationship to the OS:
engine code asks the seam; the seam answers for the machine.

The last thing missing is the thing all of this was for: one complete,
measured run that uses every part of the contract at once. That is
lesson 043.

## Code step

One change for this lesson: `tools/check-boundary.sh` — the single-boundary
scenario as a repeatable check that walks the engine's source files and
fails on OS headers or OS calls outside the implementation. Its end state
is tagged `lesson-042`.

```diff
diff --git a/tools/check-boundary.sh b/tools/check-boundary.sh
new file mode 100755
index 0000000..39582f3
--- /dev/null
+++ b/tools/check-boundary.sh
@@ -0,0 +1,54 @@
+#!/usr/bin/env bash
+#
+# check-boundary.sh — the single-OS-boundary check.
+#
+# Lesson 042: the seam's contract, audited mechanically. Engine code may
+# include the platform interface and the language's own headers — nothing
+# that knows which OS it is on. The platform implementation files are the
+# only place OS headers and OS calls may appear; a second OS replaces those
+# files and nothing else.
+#
+# Usage:
+#   ./tools/check-boundary.sh
+#
+# Exits 0 when the boundary holds, 1 when it is breached.
+
+set -euo pipefail
+
+cd "$(dirname "$0")/.."
+
+# The files a second OS replaces. Everything else is engine code.
+IMPL="src/platform_x11.cpp"
+
+# Headers that only an OS has. The language's own headers (<cstdio>,
+# <cstring>, <stddef.h>, ...) are fine anywhere — they are not an OS.
+OS_HEADERS='<X11/|<sys/|<unistd\.h>|<fcntl\.h>|<poll\.h>|<signal\.h>|<errno\.h>|<time\.h>'
+
+# Calls only an OS answers. The list grows with the seam.
+OS_CALLS='(^|[^A-Za-z0-9_:])(X[A-Z][A-Za-z]+|mmap|munmap|mprotect|clock_gettime|nanosleep|sysconf|open|close|read|write|fstat|poll|signal)\s*\('
+
+status=0
+
+echo "boundary: engine code must not name an OS"
+for f in src/*.h src/*.cpp; do
+    [ "$f" = "$IMPL" ] && continue
+    if hits=$(grep -nE "$OS_HEADERS" "$f"); then
+        echo "BREACH: $f includes an OS header:"
+        echo "$hits"
+        status=1
+    fi
+    if hits=$(grep -nE "$OS_CALLS" "$f"); then
+        echo "BREACH: $f calls an OS function:"
+        echo "$hits"
+        status=1
+    fi
+done
+
+if [ "$status" -eq 0 ]; then
+    echo "boundary: OK — OS headers and OS calls appear only in $IMPL"
+fi
+
+echo "boundary: the contract a second OS implements is declared in src/platform.h"
+echo "boundary: $(grep -cE '^[A-Za-z].*\(.*\);' src/platform.h) single-line declarations there (multi-line ones are in the header)"
+
+exit "$status"
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The second OS *(extend-the-code)*

The proof of a contract is a second implementation of it. Write
`tools/platform_stub.cpp`: every function the seam declares, implemented
by typed refusal (`OPEN_NO_DISPLAY`, `FILE_NOT_FOUND`,
`MEMORY_NO_MEMORY`) — a complete OS that does nothing, honestly. Then link
the engine against it — every engine file unchanged — and run it. What
does the link line prove that the header only promises? And where did your
stub have to define `struct Window`, and why there?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-042/ex1.md)

### Exercise 2 — What the check cannot see *(explain-in-prose)*

**This exercise breaches the boundary on purpose** — a deliberate teaching
state. Find a way to reach the OS from `main.cpp` that
`check-boundary.sh` does not catch (`std::system` is one; find your own
too), show the check passing the broken tree, and then write down the
*class* of leaks no grep can catch — calls through indirection, libc
functions that quietly talk to the kernel, code a macro expands to. What
must a reviewer understand to catch them, and why does the authoring
contract say "enforced by review, not tooling"?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-042/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 041 — arenas](lesson-041-arenas.md) ·
**Next:** [Lesson 043 — the closing demo](lesson-043-demo.md) ·
**Code tag:** [`lesson-042`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-042)
