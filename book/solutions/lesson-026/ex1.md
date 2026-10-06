# Solution: exercise 1 — The engine speaks its name

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 026 — the codebase is born](../../lessons/part-1/lesson-026-birth.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

Three additions, and every one of them is inside the law. `<cstdio>` is the
language's own standard library — C's stdio with its names in `std`, which is
not the STL and not an external library. The two `constexpr` values are the
lesson 025 kind: the compiler folds them away before the program exists, so
the version reaches `printf` as a literal argument rather than a value
fetched from memory. `Run` and the constants live in `namespace engine`, and
`main` stays the one global — it forwards a single call into the namespace,
exactly the shape lesson 025 ended on with `snek::Run`.

The build and run, in full:

```
$ ./build.sh
build: compiling 1 source(s) from src/
  CC  src/main.cpp
  LD  build/game
build: OK (1 source(s) compiled -> build/game)
$ ./build/game
the framebuffer engine v0
```

One line of output is the whole point of the exercise: the codebase now does
something you can see, it does it from inside the namespace the law names,
and adding it cost no new language features beyond the admitted five. When
lesson 027 moves the first real engine code into `namespace engine`, this is
the pattern it lands in.
