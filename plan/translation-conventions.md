# Translation conventions

The authoring contract for translated editions of the course — how a
translation follows its English source without drifting from it. Every rule
below maps to a requirement in this change's specs
(`openspec/changes/french-site-pilot/specs/`); no rule is allowed to drift
from them. `plan/conventions.md` governs the course itself; this document
governs its translations. French is the first translation; a second language
follows the same rules with its own tree.

## 1. The unit of translation

- A **page** is one markdown file of the edition: a lesson, a solution, the
  home page, or a shared fragment (`SUMMARY.md`, `stability-horizon.md`).
  *(translation-fidelity: Source marker on every translated page)*
- French page paths **mirror** the English tree's paths exactly — the same
  directory structure and the same file names, under `book-fr/src/`. A
  counterpart is derivable, never looked up.
  *(localized-site: Parallel edition)*
- The **English tree is never edited** for a translation: not `book/`, not the
  lesson tags. A translation that needs an English fix reports it (§5) instead
  of making it.
  *(localized-site: English tree untouched)*

## 2. What is translated, what is kept

**Translated:** prose, headings, part names ("Partie 0 — Fondations en C"),
lesson titles' descriptive half ("Leçon 001 — argv et saisie de fichiers : …"),
exercise prompts, solution walkthroughs, the banner, and the site chrome
(`index.md`, `SUMMARY.md`).

**Kept byte-identical, in English:**

- everything the learner types or runs: commands, compiler flags, file names;
- everything that names code: symbol names, type names, macro names, string
  literals that appear in output;
- tool names (`gcc`, `gdb`, `git`, `mdbook`), and the course title **The
  Framebuffer Course** as a proper name;
- code: fenced blocks and exercise patches. A translated page carries the same
  fenced block as its English page, and solution pages include the English
  `exN.patch` files rather than carrying copies of their own.
  *(translation-fidelity: Code is identical across editions)*

**Terminology (the term table).** Established French computing vocabulary is
used where it exists; the pilot's terms:

