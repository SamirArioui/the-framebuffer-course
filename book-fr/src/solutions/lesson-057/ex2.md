# Solution : exercice 2 — La table d'acceptation

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La table d'acceptation](../../lessons/part-2/lesson-057-demo.md) de la leçon 057.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-057/ex2.patch}}
```

## Visite guidée

Le patch fait en sorte que la démo nomme ce qu'elle utilise — une ligne par
groupe de capacités, imprimée avant la première frame :

```
engine: drawing: one blit for sprites, glyphs, and tiles; clipping and transparency included
engine: text: strings laid out one slot per character, missing glyphs skipped
engine: world: the map loaded whole, drawn through the camera's summed offset
engine: input: polled movement, collision-gated per axis; the camera follows, the additive shakes
engine: measurement: one record per frame, render attributed per subsystem
```

Ces cinq lignes sont la colonne de gauche de la table. Le reste est à vous de
remplir — et la forme qui la rend digne d'être conservée est celle-ci :

| Scénario de la spécification | Leçon | Preuves tirées d'une exécution |
| ---------------------------- | ----- | ------------------------------ |
| Les pixels du sprite sont dessinés inchangés | 045 | `blit check: 130 opaque pixels drawn unchanged, 0 mismatches` |
| La transparence n'écrit rien | 045 | `126 key pixels wrote nothing over the background` |
| Le dessin hors écran est découpé | 045 | `clip at -4,-4 landed 144 pixels … 0 touched outside` |
| Le texte est dessiné depuis la police | 050-051 | `font check: glyph 'A' … 0 mismatches` ; le HUD à l'écran |
| Les caractères manquants ne corrompent pas la disposition | 051 | `ink pixels per slot: 18, 0, 0, 23` |
| La carte apparaît au décalage de la caméra | 053-054 | `tilemap check … 0 mismatches` ; `camera check: base (100,50) scrolls the scene` |
| Le défilement garde la carte intacte | 053 | `map 768x512 px over frame 640x480 — 274365 pixels compared, 0 mismatches` |
| Le décalage additif se cumule / s'efface | 054 | `additive (7,-3) stacks over base … 0 mismatches` / `restores the base view` |
| Une carte se charge complètement | 052 | `1536 cells, 0 unknown` (les comptes se recoupent avec le fichier) |
| Un fichier manquant ou malformé est un échec typé | 052, 044 | `not a complete map`, `missing or unreadable` — par leur nom |
| Une tuile solide rapporte un chevauchement | 055 | `collision check: 12 of 12 answers as documented` |
| Une région vide ne rapporte aucun chevauchement | 055 | même table, les lignes libres |
| Les requêtes hors limites ont une réponse définie | 055 | les trois lignes de politique — solides, documentées |
| Seule la solidité des types décide | 055 | les lignes sol contre pilier, sans consulter le code de dessin |
| Enregistrement de frame / journal / cumul | 036, 046+ | chaque ligne `frame N:` ; les sommes par sous-système du cumul |
| La mesure derrière la frontière | 036 | `check-boundary.sh` — aucun chronométrage d'OS dans le code du moteur |

La dernière question — quelles lignes cassent en premier quand le moteur
change — est là où la table gagne son utilité :

- **Les lignes exactes au pixel cassent d'abord à tout changement de dessin.**
  Touchez au découpage du blit, à la vérification de la couleur clé ou à la
  permutation RGB→BGR, et les vérifications des leçons 045, 050, 053 passent au
  rouge immédiatement. C'est délibéré : ces lignes coûtent peu à relancer et
  disent précisément ce qui a cassé.
- **Les lignes de disposition cassent en silence.** Un changement de glyphe
  manquant ou de caméra plante rarement quoi que ce soit — l'écran est
  simplement en léger désaccord avec la spécification. Les cartes de créneaux
  et les comparaisons à deux dessins existent parce que « ça a l'air bon »
  n'est pas une vérification.
- **Les lignes de cumul dérivent plutôt qu'elles ne cassent.** Les nombres
  bougent avec la machine et avec le build ; ce qui ne doit pas changer, c'est
  la *forme* et le format. Les lignes à surveiller sont celles qui
  corrompraient les données que la partie 5 lit.

Comment le remarquer ? Exactement comme ce cours le fait depuis la leçon 001 :
la vérification est une commande, sa sortie est un nombre, et les nombres sont
relancés — pas relus — quand le moteur change.

*Page traduite de la version anglaise `book/solutions/lesson-057/ex2.md`,
révision `b2e596c`.*

<!-- translation-source: book/solutions/lesson-057/ex2.md @ b2e596c -->
