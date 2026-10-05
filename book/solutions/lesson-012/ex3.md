# Solution: exercise 3 — The lowercase letter

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 012 — multi-file builds: translation units and linking](../../lessons/part-0/lesson-012-multi-file.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The prediction: `nm` reports `HtHash` with a capital `T` instead of the
lowercase `t`, and nothing else changes — the program builds and runs
exactly as before. Both predictions hold. Before:

```
0000000000000000 t HtHash
```

After deleting the one keyword:

```
0000000000000000 T HtHash
```

The letter is the linker's view of the name. `t` means the symbol is defined
in this object but kept **local** — internal linkage, invisible to the
linker's matching, exactly what `static` promises. `T` means the definition
is exported — external linkage — and the linker will now match other
translation units' `U HtHash` promises against it. The behavior of the
program is unchanged because linkage is about *visibility of names*, not
about what the code does: the machine code of `HtHash` was always there,
always called from this file only.

What did change is the collision surface: any translation unit in the program
could now call `HtHash` — or define its own and die at the link step exactly
as exercise 1's `ReportOOM` did. That is the argument for `static`-by-default
from the other side: every exported name is a promise about your program's
namespace, and the cheapest promise to keep is the one never made.
