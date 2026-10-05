# Tasks

## 1. Repository scaffold

- [ ] 1.1 Initialize the git repository and create the `book/`, `src/`, `tools/`, and `plan/` directory structure (`plan/` holds authoring artifacts and is excluded from the published site); verify the tree matches design.md D10 and `git log` shows a linear history.
- [ ] 1.2 Write `build.sh` as the one-command build that compiles every source under `src/`; verify it exits 0 on the empty tree and reports what it compiled.
- [ ] 1.3 Write `README.md` (course identity, repo layout, how to run the site locally, how to build, how to resync a moved lesson state, GitHub Pages as the assumed deployment target) and verify every command it documents runs exactly as written.
- [ ] 1.4 Write `plan/conventions.md` capturing the authoring contract: `lesson-NNN` tag format, prose+code co-commit rule, symbol-reference discipline, the three-class revision policy with the `git checkout lesson-NNN -- src/` resync path, the exercise density table and six archetypes, and the C++ subset admission policy; verify every rule matches the change's specs with no drift.

## 2. Course website skeleton

- [ ] 2.1 Create `book.toml` and `book/SUMMARY.md` with a landing page and verify `mdbook serve` presents the site locally (course-website: Local operation).
- [ ] 2.2 Add a lesson page template covering the prose, code-step, and exercises sections with the solution-link pattern at each prompt's end; verify a filled sample page renders with the solution link appearing after the prompt, never before it (lesson-format: Lesson anatomy; exercises: Solutions stay out of the prompt).
- [ ] 2.3 Add the stability-horizon banner driven by a single source-of-truth file (initial value: no lessons frozen yet) and verify it appears on the index and lesson pages (course-website: Stability horizon display).
- [ ] 2.4 Verify `mdbook build` produces a self-contained static directory deployable to any static host, and record the pinned mdBook version in `README.md` (course-website: Static publication).

## 3. M0 contract artifacts

- [ ] 3.1 Write `plan/target-game-mvd.md` with the frozen feature checklist, the 4-effect feel toolkit, the 3-pass optimization menu, and the definition of done; verify every item matches specs/target-game/spec.md with nothing added or dropped.
- [ ] 3.2 Write `plan/part5-skeleton.md` listing the vertical-slice capstone (L0*) and the full Part 5 lesson list (L1-L22) with a one-line goal per lesson; verify the 22-lesson count, the L16 debt lesson, and the L17-L20 optimization grouping match design.md D8.
- [ ] 3.3 Write `plan/part1-4-backlog.md` deriving backward obligations from the MVD (camera offsets, bitmap text, tilemap + collision, time-scale hook, archetype storage, per-channel audio, frame-timing instrumentation, and the L0* services gate); verify each checklist item maps to at least one obligation.

## 4. Integration checks

- [ ] 4.1 From a clean checkout following only `README.md`: run `openspec validate` on the change, build the site, and run `build.sh`; verify all three succeed.
- [ ] 4.2 Dry-run the lesson-tag and resync workflow on scratch commits (create a scratch lesson tag, move it via a simulated surgical fix, run the documented resync procedure, then remove the scratch tags); verify `plan/conventions.md`'s recovery procedure restores the tree to the current lesson state.
