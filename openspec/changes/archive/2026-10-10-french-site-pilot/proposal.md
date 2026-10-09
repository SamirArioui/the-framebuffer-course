# Proposal

## Why

The course is published as "a free, open, English-language written course", and
the README's own audience line — a developer fluent in Python or Ruby — is not
limited to English readers. The site deploys to GitHub Pages from one mdBook
source, so a French reader today has no path through the material at all. A
French edition should be a first-class second edition of the same course, not a
fork and not a live machine-translation layer: the English tree is frozen at
`lesson-001`…`lesson-103`, which makes now the right moment to establish how a
translation follows a frozen source without silently drifting from it.

This change is a **pilot**: it builds the whole French vertical — layout, build,
deploy, switching, translation conventions, and the fidelity checks — on a small
real slice (the site shell plus Part 0's first three lessons with their
solutions), so the remaining parts can be translated later as ordinary batches
against a proven pipeline.

## What Changes

- **A parallel French book** at `book-fr/` (its own mdBook root, `language =
  "fr"`), mirroring the English tree's paths: `SUMMARY.md`, `index.md`, the
  translated stability-horizon banner, `lessons/part-N/…`,
  `solutions/lesson-NNN/…`. The English `book/` tree is not touched.
- **Publication of both editions on the same Pages site**: `mdbook build` keeps
  writing the English site to `site/`; the French book builds to `site/fr/`,
  after the English build (whose build step clears its build directory). The
  deploy workflow builds both books in that order.
- **A language switcher** on every page of both editions, mapping the current
  page to its counterpart in the other edition at any depth — rendered by a
  small shared authoring script (`tools/lang-switch.js`, loaded via mdBook's
  `additional-js`), not per-page links.
- **French pages carry a source marker** naming the English page path and the
  English revision the translation follows — the honesty mechanism that keeps a
  translation answerable to a frozen source.
- **Code stays byte-identical between editions**: code steps, commands, symbol
  names, and exercise patches are never translated. French solution pages
  include the English `exN.patch` files directly, and a checker
  (`tools/check-fr-sync.sh`) verifies code-block identity and source markers
  mechanically.
- **Translation conventions** (`plan/translation-conventions.md`): terminology
  (what is translated, what is kept in English), the marker format, and the
  batch-and-drift procedure for later parts — the authoring contract for
  translations, beside `plan/conventions.md`.
- **Pilot content**: the French site shell (home page, `SUMMARY.md` in
  curriculum order, banner) plus `lesson-001`…`lesson-003` with their twelve
  solution pages — enough to prove every content shape the course has (prose,
  code steps, exercises, diff solutions, includes) on a reviewable slice.

## Capabilities

### New Capabilities

- `localized-site`: the second language edition as a parallel book — its
  layout, its local build, its publication beside the English edition at its
  own path, per-page language switching, and navigation and stability banners
  that mirror the English edition.
- `translation-fidelity`: the honesty contract for translated content — every
  translated page names the English source and revision it follows, code and
  patches stay byte-identical to the English edition, and drift is detected
  mechanically rather than by review.

### Modified Capabilities

None. The English site's requirements (`course-website`, `curriculum`,
`lesson-format`, `exercises`) are unchanged; the French edition must satisfy
them, which `localized-site` requires rather than restates.

## Impact

- **New**: `book-fr/` (French book source), `tools/lang-switch.js`,
  `tools/check-fr-sync.sh`, `plan/translation-conventions.md`.
- **Changed**: `book.toml` and a new `book-fr/book.toml` (both load the
  switcher), `.github/workflows/deploy-site.yml` (build both books, English
  first), `README.md` (the "English-language" claim, local-run and build
  instructions for both editions).
- **Unchanged**: `book/`, `src/`, `assets/`, `tools/check-boundary.sh`,
  `tools/frame-card.sh`, every `lesson-NNN` tag. No lesson code, prose, or tag
  moves; the English edition keeps its exact published behavior.
- **Authoring cost**: the pilot slice is small; the per-page cost of later
  batches is a translation pass plus the marker line, verified by the checker.
- **Risk surface**: build ordering (English build clears `site/`, so French
  must build second) and cross-edition link depth — both handled by the design's
  build order and the path-derived switcher.

## Assumptions (recorded, minor)

- The pilot slice is the site shell plus lessons 001-003 with all their
  solutions; later parts follow as separate changes reusing this pipeline.
- The course title stays **The Framebuffer Course** as a proper name in both
  editions; French pages carry a French description and metadata.
- French is the only translation in scope; the design names a second language
  as a same-shape repeat, not as work this change does.
