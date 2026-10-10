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
| 3 | lessons 008-012 + solutions — the `ds-kit` arc complete | 23 | 9 630 | 11 433 | 1.19 | 2026-10-10 (`bc24616`) |
| 4 | lessons 013-018 + solutions — the `paint` arc complete | 30 | 11 534 | 13 551 | 1.17 | 2026-10-10 (`35d2a84`) |
| 5 | lessons 019-025 + solutions — the `snek` arc complete; **all of Part 0 translated** | 35 | 14 109 | 16 543 | 1.17 | 2026-10-10 (`f6392e2`) |
| 6 | Part 1 — the platform layer (lessons 026-043 + solutions) | 54 | 28 384 | 33 638 | 1.19 | 2026-10-10 |

Parts 0 and 1 are complete in French: the whole French tree is 179 pages,
81 414 EN → 94 010 FR prose words (summed from the batch rows; whole-tree
aggregates read lower than the per-page ratio because the English
`SUMMARY.md` is over ten times the pilot's), every fenced block byte-identical
to its English source. Remaining work, measured the same way:

| Batch | Scope | Pages | EN words |
| ----- | ----- | ----- | -------- |
| 7 | Part 2 — software rendering (044-058) | 45 | 29 940 |
| 8 | Part 3 — sound (059-070) | 36 | 33 795 |
| 9 | Part 4 — services (071-081) | 33 | 25 371 |
| 10 | Part 5 — the game (082-103) | 66 | 46 979 |

Parts 2-5 follow as their own batches; the same 1.20 per-page planning number
applies.

## Batch 6 — 2026-10-10

- **Pages:** the whole of Part 1 — 18 lessons (`026`-`043`) + 36 solutions;
  179 French pages total, **Parties 0 and 1 complete**.
- **Method:** translated by six parallel subagents (three lessons each) with a
  clean session each; the coordinator owned SUMMARY.md, the home page, the
  term table, and all verification. All 54 pages' fenced blocks were spliced
  verbatim by `splice_part.py` (the same mechanism, with `--patch` rewriting
  solution includes to the cross-tree patch path).
- **Throughput:** ratio 1.19 (28 384 EN → 33 638 FR prose words). Across six
  batches: 1.20 / 1.19 / 1.19 / 1.17 / 1.17 / 1.19.
- **Terms added to the table** (conventions §2): les nouvelles (news),
  acquisition/libération (take/release), chemin de sortie, sans écran
  (headless), appui/relâchement/frappe/touche maintenue, enregistrement de
  frame / journal / budget de frames, Kio/Mio/Gio in prose, adossé à une
  réservation, cadre de page, copie sur écriture, marque/retour arrière
  (arena), bump pointer kept English.
- **Boundary work:** Part 0 lesson-025's *Next* moved from the English
  fallback to the French lesson-026. lesson-043's footer mirrors English's
  `**Next: —` (Part 1's last lesson has no next link in either edition; one
  subagent had added an English fallback there and the coordinator removed
  it — navigation stays mirrored exactly).
- **Checker findings:** none — `OK 179 French page(s)` at the end.
- **Verification:** rendered `<pre>` blocks byte-identical to English on all
  176 lesson and solution pages; **1 455 links with 0 dead**; curriculum
  order matches the English summary for all 43 lessons.

## Batch 5 — 2026-10-10

- **Pages:** 7 lessons (`019`-`025`) + 28 solutions; 125 French pages total,
  and **Part 0 is fully translated** — all four sandbox arcs: `wordcount`,
  `ds-kit`, `paint`, `snek`.
- **Throughput:** ratio 1.17 (14 109 EN → 16 543 FR prose words). Across five
  batches: 1.20 / 1.19 / 1.19 / 1.17 / 1.17 — the ~1.20 planning number held
  for every batch. Wall-clock per page still not instrumented.
- **Terms added to the table** (conventions §2): *name mangling / vtable /
  vptr* kept in English (the reader meets them in `nm` output); *slice* →
  tranchement (slice), first use glossed.
- **Boundary work:** lesson-018's *Next* moved from the English fallback to
  the French lesson-019; lesson-025's *Next* — the first link out of Part 0 —
  now carries the fallback into Part 1 (lesson-026, untranslated).
  `SUMMARY.md` and the home page's edition note name the whole of Part 0 as
  translated.
- **Method note:** all seven lessons' fenced blocks, including four large
  code steps and lesson-025's whole-conversion diff, were spliced verbatim
  from the English source by script.
- **Checker findings:** none — `OK 125 French page(s)` at the end of the
  batch.
- **Verification:** rendered `<pre>` blocks byte-identical to English on all
  122 lesson and solution pages; **1 007 links across the built French site
  with 0 dead**; curriculum order matches the English summary for all 25
  lessons.

## Batch 4 — 2026-10-10

- **Pages:** 6 lessons (`013`-`018`) + 24 solutions; 90 French pages total in
  the tree now. Three arcs complete: `wordcount`, `ds-kit`, and `paint`.
- **Throughput:** ratio 1.17 (11 534 EN → 13 551 FR prose words) — the
  planning number holds across four batches (1.20 / 1.19 / 1.19 / 1.17).
  Wall-clock per page still not instrumented.
- **Terms added to the table** (conventions §2): *rasterize / rasterizer* →
  rastériser / rastériseur. Everything else the batch met was already covered
  (boutisme, pas, découpage, comportement indéfini, …).
- **Boundary work:** lesson-012's *Next* moved from the English fallback to
  the French lesson-013; lesson-018's *Next* now carries the fallback
  (lesson-019 is batch 5's first page). `SUMMARY.md` and the home page's
  edition note name lessons 001-018 as translated.
- **Method note:** all six lessons' fenced blocks — including three large
  code steps — were spliced verbatim from the English source by script
  (`/tmp/splice.py` pattern, same as batch 3's lesson-012), and the checker
  confirms byte-identity on every one.
- **Checker findings:** none — `OK 90 French page(s)` at the end of the
  batch.
- **Verification:** rendered `<pre>` blocks byte-identical to English on all
  87 lesson and solution pages; 720 links across the built French site with
  0 dead; curriculum order matches the English summary for all 18 lessons.

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
