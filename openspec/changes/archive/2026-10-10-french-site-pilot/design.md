# Design

## Context

The English course is one mdBook 0.5.4 book: root-level `book.toml` (`src =
"book"`, `build-dir = "site"`), markdown under `book/` (`index.md`,
`SUMMARY.md`, `stability-horizon.md`, `lessons/…`, `solutions/…`), deployed by
`.github/workflows/deploy-site.yml` to GitHub Pages at
<https://samirarioui.github.io/the-framebuffer-course/>. Every page includes its
banner by path (`{{#include ../../stability-horizon.md}}`), solution pages
include `exN.patch` files, and lesson code steps live as fenced ` ```diff `
blocks inside the lesson page. The tree is frozen (`lesson-001`…`lesson-103`),
and `plan/conventions.md` is the single authoring contract. See proposal.md —
Why for motivation and scope.

Two mdBook 0.5.4 behaviors were verified empirically before this design was
written (throwaway books under `/tmp`):

1. `mdbook build` **clears its build directory** before writing — a stray file
   in `site/` disappears when the English build runs.
2. A second book root builds fine with `build-dir` pointing into another
   book's output tree, `{{#include}}` **can reach outside the book's own source
   tree**, and each page exposes a `path_to_root` JS variable.

Constraints that shape the how: the English tree is untouchable (frozen
prefix, tags, published URLs); the no-external-libraries rule governs
student-visible C/C++ only, not authoring tooling; and this course's honesty
culture ("a number that lost its machine is a rumor") applies to translation
claims as much as to performance claims.

## Goals / Non-Goals

**Goals:**
- One publication carrying both editions, each self-navigable, with a working
  per-page switch between counterparts.
- A translation that is answerable to its source: named English page, named
  English revision, mechanically checked code identity and drift.
- A pipeline and convention set that later batches (Parts 0-5 in full) run
  through unchanged — the pilot is a slice of content on the final machinery.

**Non-Goals:**
- No retranslation policy debate: the pilot proves the contract, review passes
  over French prose quality happen in the batches.
- No third language in this change — the design is language-agnostic but only
  French is built.
- No restructuring of `book/`, no change to lesson code, tags, or the English
  build's one-command story.
- No runtime machine translation, no per-page duplicated patch files.

## Decisions

### D1. The French book is its own mdBook root at `book-fr/`

mdBook 0.5.4 resolves a book only from a directory holding `book.toml`
(`mdbook build [dir]`, no alternate-config flag), so a second edition needs a
second root. Layout:

```
book-fr/
  book.toml          # language = "fr", src = "src", build-dir = "../site/fr"
  src/
    SUMMARY.md
    index.md
    stability-horizon.md      # translated banner (same frozen range)
    lessons/part-0/…          # mirrors book/lessons/…'s paths
    solutions/lesson-001/…    # mirrors book/solutions/…'s paths
```

French markdown mirrors the English tree's *paths* (so counterpart pages are
derivable — `localized-site`: Mirrored counterpart) even though the root
differs (`book-fr/src/` vs `book/`). *Alternative considered:* a flat `book-fr/`
with `src = "."` to match the English tree's flatness — rejected: the config
file would sit inside the markdown tree, and `SUMMARY.md` must be the source
root's summary.

### D2. Build order is English first, French second

Because `mdbook build` clears its build directory, building English after
French would delete `site/fr/`. The documented procedure and the workflow
therefore run `mdbook build` (English → `site/`) then `mdbook build book-fr`
(French → `site/fr/`), and the French book's `build-dir` is `../site/fr` so
each edition is still one command. *Alternative considered:* building each
edition to its own output tree and merging with `rsync` — rejected: it breaks
the README's one-command story for the sake of a hazard that a fixed order
removes.

### D3. The language switcher is one shared script, not per-page links

`tools/lang-switch.js`, loaded by both books via `[output.html] additional-js`,
adds a switch to every page. It derives both edition roots from the page's own
`path_to_root` (French root ends in `fr/`; its parent is the site root), then
relinks the current page's path within the edition — so the switch always opens
the *counterpart page*, at any depth, locally (`/`) and on Pages
(`/the-framebuffer-course/`). *Alternatives considered:* per-page markdown
links — rejected: the authoring tree's depth (`book-fr/src/lessons/part-0/x.md`
→ `book/…` is four levels up) differs from the built site's depth
(`site/fr/…` → `site/…` is three), so no static relative link can serve both,
and absolute links break under the Pages base path. A theme `index.hbs`
override — rejected: it pins us to mdBook's template across versions for the
same effect a 40-line additive script achieves. Fallback when JS is off: the
French home page carries a plain static link to the English home (both sit at
edition roots, so that link is depth-safe). The English home page gets no such
link — `book/` stays untouched (`localized-site`: English tree untouched) — so
the no-JS path exists only on the side the French edition owns.

