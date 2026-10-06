# Solution: exercise 2 — Two translation units

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 026 — the codebase is born](../../lessons/part-1/lesson-026-birth.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The split is one header, one implementation, and a `main.cpp` reduced to its
one job. `engine.h` is a contract between translation units in the sense
lesson 012 taught: it declares `engine::Run` and nothing else, wrapped in the
`ENGINE_H` include guard so that a second include changes nothing. The
`#include "engine.h"` in both source files finds it because the quoted form
searches the including file's directory first — no `-I` flags, no paths.

The engine's code now compiles on its own, and `Run`'s definition is a plain
function in `namespace engine`: the qualified name the linker sees is the
same kind of length-prefixed mangled name lesson 025 read off `nm`.

The build proves the shape:

```
$ ./build.sh
build: compiling 2 source(s) from src/
  CC  src/engine.cpp
  CC  src/main.cpp
  LD  build/game
build: OK (2 source(s) compiled -> build/game)
$ ls build/obj
engine.o
main.o
$ ./build/game
$ echo $?
0
```

Why the build script never changed: `build.sh` never names a source file. It
*finds* every C and C++ file under `src/`, compiles each to its own object
under `build/obj/`, and links whatever it compiled. Adding `engine.cpp` and
`engine.h` to the codebase is all it took — the one-command build is doing
exactly what it was written to do, and the codebase can now grow a file per
lesson without anyone touching the build. (The `.h` is never compiled
directly; it is text included into the two translation units that need it.)
