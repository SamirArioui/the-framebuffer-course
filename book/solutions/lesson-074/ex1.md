# Solution: exercise 1 — The refusal that gets ignored

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 074 — one fixed store](../../lessons/part-4/lesson-074-store.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is the request *past* the full one, handled the way the contract
says: the error read first, the refusal acted on, and the run carrying on
with the 64 entities it has. The `else` branch is there on purpose — it
is the branch a correct run never takes and a broken one would, and
having it makes the check visible in the code.

Now the crash the exercise asks you to produce first. Hoisting the
script's `EntityResult` out of the loop and using the last request's
entity after it —

```cpp
    made.entity->x += 1; /* the run moves the entity it just made */
```

— is a write through a pointer the contract says is 0. Rebuilt with
lesson 013's instrument (`CXXFLAGS="-std=c++17 -O0 -g -Wall -Wextra
-fsanitize=address" LDFLAGS="-fsanitize=address -lX11 -lasound"`), the
run dies before the first frame:

```
AddressSanitizer:DEADLYSIGNAL
=================================================================
==479187==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000010 (pc ... T0)
==479187==The signal is caused by a READ memory access.
==479187==Hint: address points to the zero page.
    #0 ... in engine::Run() src/main.cpp:268
```

Address `0x10` is not a mystery: it is the zero page plus the offset of
`x` in `Entity` — the null pointer's `x` field, sixteen bytes in. The
sanitizer names the line; the address names what the code believed it
had.

One wrinkle worth knowing, because it is how this bug hides. The *first*
version of the broken code did not crash at all: it printed
`made.entity->name` through the same null pointer and glibc's `printf`
answered `(null)` — a `%s` that arrives as a null pointer is printed, not
dereferenced. The run carried on as if nothing were wrong, and the bug
waited for a line that actually touches the entity's fields. A contract
is checked by reading the error, never probed by whichever access happens
to survive.

The fix is the check the contract asks for, in the order it asks for it:
`if (past.error != ENTITY_OK)` before anything reads `past.entity`. From
a real run of this lesson's end state plus the patch:

```
engine: store: live 64 of 64 — the hero and 63 from the script
engine: store: creation refused (full), arena 1253648 -> 1253648 — creation allocates nothing
engine: store: the request past the full one refused (full) — the run carries on with 64 live
engine: closed
```

The run survives the full store, names the refusal, and ends cleanly —
`engine: closed`, the same ending every other path gives. That is what
"the game decides what a refusal means" means in practice: the store's
answer is a value, and the game's handling of it is ordinary code that
can be wrong in ordinary ways — which is exactly why the answer is typed
in the first place. An error that arrives as a crash is an error nobody
can handle.

Nothing here touches the store, the creation rule, or the game's loop:
the patch is one request and its handling, beside the lesson's own.
