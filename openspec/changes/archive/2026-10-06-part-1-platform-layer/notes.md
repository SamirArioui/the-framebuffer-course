# Change notes

## Consistency read: lesson-025 C++ subset wording ↔ conventions §5 ↔ lesson-026's language-law section

**Date:** 2026-10-06 · **Task:** 1.1 · Fulfils the promise in `plan/part0-review.md`
("the L25 C++ subset wording will get a consistency read against Part 1's
engine-opening lesson when that is planned").

**Read:** `book/lessons/part-0/lesson-025-cpp-subset.md` (prose + code step)
against `plan/conventions.md` §5 and design D1's Birth batch ("the admitted
C++ subset is documented as the engine's language law (conventions §5)").

**Verdict:** 025 and §5 agree on the policy and on the five admitted
features. Six wording points need explicit reconciliation in lesson-026's
language-law section; none requires editing 025 (all are 026-side
clarifications, so the Part 0 tag does not move).

### Wording that lesson-026 must reconcile

1. **Templates: "never" vs. the tooling carve-out.** 025 says "Templates are
   never admitted into game code" and lists "no templates" among what is not
   in `snek`; §5 says "Templates appear only in clearly labeled tooling code".
   026's law must state §5's precise rule — templates are outside the engine
   subset and may appear only in clearly labeled tooling code — not a flat
   "no templates", which would forbid the case §5 permits.

2. **What "classes with vtables" covers.** 025 names five features, but its
   code step uses more class surface than the names spell out: constructor
   initializer lists, `public:`/`private:` access control, single inheritance
   (`class GridView : public Drawable`), pure virtual `= 0`, and `const`
   member functions. Since the policy is "enforced by review, not tooling",
   026's law must enumerate what "classes" admits so the review has a
   checkable boundary.

3. **The excluded list is prose in 025, not policy.** 025 excludes
   "exceptions, the STL, `new`/`delete` churn" and RTTI is never mentioned;
   §5 implies them only by listing the admitted five. 026's law must carry the
   exclusions explicitly (exceptions, STL containers, `new`/`delete`,
   RTTI/`dynamic_cast`) — consistent with design D4, where engine memory comes
   from reservations and arenas, never `new`/`delete`.

4. **The trailing-underscore convention.** 025 announces "private members
   carry a trailing underscore — the convention this course adopts for them";
   §5 does not contain it. 026's engine code must obey it and the language-law
   section should restate it (a course convention announced in prose must not
   silently vanish at the engine's birth).

5. **`main` stays global; the engine's namespace is named in 026.** 025's rule
   — `main` must be the global namespace's `main`, everything else lives in
   the namespace, global `main` forwards — must survive into 026, whose code
   step is born as a blank `main` in `src/`. 026 names the engine's namespace
   (025's `snek` was program-specific) and keeps `main` as the one global.

6. **The language standard is pinned by 025's build line.** 025's build
   command pins `g++ -std=c++17 -O0 -g -Wall -Wextra` at zero warnings; 026's
   `build.sh` must carry the same standard and warning flags (or the law must
   say the standard changed), so "the admitted subset" means the same language
   in both lessons.

### Admission timing (no conflict, but 026 must not re-admit)

§5: "Each feature is admitted in the lesson that explains its code
generation, and the subset is documented in the first engine lesson." 025
explained and admitted all five in one lesson; 026's language-law section is
therefore a **restatement and boundary-setting** of the admitted set, not a
new admission. Any C++ construct 026's code introduces beyond the five-plus-
boundaries above must pass the "explain what it compiles down to" test on its
own or stay out of the code step.

*Verified: this read is recorded here before lesson-026 is authored.*

## Toolchain verification for the platform batch

**Date:** 2026-10-06 · **Task:** 1.2 · All commands below run as written on
this authoring machine (headless).

### Versions used

| Tool | Version |
| ---- | ------- |
| g++ / gcc | 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) |
| libx11-dev (Xlib headers + `-lX11`) | 2:1.8.7-1build1 |
| Xvfb | 2:21.1.12-1ubuntu1.8 |
| xdotool (scripted input for 033-034 checks) | 3.20160805.1 |
| xkbcomp (keymap tooling under Xvfb) | present |
| mdBook | 0.5.4 (pinned) |

### What was verified

1. **Xlib development headers** — `/usr/include/X11/Xlib.h` and `Xutil.h`
   present; a probe compiles with
   `g++ -std=c++17 -O0 -g -Wall -Wextra probe.cpp -o probe -lX11`.
2. **`./build.sh` compiles `src/`** — with a temporary source in `src/`, the
   script compiles and links `build/game` exactly as documented
   (`build: OK (1 source(s) compiled -> build/game)`); the probe source and
   `build/` were removed afterwards, `src/` is empty again as the birth
   requires.
3. **Xvfb-driven headless check** — `Xvfb :99 -screen 0 800x600x24
   -nolisten tcp` started on a machine with no display; a test window opened,
   a known 4×4 pattern was presented via `XImage`/`XPutImage`, and an
   `XGetImage` readback matched all 16 pixels. The presentation scenario is
   checkable headlessly, as the design's risk table assumed.

*Note for later tasks:* 033-034's scripted-input checks use `xdotool` against
the same Xvfb display; installed and versioned above.