| English | French | Note |
| ------- | ------ | ---- |
| byte | octet | |
| buffer | tampon | |
| stack / stack frame | pile / trame de pile | |
| heap | tas | |
| breakpoint | point d'arrêt | |
| debugger | débogueur | tool name `gdb` stays English |
| linker | éditeur de liens | |
| preprocessor / compiler / assembler | préprocesseur / compilateur / assembleur | |
| segmentation fault | erreur de segmentation | |
| undefined behavior | comportement indéfini | |
| memory leak | fuite de mémoire | |
| frame, sprite, tilemap, hitstop, screenshake, diff, patch, commit, tag, build | kept in English | loanwords French developer usage keeps |
| sanitizer, LeakSanitizer, AddressSanitizer, redzone, shadow memory, inline | kept in English | tool and mechanism names; the reader meets them as-is in the output |
| hook, callback, shim | kept in English | loanwords French developer usage keeps |
| padding / alignment | remplissage / alignement | |
| wraparound | retournement | first use glossed "retournement (wrap)" |
| out-of-bounds access | accès hors limites | |
| endianness | boutisme | |
| stride | pas | |
| hashtable / bucket / chain / load factor | table de hachage / seau / chaîne / facteur de charge | |
| translation unit / object file / symbol table | unité de traduction / fichier objet / table de symboles | |
| include guard / forward declaration | garde d'inclusion (include guard) / déclaration en avant | |
| linkage (internal/external) | lien (interne/externe) | |
| predicate | prédicat | |
| rasterize / rasterizer | rastériser / rastériseur | |
| name mangling / vtable / vptr | kept in English | toolchain terms the reader meets in `nm` output |
| slice (object slicing) | tranchement (slice) | first use glossed |
| platform layer / seam | couche plateforme / couture | seam already used for lesson 012's linker seam |
| event pump | pompe à événements | |
| framebuffer / present | kept in English | code-visible names; prose capitalizes `Present` as a symbol |
| poll (input) | scrutation (en scrutant) | |
| latch (input) | mémoriser (latch) | first use glossed; prose verb "mémoriser" |
| virtual memory / page table / mapping | mémoire virtuelle / table de pages / mappage | |
| reservation / commit (memory) | réservation / validation (commit) | |
| arena | arena | kept: it names `struct Arena` in the sources |
| MMIO / TLB / MMU / X11 / Xlib / DISPLAY / pty | kept in English | platform and tool names |
| news (event pump) | les nouvelles | |
| take / release (OS resource) | acquisition / libération | verbs: acquérir / libérer |
| exit path | chemin de sortie | |
| headless(ly) | sans écran | |
| press / release / tap / held key | appui / relâchement / frappe / touche maintenue | |
| frame record / frame log / frame budget | enregistrement de frame / journal / budget de frames | |
| KiB / MiB / GiB (prose) | Kio / Mio / Gio | quoted outputs keep `KiB`; MB → Mo in prose |
| reservation-backed / demand-zero | adossé à une réservation / demand-zero (glossed) | |
| page frame / copy-on-write / bump pointer | cadre de page / copie sur écriture / kept in English | |
| mark (ArenaMark) / rollback | marque / retour arrière | |
| tile / tilemap / blit / sprite | tuile / tilemap / blit / sprite | tilemap, blit, sprite kept; tile translated |
| font / glyph | police (de caractères) / glyphe | first use of "police" glossed "(font)" |
| cache / cache line / miss / hit | cache / ligne de cache / défaut de cache / succès de cache | cache stays "cache" in French |
| prefetch / memory bandwidth | préchargement / bande passante mémoire | |
| SoA / AoS / SIMD / lane | kept in English / kept in English / kept in English / voie | data-layout and ISA jargon the code names |
| vectorization | vectorisation | |
| profiler / profiling | profileur / profilage | |
| viewport / scrolling | zone d'affichage / défilement | matches lessons 015/022 French |
| mover / AABB / overlap / solid tile | mover kept / AABB kept / chevauchement / tuile solide | mover names the code's type |
| kind (tile kind) | type | |
| blitter / knockback / hitbox / HUD | kept in English | loanwords; same family as hitstop/screenshake |
| key color / checksum / sheet / ink | couleur clé / somme de contrôle / planche / encre | |
| advance (width) / kerning | avance / crénage | |
| hotspot / working set / sweep / substep | point chaud / ensemble de travail / balayage / sous-pas | |
| sample / sample rate / sample frame | échantillon / fréquence d'échantillonnage / trame d'échantillons | |
| stream (audio) / mixer / to mix / mix (noun) | flux (audio) / mixeur / mixer / mixage | mixeur established by the course home page |
| channel / playback / to play | canal / lecture / jouer | |
| waveform / sine wave | forme d'onde / sinusoïde | |
| clipping (audio) | écrêtage | distinct from clipping (graphics) → découpage |
| gain / fade / loop (music) / one-shot | gain / fondu / boucle (boucler) / one-shot kept | |
| device / underrun / latency | périphérique / underrun kept / latence | |
| feed (audio) / pull (mixer) | alimentation (alimenter) / tirage (tirer) | feed established by lesson 060 |
| gate | barrière (audio feed, lesson 060) / porte (services checkpoint, lesson 080) | two senses, scoped by context |
| clamp (audio) | écrêter / écrêtage | distinct from clamp (geometry) → bornage |
| tone / peak / loudness / starvation | tonalité / crête / sonie / famine | |
| pool / chunk / ducking / one-shot | kept in English | ducking keeps its verb "ducker" |
| archetype (table) / row | archétype / ligne | |
| store (entity store) | magasin (store) | first use glossed; the `Store` symbol stays |
| free slot / free list / walk (entities) | emplacement libre / liste libre / marche | matches Part 2's "marche" |
| entity / hero | entité / héros | |
| game time / game-time scale / wall clock | temps de jeu / échelle du temps de jeu / horloge murale | matches lesson 020 |
| vertical slice / typed failure | tranche verticale / échec typé | matches the course home page |
| retirement (entities) / live entity | retrait (retirer) / entité vivante | `EntityRetire` symbol stays |
| movement request / slide / pathfinding | requête de déplacement / glissement / recherche de chemin | |
| scratch probe / high-water mark / headroom | sonde jetable / marque du plus haut emplacement / marge | |
| balance / health / facing | équilibrage / vie (points de vie) / facing kept | facing is the column name in prose |
| knob / wall step / wall time | bouton / pas mural / temps mural | |
| spawn / wave / bounty | apparition (faire apparaître) / vague / prime | |
| boss / particles / score / high score | boss kept / particules / score / meilleur score | |
| debt / retrospective / hand-over | dette / rétrospective / passation | |
| fix-forward | corriger en avançant (fix-forward) | first use glossed |
| measure pass / fix pass / three-pass optimization | passe de mesure / passe de correction / optimisation en trois passes | |
| pattern (boss) / projectile / to fire | schéma / projectile / tirer | |
| feedback / damage / hit / kill | rétroaction / dégâts / coup / mise à mort | |
| weapon / shot / cooldown / rate / range | arme / tir / temps de recharge / cadence / portée | |
| roster / readout / stand-in | effectif / indicateur / substitut | |
| sprite sheet / walk cycle / stride (animation) | planche de sprites / cycle de marche / foulée | |
| wind-up / enrage / keeper / tunneling | préparation / enrage kept / gardien / tunneling kept (glossed) | |
| overshoot / anticipation (camera) / sawtooth | dépassement / anticipation / en dents de scie | |
| report template / frozen menu / hand-over card | gabarit de rapport / menu figé / carte de passation | |
| leg (of a played run) / aliasing / clear (noun) | manche / aliasing kept / clear kept | |
| exercise archetype tags (`predict-the-output`, `fix-the-crash`, `extend-the-code`, `measure-the-performance`, `explain-in-prose`, `port-to-your-own-machine`) | kept in English | they name the six archetypes of the `exercises` spec, as identifiers |
| `REPL`, `traceback`, `backtrace`, `sandbox` | kept in English | loanwords or program names the pilot's pages use as-is |