### D4. The source marker is a machine-readable comment plus a visible line

Every French page ends with:

```
<!-- translation-source: book/lessons/part-0/lesson-001-first-program.md @ bbe74f5 -->
```

and carries the same facts as visible prose ("*Page traduite de la version
anglaise du `bbe74f5`.*"). The comment is what `tools/check-fr-sync.sh` parses;
the visible line is what the learner sees. One line per page, greppable, no
front matter (the English pages have none).

### D5. Patch identity is structural: French solutions include the English patches

French solution pages `{{#include}}` the English `exN.patch` files cross-tree
(e.g. `{{#include ../../../../book/solutions/lesson-001/ex1.patch}}`), so the
diff a French learner sees is the English diff by construction, with zero
duplication across the eventual 360+ solution pages. Verified working in
mdBook 0.5.4. *Trade-off:* the French tree builds only from the repo, not as a
standalone checkout — acceptable, since every build runs from the repo.
*Alternative considered:* copying patches into `book-fr/` and checking
equality — rejected: hundreds of duplicate files whose equality only the check
would maintain. Inline code-step diffs cannot use this mechanism (extracting
them would mean editing the frozen English tree), so they travel as verbatim
fenced blocks in the French page and are checked instead (D6).

### D6. `tools/check-fr-sync.sh` is the enforcement behind the fidelity contract

One script, run by hand and in the deploy workflow, verifying per French page:
(a) the marker exists, parses, and names an English page that exists; (b) every
fenced code block matches the corresponding block of the English page
byte-for-byte and in order (pulled out of both files and diffed); (c) drift —
the English page's own `git log -1` revision equals the marker's declared
revision, or the page is reported as behind, naming both revisions; (d) both
`stability-horizon.md` files name the same frozen range. A failure names the
page and the block; the workflow stops. *Note:* drift is per English page, not
per tree — a bookkeeping commit elsewhere does not invalidate a translation.

### D7. Translation conventions live in `plan/translation-conventions.md`

Beside `plan/conventions.md`, never published. It carries the term table
(what is translated — prose, headings, exercise prompts; what stays English —
symbol names, tool names, flags, file names, commands, code; the course title
stays **The Framebuffer Course** as a proper name), the marker format (D4), the
batch procedure (translate → check → declare), and the drift procedure (when an
English page moves: retranslate the page and redeclare its revision — never
edit the marker to chase the new revision without retranslating).

### D8. Deployment builds both books in one workflow

`.github/workflows/deploy-site.yml` gains a second build step after the English
one and before `upload-pages-artifact` (which uploads `site/`, now including
`fr/`), plus a `tools/check-fr-sync.sh` step. `README.md`'s "English-language"
claim becomes "English-language, with a French edition", and its local-run and
build sections gain the French commands. Rollback is removing the French steps:
the English site is byte-identical either way.

## Risks / Trade-offs

- [mdBook tightens include scoping and the cross-tree patch include stops
  resolving] → The build fails loudly at the include; the documented fallback
  is copying the patches into `book-fr/` and letting D6's block check own their
  equality. No spec change needed either way.
- [Someone builds French then English locally and wonders where `site/fr/`
  went] → D2's ordering is stated in README's build section and in the
  workflow; the checker runs last and would notice the missing edition.
- [A marker gets edited to silence drift without retranslating] → The marker
  names a revision, not a claim of freshness; D7 forbids redeclaring without
  retranslating, and (b)'s block check still guards code identity regardless.
- [French search index duplicates the English one and mdBook's
  `WARN search index is very large` appears for both books] → Known, harmless,
  same as the English site's existing warning; each edition searches its own
  content.
- [The switcher script is the one moving part in otherwise static output] → It
  is additive and fails soft (no switch rendered if it does not run); the
  static cross-link on the French home page remains as the no-JS path.

## Migration Plan

No content migration. Land the change, push, and the workflow publishes both
editions; English URLs are untouched (D2/D8). Rollback: drop the French build
step and `site/fr` stops being published — the English site does not depend on
anything this change adds. Later batches (full Part 0, then Parts 1-5) are
content-only changes that run the same pipeline and add no new machinery.

## Open Questions

- Whether later batches are planned one per part or per half-part depends on
  measured translation throughput from the pilot; the specs and this design
  hold either way.
- The exact French wording of the home page and the term table entries are
  settled when the pilot slice is written, not here.
