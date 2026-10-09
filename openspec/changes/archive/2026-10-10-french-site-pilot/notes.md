# french-site-pilot — change notes

Record of what the pilot actually built and what it actually measured. Every
number below was read off the tree or the checker during this change; where
something was not measured, it says so instead of estimating.

## What exists now

| | |
| --- | --- |
| `book-fr/` | The French book root: `book.toml`, `src/` mirroring `book/`'s paths |
| French pages | 18 — `index.md`, `SUMMARY.md`, `stability-horizon.md`, 3 lessons, 12 solutions |
| `tools/lang-switch.js` | Per-page switch between editions, loaded by both books |
| `book-fr/lang-switch.js` | Symlink to the above (mdBook copies `additional-js` only from inside the book root) |
| `tools/check-fr-sync.sh` | The translation-fidelity checker (marker, code, drift, horizon) |
| `plan/translation-conventions.md` | The translation authoring contract |
| `.github/workflows/deploy-site.yml` | Builds English → checks → builds French → publishes `site/` |
| `README.md` | Language claim, French local-run and build commands, the checker |

Pilot slice: the site shell plus lessons 001-003 with all their exercises'
solutions — every content shape the course has (prose, code steps, exercise
prompts, diff solutions, includes, navigation).

## Measured during the pilot

**Size and throughput proxy (prose words per page, code blocks excluded):**

| Page | English words | French words | Fenced blocks |
| --- | --- | --- | --- |
| `index.md` | 293 | 484 | 1 |
| `lessons/part-0/lesson-001-first-program.md` | 1012 | 1147 | 2 |
| `lessons/part-0/lesson-002-gdb.md` | 1681 | 1924 | 5 |
| `lessons/part-0/lesson-003-char-buffers.md` | 1331 | 1562 | 6 |
| `solutions/lesson-001/ex1..ex4` | 200 / 226 / 243 / 228 | 229 / 278 / 284 / 271 | 1 / 1 / 1 / 2 |
| `solutions/lesson-002/ex1..ex4` | 151 / 206 / 192 / 188 | 196 / 266 / 241 / 247 | 2 / 2 / 2 / 3 |
| `solutions/lesson-003/ex1..ex4` | 176 / 228 / 187 / 187 | 209 / 280 / 239 / 231 | 2 / 2 / 2 / 3 |
| `stability-horizon.md` | 161 | 179 | 0 |
| **Content total** | **6890** | **8267** | **37** |

- French prose runs **1.20× the English word count** on the same pages
  (8267 / 6890). That ratio is the planning number for later batches: a part's
  English word count times 1.2 is what the French tree will hold.
- `SUMMARY.md` is excluded from the ratio: the English one carries all 103
  lessons (2991 words), the pilot's carries 3 (221 words).
- **Wall-clock translation time per page was not instrumented** in this
  change, so no hours-per-page figure is claimed here. A next batch should
  time its translation pass and add that column.

**Checker, in real use:** `tools/check-fr-sync.sh` runs over 18 pages in one
pass and was green at the end. It found one real defect while the pilot was
being written: `book-fr/src/solutions/lesson-002/ex4.md` declared revision
`a3b4a6a` while its English source stands at `bb8d4ab` — a mistyped marker,
not drift. The drift check named the page and both revisions before any human
reviewed it.

**Checker, planted defects (6, all named their page and recovered to clean):**
missing marker · marker naming a non-existent English page · marker naming a
non-mirrored page · a changed code block (named "block 2") · a stale declared
revision · a mismatched frozen range between the two banners.

**Rendered equality:** every `<pre>` block of all 15 rendered lesson and
solution pages is byte-identical to its English counterpart's, in the built
output — the patches, included cross-tree from the English `exN.patch` files,
are equal by construction.

**Link integrity:** 129 links across the built French site resolve, 0 dead —
including the cross-edition links to the English edition.

## Convention amendments (decided by this batch, per `plan/translation-conventions.md` §2)

- **Exercise archetype tags** (`predict-the-output`, `fix-the-crash`, …) are
  kept in English: they name the six archetypes of the `exercises` spec, as
  identifiers. Added to the term table.
- **Links into not-yet-translated territory** are raw HTML anchors to the
  English page's *output* path, labelled *(en anglais ; traduction à venir)* —
  the source tree's depth and the output tree's depth differ, so a markdown
  link cannot serve both (design D3). Added as a rule in §2.
- **Fragments and `SUMMARY.md`** carry the comment marker only; the visible
  line appears on rendered content pages, so the banner does not repeat it.

## Decisions taken while implementing

- **No-JS fallback narrowed to the French home page** (user decision,
  2026-10-09): task 2.1 asked for static cross-links on both editions' home
  pages, which conflicted with the proposal's "English tree untouched" and the
  `localized-site` spec's "English tree untouched" scenario. The switcher is
  JS on both editions; the static cross-link lives on the French home page
  only. `design.md` D3 and `tasks.md` 2.1 were amended to say so.
- **`build-dir` spelling corrected** from `../../site/fr` to `../site/fr`:
  mdBook resolves `build-dir` against the *book root* (`book-fr/`), so the
  design's spelling landed outside the repository. The first build caught it;
  `design.md`, `tasks.md`, and `book-fr/book.toml` now agree. No spec or
  behavior change — the target is still `site/fr/`.
- **`book-fr/lang-switch.js` is a symlink** to `tools/lang-switch.js`: mdBook
  copies an `additional-js` file into the output only when it lives inside the
  book root, and a path outside it is referenced but not copied (verified
  against mdBook 0.5.4). Trade-off: a Windows checkout without symlink
  support gets a text file there; the authoring platform is Linux/WSL and CI
  is Linux.
- **lesson-003's *Next* link** points at the English lesson-004 page, labelled
  as English — the pilot's boundary is mid-part, and a French link must never
  404 (§2's new rule).

## Findings for the English side (reported, not fixed here)

- `book/index.md` says "*a free, open, English-language written course of
  ~130 medium lessons*" while the course is 103 lessons — README.md has said
  103 since `454b954`. The French home page follows its source faithfully
  ("environ 130 leçons") rather than silently diverging; a class-2 fix to
  `book/index.md` would let the next French batch redeclare against the
  corrected revision (`plan/conventions.md` §3).

## Verification reruns

- `./tools/check-fr-sync.sh` → `OK 18 French page(s) in sync` (exit 0).
- `mdbook build` then `mdbook build book-fr` → `site/` with both editions;
  the workflow's own step order, YAML validated.
- Clean clone at `HEAD` + this change's exact files, README-only steps: the
  English HTML path set is **unchanged** (363 pages before and after), the
  French edition browses from `fr/`, and the switcher round-trips a lesson
  page and a solution page in both directions.
- Both `mdbook serve` and `mdbook serve book-fr` run as documented, each
  serving its edition.
