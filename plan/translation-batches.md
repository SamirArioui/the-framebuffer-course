# Translation batches

The log every translation batch leaves behind, per
`plan/translation-conventions.md` §4 step 5. Each row is measured from the
tree after the batch lands — never estimated. The English word count is prose
only (code blocks excluded), and the ratio is the planning number for what
comes next: **an English page's word count × ~1.20 is what the French tree
holds**.

| Batch | Scope | Pages | EN words | FR words | Ratio | Landed |
| ----- | ----- | ----- | -------- | -------- | ----- | ------ |
| 1 (pilot) | site shell + lessons 001-003 + solutions | 15 | 6 890 | 8 267 | 1.20 | 2026-10-09 (`daa7088`) |
| 2 | lessons 004-007 + solutions — the `wordcount` arc complete | 19 | 7 876 | 9 334 | 1.19 | 2026-10-10 (`0328bc5`) |
| 3 | lessons 008-012 + solutions — the `ds-kit` arc complete | 23 | 9 630 | 11 433 | 1.19 | 2026-10-10 |

Remaining in Part 0, by sandbox program (measured the same way):

| Batch | Scope | Pages | EN words |
| ----- | ----- | ----- | -------- |
| 4 | `paint`: lessons 013-018 + solutions | 30 | 11 534 |
| 5 | `snek`: lessons 019-025 + solutions | 35 | 14 109 |

Parts 1-5 follow as their own batches once Part 0 is done.

## Batch 3 — 2026-10-10

- **Pages:** 5 lessons (`008`-`012`) + 18 solutions; 60 French pages total in
  the tree now. Two arcs complete: `wordcount` and `ds-kit`.
- **Throughput:** ratio 1.19 (9 630 EN → 11 433 FR prose words) — the
  planning number holds for a third batch. Wall-clock per page still not
  instrumented.
- **Terms added to the table** (conventions §2): `hook`, `callback`, `shim`
  kept in English; *stride* → pas, *hashtable/bucket/chain/load factor* →
  table de hachage / seau / chaîne / facteur de charge, *translation unit /
  object file / symbol table* → unité de traduction / fichier objet / table
  de symboles, *include guard / forward declaration* → garde d'inclusion /
  déclaration en avant, *linkage* → lien (interne/externe), *predicate* →
  prédicat.
- **Boundary work:** lesson-007's *Next* moved from the English fallback to
  the French lesson-008; lesson-012's *Next* now carries the fallback
  (lesson-013 is batch 4's first page). `SUMMARY.md` and the home page's
  edition note name lessons 001-012 as translated.
- **Method note:** lesson-012's code step is a ~550-line file-split diff; its
  fenced blocks were spliced verbatim from the English source by script
  rather than retyped, and the checker confirms byte-identity.
- **Checker findings:** none — `OK 60 French page(s)` at the end of the
  batch. A measurement bug of the *log's own* tooling was caught during
  recording: Python's `glob` does not expand `{08,09}` braces, which had
  silently skipped seven solution pages from a preliminary word count; the
  numbers above come from the corrected count.
- **Verification:** rendered `<pre>` blocks byte-identical to English on all
  57 lesson and solution pages; 474 links across the built French site with
  0 dead; curriculum order matches the English summary for all 12 lessons.

## Batch 2 — 2026-10-10

- **Pages:** 4 lessons (`004`-`007`) + 15 solutions; 37 French pages total in
  the tree now.
- **Throughput:** ratio 1.19 (7 876 EN → 9 334 FR prose words). Wall-clock per
  page is still not instrumented — no hours-per-page figure is claimed.
- **Terms added to the table** (conventions §2): `sanitizer`,
  `LeakSanitizer`, `AddressSanitizer`, `redzone`, `shadow memory`, `inline`
  kept in English; *padding* → remplissage, *alignment* → alignement,
  *wraparound* → retournement, *out-of-bounds access* → accès hors limites,
  *endianness* → boutisme.
- **Boundary work:** lesson-003's *Next* moved from the English fallback to
  the French lesson-004; lesson-007's *Next* now carries the English
  fallback (lesson-008 is batch 3's first page). `SUMMARY.md` and the home
  page's edition note updated to name lessons 001-007 as translated.
- **Checker findings:** none — `tools/check-fr-sync.sh` reported
  `OK 37 French page(s)` at the end of the batch, and every planted-defect
  class from the pilot still fails loudly (re-verified in batch 1's notes).
- **Verification:** rendered `<pre>` blocks byte-identical to English on all
  34 lesson and solution pages; 285 links across the built French site with
  0 dead; curriculum order matches the English summary for all 7 lessons.

## Batch 1 — 2026-10-09 (pilot)

Recorded in full in
`openspec/changes/archive/2026-10-10-french-site-pilot/notes.md`: measured
sizes per page, the checker's findings (including the shallow-clone drift
catch, whose fix landed in `66e943a`), the convention amendments the batch
decided, and the verification reruns.
