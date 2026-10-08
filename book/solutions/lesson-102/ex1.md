# Solution: exercise 1 — our engine, honestly compared

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 102 — the retrospective: our engine against real ones](../../lessons/part-5/lesson-102-retrospective.md).*

## The diff

None, on purpose: this exercise writes no code, so `ex1.patch` is
empty — there is nothing to apply and nothing to check. The deliverable
is the page of prose the prompt asks for. A model answer follows.

## Walkthrough

A model one-pager, on the pixels — one of the six subsystems the
prompt names. Yours may pick any of them; the shape below is the one
to imitate: three claims with measured numbers, three facts from the
other side, one judgment.

---

**The pixels — ours against theirs.**

1. *Every pixel of a frame is the CPU's.* `ClearBuffer` fills all
   307,200 pixels (640×480) of the framebuffer, `DrawTileMap` blits
   the camera's cells through `BlitSprite`, and sprites and
   `FontGlyph` text land on top — measured at `lesson-101`'s state on
   WSL2/Xvfb at `-O0`: `clear 0.244 ms`, `tilemap 0.538 ms`, sprites
   and text together `0.017 ms` of a `1.313 ms` play frame (lesson
   101's ten-leg run).
2. *The frame leaves the engine through exactly one copy loop.*
   `platform::Present` carries the whole buffer to the window once per
   frame: `0.438 ms` of wall time for `0.004 ms` of CPU (lesson 098's
   seam split) — the wait is the X server's, not ours.
3. *The cost is attributable to the phase.* The frame account's rows
   sum to the total with no unnamed remainder (`clear + sprites +
   text + tilemap = render`), which is how the two hotspots were named
   and fixed (lessons 098-100).

Their side, three facts: frames are drawn through a GPU — command
lists from the CPU, textures in VRAM; the display's copy is a swap;
and frame cost moves off the CPU budget onto one the engine team
meets through driver counters and GPU profilers rather than through
their own code.

The judgment: the gap buys them scale — resolution, effect counts,
particles in the thousands — and costs them a driver stack, shader
toolchains, and frames that are hard to attribute line by line. Ours
buys the opposite: every byte is readable, every phase is named, and
an optimization is a diff you can verify against measured rows. For a
course — and for a game of this size — legibility is worth more than
the GPU's scale; for your game's ambitions, that trade is yours to
re-open, and the frame account is how you will know when to.

---

Three things the one-pager must do to satisfy the prompt, whatever
subsystem it picks:

1. **Every claim carries its machine.** The numbers above say `-O0`,
   WSL2, Xvfb — because D12 says a performance claim without its
   machine is a rumor. A number quoted from a lesson inherits that
   lesson's machine; a number you measure yourself names yours.
2. **The comparison is like with like.** A production engine's frame
   is not "faster at the same job" — it is a different job (GPU work
   the CPU never does). Judgments that compare our milliseconds to a
   shipped game's milliseconds without saying what each frame *does*
   are the exact dishonesty the retrospective refuses.
3. **The judgment says what ours buys.** The comparison is not an
   apology. Every gap has a price on their side and a purchase on
   ours; a one-pager that only lists what we lack has missed what the
   course built.
