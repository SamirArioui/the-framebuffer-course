<!--
  The stability horizon — the single source of truth for the banner shown at
  the top of every page of the site. Edit this file only; pages include it via
  mdBook's {{#include}} and never restate it. When a frozen prefix exists,
  name the frozen lessons and the volatile tail here.
-->

> **Stability horizon — `lesson-001`…`lesson-103` are frozen; the volatile
> tail is empty.** The course is complete — all six parts, from the `wordcount`
> arc to the finished game — and every `lesson-NNN` state is stable. A frozen
> tag moves only for a behavior-changing correctness fix, applied surgically
> with the downstream states re-tagged. If a tag moves under you, restore your
> working tree to the current lesson state with `git checkout lesson-NNN --
> sandbox/` (lessons 001-025, Part 0's code), `git checkout lesson-NNN -- src/`
> (lessons 026-043, the engine before assets), or `git checkout lesson-NNN --
> src/ assets/` (lessons 044-103, once assets exist).
