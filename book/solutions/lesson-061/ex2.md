# Solution: exercise 2 — The file that ends too soon

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 061 — the WAV container](../../lessons/part-3/lesson-061-wav.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The experiment first, because the fix is nothing without it. The walk's
refusal — `ok = ok && chunk <= size - body;` — comes out, and the file is
cut to 20000 bytes with its RIFF size following the cut (19992) so the
container's first claim still agrees with the bytes. The `data` chunk's
claim is the lie that remains: it still says 44100. Built with
lesson 013's instrument —

```
CXXFLAGS="-std=c++17 -O0 -g -Wall -Wextra -fsanitize=address" \
LDFLAGS="-lX11 -lasound -fsanitize=address" ./build.sh
```

— the run does not fail typed. It aborts:

```
==377548==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x52a000011020
READ of size 1 at 0x52a000011020 thread T0
    #0 ... in ReadFrame src/audio.cpp:42
    #1 ... in engine::LoadSample(engine::Arena&, char const*) src/audio.cpp:169
    #2 ... in engine::Run() src/main.cpp:133
0x52a000011020 is located 0 bytes after 20000-byte region
allocated by thread T0 here:
    #1 ... in platform::ReadFile(char const*) src/platform_x11.cpp:291
```

Read the two halves of that report together: the copy in `LoadSample` is
reading one byte past the 20000-byte buffer `ReadFile` allocated — the
frames the `data` chunk *claims* run 44100 bytes past their start, and
the copy believes the claim instead of the bytes. AddressSanitizer pads
`malloc`'d regions with poisoned guard bytes and names the exact call
stack, which is why the abort points straight at `ReadFrame`. Without the
sanitizer the same read lands in heap slack and the run "works" — a
sample whose tail is whatever the heap had lying around. A truncated file
producing silent garbage is strictly worse than a crash, and that is the
whole point of the exercise: **a claim is not bytes, and a loader that
follows a claim off the end of a file is not a loader.**

The fix has two halves, and both matter:

- **The walk's refusal comes back.** That is this lesson's own check, and
  it stays where it was: a chunk whose size runs past the file makes the
  whole load `SAMPLE_MALFORMED`, before anything is copied. The patch
  does not show it because the sabotage deleted it and the fix restores
  it to exactly the line the lesson ships.
- **The copy grows its own bound** — the part the patch does show. Right
  before the first frame is taken, `frames_at + frames_bytes > size` is
  refused typed, and the rollback (`ArenaRollback` to the mark taken
  before the allocation) means the refused load leaves no partial frames
  behind. The claim is now checked where it is *made* (the walk) and
  where it is *spent* (the copy): the second check costs one comparison
  per load and buys that no missed check upstream can become an overrun.

The same run, with the fix, under the same sanitizer:

```
engine: assets/tone.wav: could not load (malformed)
```

Exit 1, no AddressSanitizer report, and the typed failure names the file
and the class of failure — the same line the lesson's other corruptions
print. The file's lie costs the run a failure value, never its safety.

One boundary note, since this exercise pushes on it: the probe is the
compiler's, not the engine's — `-fsanitize=address` never appears in
`build.sh`'s defaults and no engine code knows it exists. The lesson's
loader is unchanged in spirit by the fix; it simply stops trusting a claim
for even the length of one copy.