A batch that meets a term this table does not cover decides it, writes it into
the table, and names the decision in the change's notes — the table grows by
decision, never by accident.

**Links to pages not yet translated.** A French page links to a translated
page by its French path. A link into a part of the course the batch has not
translated yet (a lesson's *Next* into untranslated territory, say) is a raw
HTML anchor to the English page's **output** path — `../../..` up to the site
root, then the English page's path with `.html` — labelled *(en anglais ;
traduction à venir)*. A raw anchor is deliberate: mdBook neither rewrites nor
validates it, and the source tree's depths and the output tree's depths differ
(design D3). A French link must never 404.

## 3. The source marker

Every French page ends with one machine-readable line and the same facts as
visible prose:

```
<!-- translation-source: book/lessons/part-0/lesson-001-first-program.md @ 57b9b3e -->
```

```
*Page traduite de la version anglaise `book/lessons/part-0/lesson-001-first-program.md`,
révision `57b9b3e`.*
```

- The path is the English page's path in this repository; the revision is the
  English commit the translation follows (`git log -1 --format=%h -- <path>`).
  *(translation-fidelity: Source marker on every translated page)*
- The comment is what `tools/check-fr-sync.sh` parses; the visible line is
  what the learner sees. Neither replaces the other.
- Shared fragments and `SUMMARY.md` carry the comment marker; the visible line
  appears on rendered content pages only (home, lessons, solutions), so the
  banner does not repeat it on every page.

## 4. The batch procedure

A **batch** is a slice of English pages translated together (the pilot: the
shell plus lessons 001-003 with their solutions).

1. Translate the prose; copy every fenced block verbatim; point solution pages'
   includes at the English patches.
2. Add each page's source marker, declaring the English pages' **current**
   revisions.
3. Run `tools/check-fr-sync.sh`; fix everything it reports.
4. Build and read the pages once (`mdbook build book-fr`).
5. Record the batch in the batch log (`plan/translation-batches.md`): pages
   translated, throughput measured, checker findings, terms added to the
   table. A batch that rides an OpenSpec change records it in the change's
   notes too.
   *(translation-fidelity: Terminology follows the stated conventions)*

## 5. The drift procedure

When `tools/check-fr-sync.sh` reports a page as **behind**, its English source
moved after the revision the page declares:

1. Translate the page again from the current English page.
2. Update its marker to the new revision.
3. Re-run the checker.

**A marker is never redeclared without retranslating.** Bumping the revision to
silence the report is a false claim that the French text says what the English
text now says — the one dishonesty this whole contract exists to prevent.
*(translation-fidelity: Drift is detected mechanically)*

The checker reports drift per English page: a commit that touches no page a
translation declares invalidates nothing.

## 6. Reviewer checklist

A reviewer who knows no French can clear a page with four questions:

1. Does its marker name an existing English page at a revision equal to that
   page's current revision? (`tools/check-fr-sync.sh` answers this.)
2. Do its fenced blocks and its presented diff match the English page's?
   (the checker answers this too)
3. Do the terms in §2's table appear untranslated where the English page uses
   them (symbol names, tools, flags, files)?
4. Does the page render, link its solutions, and show the banner's frozen
   range? (the build answers this)

Only the prose itself needs a French reader.
