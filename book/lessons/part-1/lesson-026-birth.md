# Lesson 026 — the codebase is born

{{#include ../../stability-horizon.md}}

## Prose

Part 0 is over, and its four programs have served their purpose: `wordcount`
taught the pipeline and the memory, `ds-kit` layout and linkage, `paint` bytes
and pixels, `snek` loops and state. None of them is the engine. Today the
engine's codebase is born — from a blank file, in a repository whose `src/`
has been empty since the scaffold — and it grows from here to the end of the
course, one code step per lesson, in a subset of C++ you can read as assembly.

### The birth

Three things happen in one code step, committed together with this prose and
tagged `lesson-026`:

1. **`src/main.cpp` appears.** It contains a blank `main` — the whole program
   is `int main(void) { return 0; }` under a header comment. It compiles, it
   links, it runs, it exits 0 with no output. That is on purpose: every line
   the engine gains from today on shows up in some lesson's diff, starting
   from nothing.
2. **`build.sh` goes live.** The script has been sitting in the repository
   since the scaffold was laid down; today it finally has something to
   compile. It is the one-command build for the rest of the course — Make and
   CMake are never curriculum.
3. **`sandbox/` is deleted, wholesale, in the same step.** This is the
   designed discard, not a loss: the sandbox programs were throwaway by
   contract ("Throwaway Part 0 sandbox"), and from Part 1 on, code lives in
   `src/` and nowhere else. Every Part 0 state stays retrievable — the tags
   keep all of it. If you want the sandbox back at any point:

   ```
   git checkout lesson-025 -- sandbox/
   ```

   The horizon banner above already names the switch: Part 0's states restore
   `sandbox/`, Part 1's engine states restore `src/`.

### The build

`./build.sh` is deliberately small enough to read in one sitting. It finds
every C and C++ source under `src/`, compiles each one to its own object file
under `build/obj/`, and links the result to `build/game`. Nothing about the
build is hidden in a generated makefile; when the codebase grows a file, the
build notices, because it is just `find` over `src/`.

The flags are the ones lesson 025 ended on, and they mean exactly what they
meant there:

```
-std=c++17 -O0 -g -Wall -Wextra
```

`-std=c++17` pins the language version — the compiler agrees with this book
about what the code means (and the language law below is written against that
standard). `-O0` keeps the optimizer out of the way while code is being read;
`-g` keeps debug information for gdb; `-Wall -Wextra` keeps warnings on, and
the build stays at zero warnings from the first line of the engine to the
last.

Today's build and run, in full:

```
$ ./build.sh
build: compiling 1 source(s) from src/
  CC  src/main.cpp
  LD  build/game
build: OK (1 source(s) compiled -> build/game)
$ ./build/game
$ echo $?
0
```

### The language law

The engine is about to carry the rest of the course — on the order of a
hundred lessons of code stacked on top of today's blank file. Its language is
therefore not "C++" in general. It is a **subset**, chosen by the policy
lesson 025 taught with the machine underneath each decision in view:

> **A language feature is admitted only if we can explain what it compiles
> down to.**

This lesson writes that policy down as the engine's language law. Every code
step from here obeys it, and the policy is enforced the way lesson 025
described: **by review, not tooling**. Nothing stops you from breaking the
law in your own copy; a reviewer is what stops it in the course.

**The admitted features.** Exactly the five lesson 025 admitted, each one
already explained by its code generation:

- **references** — a pointer the compiler dereferences for you;
- **overloading** — one name, several functions, kept apart by name mangling;
- **namespaces** — qualified names, zero runtime cost;
- **`constexpr`** — values the compiler folds away before the program exists;
- **classes with vtables** — and the whole class surface lesson 025 used is
  inside this one item: constructors with member-initializer lists,
  `public:`/`private:` access control, single inheritance, virtual and
  pure-virtual functions, and `const` member functions. Each of those
  compiled down to plain struct layout, plain functions, and the tables you
  watched `nm` print. The law admits the surface it can explain, not a
  keyword list.

**Namespaces.** Engine code lives in `namespace engine`. `main` is the single
global function — the C++ runtime looks up `::main` and nothing else — and it
forwards one call into the namespace. Lesson 025 closed on exactly this
pattern with `snek::Run`; the engine keeps it permanently.

**Naming.** Private members carry a trailing underscore: `commands_`,
`grid_`. That convention was adopted in lesson 025 and it holds for the
engine.

**Templates** stay out of the engine — with one precise carve-out: they may
appear in **clearly labeled tooling code** (build and check tools under
`tools/`), never in `src/`. Lesson 025 said templates are "never admitted
into game code"; the law's wording is this one, because a tool is not game
code and a template in a clearly labeled tool is not a loophole in the
engine.

**Stays out until the policy admits it:** exceptions, the STL's containers
and algorithms, `new`/`delete`, and RTTI (`dynamic_cast`, `typeid`). This is
not asceticism — it is the same "explain what it compiles down to" test,
applied to features whose machinery the engine will own instead. The engine's
large buffers will come from OS-level memory reservations later in this part,
and code that owns its memory that way has no business growing
`new`/`delete` churn. (Lesson 025 listed the same exclusions for `snek`;
nothing changed at the boundary.)

**The standard is C++17**, as pinned by the build line since lesson 025.

**Where the law bites.** When a code step would add a construct outside this
list, the lesson admits it first — by explaining what it compiles down to —
or the construct stays out. That is the whole procedure. There is no
`law.h`, no checker, no enforcement in the build; the build is not the place
a language policy lives. The place it lives is here, in the lessons, and in
the review that reads them.

### What the blank buys

A blank file is not an empty lesson. It is the baseline that makes every
later claim checkable: the next lesson's diff is the platform seam and
nothing else; the lesson after that's diff is one window; and by the time the
engine is a hundred files deep, `git diff lesson-026 lesson-027` is still
exactly one step you can read. Continuity is the machinery Part 0 built —
one linear history, one tag per lesson, prose and code co-committed — and it
is now pointed at the engine.

## Code step

One change for this lesson: `src/main.cpp` is born as a blank `main`,
`src/.gitkeep` — the placeholder that held the empty directory — dies with
it, and `sandbox/` is deleted in the same step. Its end state is tagged
`lesson-026`.

```diff
diff --git a/src/.gitkeep b/src/.gitkeep
deleted file mode 100644
index e69de29..0000000
diff --git a/src/main.cpp b/src/main.cpp
new file mode 100644
index 0000000..7ecbde3
--- /dev/null
+++ b/src/main.cpp
@@ -0,0 +1,13 @@
+// main.cpp — the engine, born.
+//
+// Lesson 026: the codebase is born from a blank file. This file obeys the
+// language law documented in the lesson: the admitted C++ subset of
+// lesson 025 — references, overloading, namespaces, constexpr, and classes
+// with vtables — and nothing else. `main` is the one global function the
+// runtime looks up; every engine symbol that follows lives in
+// `namespace engine`.
+
+int main(void)
+{
+    return 0;
+}
```

The deletion of `sandbox/` is the rest of the step — 13 files and 1231 lines
of Part 0 walking out the door together, summarized rather than pasted:

```
 sandbox/README.md             |  29 ---
 sandbox/ds-kit/.gitkeep       |   0
 sandbox/ds-kit/dynarray.c     |  58 -----
 sandbox/ds-kit/dynarray.h     |  24 ---
 sandbox/ds-kit/hashtable.c    |  86 --------
 sandbox/ds-kit/hashtable.h    |  31 ---
 sandbox/ds-kit/main.c         |  81 -------
 sandbox/paint/.gitkeep        |   0
 sandbox/paint/paint.c         | 284 -------------------------
 sandbox/snek/.gitkeep         |   0
 sandbox/snek/snek.cpp         | 482 ------------------------------------------
 sandbox/wordcount/.gitkeep    |   0
 sandbox/wordcount/wordcount.c | 156 --------------
 13 files changed, 1231 deletions(-)
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The engine speaks its name *(extend-the-code)*

Make the born codebase yours: give the engine a voice, in the shape the law
requires. Put the engine's first symbols in `namespace engine` — a `constexpr`
name and a `constexpr` version — have `Run` announce the engine with one line
at startup, and keep `main` as the one global that forwards into the
namespace, exactly the pattern lesson 025 closed on. Build it, run it, and
then check your additions against the law line by line: which admitted
feature does each new line use, and what did it compile down to?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-026/ex1.md)

### Exercise 2 — Two translation units *(extend-the-code)*

The next lessons grow this codebase file by file, so make that shape yours
now: split the engine's code out of `main.cpp` into its own translation unit.
A header declares what `main.cpp` may rely on, a source file defines
`engine::Run`, and `main.cpp` is left with nothing but the global `main`.
Build with `./build.sh` — it must compile two sources and link one program —
and look at what `build/obj/` holds afterwards. Nothing in the build script
changes for any of this; explain why that is.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-026/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 025 — the C++ subset: classes and vtables](../part-0/lesson-025-cpp-subset.md) ·
**Next:** [Lesson 027 — the platform seam and the first X11 window](lesson-027-first-window.md) ·
**Code tag:** [`lesson-026`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-026)
