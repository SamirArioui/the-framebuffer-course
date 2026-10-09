# Tasks

## 1. Translation conventions and the French book skeleton

- [x] 1.1 Write `plan/translation-conventions.md` beside `plan/conventions.md`: the term table (what is translated vs. what stays English — symbol names, tool names, flags, file names, commands, code; the title **The Framebuffer Course** kept as a proper name), the source-marker format from design D4, the batch procedure (translate → check → declare), and the drift procedure (retranslate and redeclare, never redeclare alone); verify a reviewer can check a translated page against the document without asking the author anything.
- [x] 1.2 Create the French book root `book-fr/book.toml` (`language = "fr"`, `src = "src"`, `build-dir = "../site/fr"` — the path is relative to the book root —, `create-missing = false`) with `book-fr/src/SUMMARY.md`, `book-fr/src/index.md`, and `book-fr/src/stability-horizon.md` (translated banner naming the same frozen range `lesson-001`…`lesson-103` as `book/stability-horizon.md`); verify `mdbook build book-fr` succeeds and writes `site/fr/` without touching `book/` (`git status` clean outside `book-fr/`).
- [x] 1.3 Keep French page paths mirroring the English tree (`book-fr/src/lessons/part-0/…`, `book-fr/src/solutions/lesson-NNN/…`); verify every French page created so far has an English counterpart at the mirrored path and no English page was edited (`git diff --stat` over `book/` is empty).

## 2. Switcher and fidelity tooling

- [x] 2.1 Implement `tools/lang-switch.js` per design D3: derive both edition roots from the page's `path_to_root`, then render a switch to the counterpart page; load it from both books via `[output.html] additional-js`; verify from a deep page (`site/fr/lessons/part-0/lesson-001-….html`) the switch opens the English counterpart page and back, and that the French home page carries a static cross-link to the English home that works with JS disabled (the English home page is not edited).
- [x] 2.2 Implement `tools/check-fr-sync.sh` per design D6: per French page check (a) the `<!-- translation-source: <path> @ <revision> -->` marker exists, parses, and names an existing English page, (b) every fenced code block matches the English page's corresponding block byte-for-byte and in order, (c) drift — the English page's `git log -1` revision equals the declared revision, (d) both stability horizons name the same frozen range; verify each check fails with a named page and block when a deliberate defect is planted and passes when the defect is removed.

## 3. Pilot content: the shell and Part 0's first three lessons

- [x] 3.1 Translate the site shell: `book-fr/src/index.md` (the arc, how the site is organized, the resync instructions) and `book-fr/src/SUMMARY.md`'s part headings and lesson entries for the pilot slice; verify the French home page renders with the French banner, and the summary lists the pilot lessons in the same curriculum order as the English one.
- [x] 3.2 Translate `lesson-001`…`lesson-003` into `book-fr/src/lessons/part-0/` with their code-step `diff` blocks kept verbatim and each page carrying its source marker; verify `tools/check-fr-sync.sh` passes for these pages and `mdbook build book-fr` renders each one.
- [x] 3.3 Translate the twelve solution pages (`solutions/lesson-001`…`lesson-003`, `ex1`…`ex4` each) into `book-fr/src/solutions/`, keeping each exercise's `{{#include}}` pointed at the English `exN.patch` (design D5) and each page carrying its source marker; verify the checker's patch and code-block checks pass and the rendered solution shows the same diff as its English page.
- [x] 3.4 Verify exercise-to-solution linkage and per-lesson navigation in the French edition (`localized-site`: Solution linkage, Mirrored navigation); verify every exercise prompt links its French solution and each lesson page links its previous, next, part, and code tag.

## 4. Publication and documentation

- [x] 4.1 Extend `.github/workflows/deploy-site.yml`: after the English build, run `tools/check-fr-sync.sh` and `mdbook build book-fr` (English first, French second — design D2/D8), then upload `site/`; verify the workflow's build job produces `site/` containing both editions and that a failing checker stops the deploy.
- [x] 4.2 Update `README.md`: the course's language claim ("English-language, with a French edition"), the French local-run and build commands, and the note that the English build clears `site/` so it runs before the French build; verify every documented command runs exactly as written (local serve of each edition, both builds, the checker).

## 5. Integration checks

- [x] 5.1 End-to-end publication check on a clean clone (README-only steps): build both editions, run the checker, serve `site/`, and verify the English URLs are unchanged, the French edition browses from `fr/`, and the switcher round-trips a lesson page and a solution page (`localized-site`: Editions publish side by side, Language switching).
- [x] 5.2 Fidelity spot-check: plant one changed code block in a French lesson and one marker naming a stale revision; verify `tools/check-fr-sync.sh` reports both by page name and that restoring them returns it to clean (`translation-fidelity`: Divergence is caught, Drift is detected mechanically).
- [x] 5.3 Record the pilot's outcome (translation throughput per page, checker findings, any convention amendments) in the change's notes; verify the record names numbers measured during the pilot rather than estimates.
