# From C to a Finished Game

*A working title — the course's final name is still to be decided.*

A free, open, English-language written course: ~130 medium lessons (30-60
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
appear in published output.

## Prerequisites

- **gcc** (or clang) and **gdb** — Part 0 teaches the toolchain explicitly; for
  this audience the toolchain is curriculum
- **git**
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

## Build the code

```
./build.sh
```

Compiles every C/C++ source under `src/` and links `build/game`. On an empty
`src/` it reports that there is nothing to compile and exits 0. This one shell
script is the build — Make and CMake are never curriculum.

## Validate the planning change

```
openspec validate course-curriculum-foundation
```

## Lesson states, tags, and resync

Each published lesson's end-of-lesson code state is tagged `lesson-NNN` with
zero-padded sequence numbers. The site's stability-horizon banner (driven by
`book/stability-horizon.md`) names which lessons are frozen and which are still
subject to change.

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
