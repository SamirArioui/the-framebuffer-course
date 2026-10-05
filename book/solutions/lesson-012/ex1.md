# Solution: exercise 1 — The helper that clashed

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 012 — multi-file builds: translation units and linking](../../lessons/part-0/lesson-012-multi-file.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

Each file compiles clean because each translation unit sees exactly one
`ReportOOM` — the compiler has no idea the other file exists. The link is
where the two definitions meet, and the name is exported by both:

```
/usr/bin/ld: hashtable.o: in function `ReportOOM':
hashtable.c:(.text+0x0): multiple definition of `ReportOOM'; dynarray.o:dynarray.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
```

The chosen fix is the lesson's keyword: `static` on both definitions. Each
translation unit keeps its own private copy — internal linkage, nothing
exported, no name to collide — and `nm` would now show a lowercase `t
ReportOOM` in both objects. The alternative is to want one shared copy:
define `ReportOOM` in a single `.c` file, declare it once in a header both
files include, and the linker sees one definition and one promise per use.

Which fix is right depends on the question the helper answers. `DaPush` and
`HtInit` failing to allocate is each module's private emergency — six lines,
no policy — so two static copies are honest and dependency-free. If the
helper grew policy — logging, cleanup, an error-report format — one shared
translation unit would stop the copies from drifting apart. The rule the
kit follows: `static` by default, external only when a name is genuinely
part of a header's contract.
