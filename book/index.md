# Welcome

{{#include stability-horizon.md}}

This is the course site for **The Framebuffer Course** — *from C foundations to
a finished 2D arcade game on an engine you wrote yourself*: a free, open,
English-language written course of ~130 medium lessons that takes a
Python or Ruby developer with no C/C++ experience from C foundations to a
finished 2D top-down arcade game running on a fully hand-written engine — no
external libraries in any student-visible C/C++ code.

## The arc

- **Part 0 — C foundations.** Four throwaway sandbox programs: the pipeline and
  the memory, layout and linkage, bytes and pixels, loops and state.
- **Part 1 — the platform layer.** The codebase is born from a blank file;
  window, input, timing, and file I/O against the OS (Linux/X11 first).
- **Part 2 — software rendering.** Every pixel is written by code we own.
- **Part 3 — sound.** Music and effects through our own mixer.
- **Part 4 — services.** Arenas, entities, asset formats — and a vertical slice
  that draws a hero walking a tilemap with the camera following.
- **Part 5 — the game.** Feel, enemies, waves, and a fixed three-pass
  optimization ending in a frame-budget report.

The mandatory deep dives — compiler/linker internals, virtual memory, caches,
SIMD/assembly reading — sit inside the parts that need them.

## How this site is organized

Lessons appear in curriculum order. Every lesson contains prose, exactly one
code step, and its exercises, and targets 30-60 minutes of read-and-code time.
Each exercise prompt ends with a link to its solution: a diff against the
lesson's end state plus a short walkthrough. Nothing in a prompt spoils its
solution.

Lesson code lives in one linear history tagged `lesson-NNN`. If a lesson's tag
moves under you, resync with:

```
git checkout lesson-NNN -- src/
```
