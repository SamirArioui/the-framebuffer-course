# The Framebuffer Course

*From C foundations to a finished 2D arcade game on an engine you wrote
yourself.*

A free, open, English-language written course: 103 medium lessons (30-60
minutes each) that take a developer fluent in Python or Ruby — with zero
C/C++ and no manual-memory experience — from C foundations to a finished 2D
top-down arcade game running on a fully hand-written engine. No external
libraries appear in any student-visible C/C++ code; the game is built against
the OS behind a platform layer we write ourselves.

The arc: **Part 0** C foundations (throwaway sandbox programs) → **Part 1**
platform layer (Linux/X11 first) → **Part 2** software rendering →
**Part 3** sound (our own mixer) → **Part 4** services → **Part 5** the game
(feel toolkit + a fixed 3-pass optimization). The mandatory deep dives —
compiler/linker internals, virtual memory, caches, SIMD/assembly reading — are
placed inside the parts that need them, not filed away as asides.

## Repository layout

| Path       | What it holds                                                            |
| ---------- | ------------------------------------------------------------------------ |
| `book/`    | The course site: lesson prose, exercises, and solutions (mdBook source)  |
| `src/`     | The engine: one linear history, each lesson's end state tagged `lesson-NNN` |
| `tools/`   | Build, check, and asset tooling                                          |
| `plan/`    | Authoring artifacts (conventions, contracts, part skeletons) — never published to the site |
| `build.sh` | The one-command code build                                               |
| `openspec/`| Planning artifacts behind the course's design decisions                  |
| `site/`    | *(generated)* static site output from `mdbook build`                     |
| `build/`   | *(generated)* compiled output from `./build.sh`                          |

The site is built only from `book/`, so `plan/`, `src/`, and `openspec/` never
appear in published output. Part 0's `sandbox/` (four throwaway programs) was
deleted in the `lesson-026` code step, as designed; every Part 0 state remains
retrievable from its tag (`git checkout lesson-025 -- sandbox/`).

## Prerequisites

- **gcc** (or clang) and **gdb** — Part 0 teaches the toolchain explicitly; for
  this audience the toolchain is curriculum
- **git**
- **libasound2-dev** — the ALSA development headers, the sound device's
  interface (Part 3). On Debian/Ubuntu: `apt install libasound2-dev`. The
  `libx11-dev` analog for the sound line; a machine without a working output
  still runs the engine, which reports the missing device as a typed failure.
- **mdBook 0.5.4** (pinned) — site authoring tooling. The course's "no external
  libraries" rule governs student-visible C/C++ only, not authoring tools.
- **openspec** (CLI) — to validate the planning changes under `openspec/`

## Pinned tooling

The site is built with **mdBook 0.5.4** — the pin protects a multi-year
authoring arc from tooling drift (`book/` markdown stays the source of truth and
is tool-agnostic). Install exactly that version:

```
curl -sL -o /tmp/mdbook.tar.gz https://github.com/rust-lang/mdBook/releases/download/v0.5.4/mdbook-v0.5.4-x86_64-unknown-linux-gnu.tar.gz
tar -xzf /tmp/mdbook.tar.gz -C /tmp
install -m 755 /tmp/mdbook ~/.local/bin/mdbook
```

Cargo users can install the same pinned version instead. Check the pin with
`mdbook --version`, which must print `mdbook v0.5.4`.

## Run the site locally

```
mdbook serve
```

Then open the URL mdBook prints (by default <http://localhost:3000>). The whole
course is browsable on your own machine; editing anything under `book/`
reloads the page.

## Build the site for publication

```
mdbook build
```

This writes a self-contained static directory to `site/` that needs no
server-side runtime and deploys to any static file host. **GitHub Pages is the
assumed deployment target**: publish the contents of `site/`.

## Deployment

The site deploys to GitHub Pages automatically: every push to `main` runs
`.github/workflows/deploy-site.yml`, which installs the same pinned mdBook
0.5.4, builds the site, and publishes `site/` to Pages. The published course
lives at <https://samirarioui.github.io/the-framebuffer-course/>. No manual
step is needed; trigger the workflow by hand from the Actions tab when a build
without a push is wanted.

## Build the code

```
./build.sh
```

Compiles every C/C++ source under `src/` and links `build/game`. On an empty
`src/` it reports that there is nothing to compile and exits 0. This one shell
script is the build — Make and CMake are never curriculum.

## Produce the 60 fps hardware card

The game's perf line — *60 fps on modest hardware* — is answered by a
**hardware card**: the machine named, the run's shape named, the report's
table with its percentiles and budget line, and the sentence the checklist
needs. One command produces it on whatever machine you are sitting at:

```
./tools/frame-card.sh
```

The game opens a window on your desktop: play hard (walk, fire, die,
restart), then close the window. The script builds the finished game with
lesson 101 exercise 1's percentiles instrument in a throwaway export of the
committed tree — your working tree is never touched — and writes
`frame-card.md` with the run's whole `frame-log.txt` beside it. Options:
`--optimize` for a `-O2` row (run it twice to carry both), `--auto SECONDS`
for a scripted xdotool run, `--machine NAME` for the machine line,
`--out DIR` for where the card lands. A card carries only its own machine's
numbers — a number that lost its machine is a rumor.

## Build the sandbox programs (Part 0 tags)

Part 0's throwaway programs under `sandbox/` did not use `build.sh`. Each one
builds standalone with **one literal `gcc` command, run from its own
directory** — no Makefile, no wrapper script. The lessons grow these commands
flag by flag; the lines below build each program as it stands at the end of its
part of Part 0:

```
cd sandbox/wordcount && gcc -std=c11 -O0 -g -Wall -Wextra wordcount.c -o wordcount
cd sandbox/ds-kit     && gcc -std=c11 -O0 -g -Wall -Wextra *.c -o ds-kit
cd sandbox/paint      && gcc -std=c11 -O0 -g -Wall -Wextra paint.c -o paint
cd sandbox/snek       && g++ -std=c++17 -O0 -g -Wall -Wextra *.cpp -o snek
```

Each line leaves the program's binary beside its sources. Through
`lesson-024` `snek` is C and its line reads `gcc -std=c11 -O0 -g -Wall
-Wextra snek.c -o snek`; from `lesson-025` on it is C++ and builds as shown
above. The `sandbox/` directory was deleted in the `lesson-026` code step, as
designed — restore it from a Part 0 tag (`git checkout lesson-025 -- sandbox/`)
before using the lines above.

## Validate the planning

```
openspec validate --all
```

Validates every spec and planning change under `openspec/`. To validate a
single change, name it instead (for example `openspec validate
part-0-c-foundations`).

## Lesson states, tags, and resync

Each published lesson's end-of-lesson code state is tagged `lesson-NNN` with
zero-padded sequence numbers, in the course repository:
<https://github.com/SamirArioui/the-framebuffer-course>. The site's
stability-horizon banner (driven by `book/stability-horizon.md`) names which
lessons are frozen and which are still subject to change.

If a tag moves — a behavior-changing fix in the frozen prefix is applied
surgically and lesson states are re-tagged — resync your working tree to the
current lesson state:

```
git checkout lesson-NNN -- src/
```

Replace `lesson-NNN` with the lesson you are working through. The full
authoring contract (tag format, co-commit rule, symbol-reference discipline,
the three-class revision policy, exercise rules) lives in
[`plan/conventions.md`](plan/conventions.md).
