# Solution: exercise 2 — What the check cannot see

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 042 — the interface as a contract](../../lessons/part-1/lesson-042-contract.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

Another **deliberate teaching state**: this patch breaches the boundary on
purpose, and gets away with it. `std::system("true")` is the OS — process
execution, shell and all — called from engine code, one call, no OS
header required.

```
$ ./tools/check-boundary.sh
boundary: engine code must not name an OS
boundary: OK — OS headers and OS calls appear only in src/platform_x11.cpp
boundary: the contract a second OS implements is declared in src/platform.h
boundary: 14 single-line declarations there (multi-line ones are in the header)
$ echo $?
0
$ DISPLAY=:99 ./build/game '#leak'
engine: calling the OS behind the seam's back
```

The check passed a tree with a live breach in it. That is not a bug in the
check — it is the check being what a check is: **a list of the mistakes
someone thought of**. Its two lists (OS headers, OS calls) catch the
ordinary leaks — an `XOpenDisplay` here, an `<unistd.h>` there — and they
catch nothing outside the list. `system` is not on it. Neither is
`syscall`, nor inline assembly, nor a libc function that quietly does OS
work, nor a macro that expands to one.

What class of leaks can no grep catch? Everything that reaches the OS
through *indirection*: a function pointer filled in who-knows-where, a
library call whose implementation talks to the kernel (as `printf`
already does), a template in tooling that generates an OS call. The
boundary is a property of *meaning*, and meaning is not syntax.

Which is exactly why the conventions say what they say: the C++ subset is
**enforced by review, not tooling**, and the boundary is enforced the same
way — the check is the first, cheap, always-run filter that catches the
slips; the review is what catches the rest. A tool that passes is not a
proof; it is one less thing for the reviewer to look for.

The write-up to aim for: name your own sneakiest leak (write it, run the
check on it), and then describe what a reviewer must understand to catch
it that the check never can.
