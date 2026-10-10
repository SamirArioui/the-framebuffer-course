# Solution : exercice 1 — Le dump de la police

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le dump de la police](../../lessons/part-2/lesson-050-font.md) de la leçon 050.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-050/ex1.patch}}
```

## Visite guidée

Le patch redessine les 96 glyphes là où la découpe les a trouvés — `BlitSprite`
à `32 + (k % 16) * 8, 32 + (k / 16) * 8`, l'arithmétique propre de la planche
parcourue à l'envers — puis balaie tout le dump avec `GetPixel` et compare
chaque pixel aux octets du sprite du glyphe, les pixels clés étant vérifiés
pour *ne pas* avoir écrit. L'exécution :

```
engine: font dump: 96 glyphs drawn, 0 pixel mismatches
```

**Zéro écart sur 6 144 pixels** (96 × 64) : la découpe est sans perte. Les
pixels de la planche et ceux des sprites de glyphes concordent partout, ce qui
est l'affirmation que fait le chargeur et la seule qui compte en aval — le
texte de la leçon 051 fera entièrement confiance à ces sprites.

Vient maintenant la question de diagnostic, et c'est elle qui rend le dump
digne d'être dessiné. Un compte non nul n'est pas juste « un bug » —
*l'endroit* où tombent les écarts dit quelle ligne est fausse :

- **Des écarts dans chaque glyphe, toujours le même motif** — l'arithmétique
  interne de la découpe est fausse *systématiquement* : le pas source (la
  largeur de la planche) ou le pas destination (la largeur de cellule) est
  faux. Chaque glyphe est découpé de la même mauvaise façon, donc chaque glyphe
  échoue de la même façon. Le premier glyphe que vous vérifiez (`A`) le montre
  déjà ; le dump ne fait que confirmer qu'il ne s'agit pas d'un cas isolé.
- **Des écarts dans exactement un glyphe** — la *correspondance des cellules*
  est fausse pour un index : `cell_x = k % 16` / `cell_y = k / 16` est de
  l'arithmétique par glyphe, et une erreur là (un `+1`, un modulo inversé)
  déplace une cellule — le glyphe reçoit les pixels d'un voisin. Le dump le
  localise : la *position du glyphe cassé dans le dump* nomme le `k` qui a
  échoué.

Cette lecture à deux niveaux — systématique contre localisée — est la même
habitude que les vérifications octet par octet de la leçon 044 ont entraînée :
quand une vérification échoue, la *forme de l'échec* est une donnée. Une forme
de plus mérite d'être nommée : des écarts dans toute une *ligne* du dump
pointeraient vers l'arithmétique des lignes (`k / FONT_COLS`), et des écarts
uniquement dans le dernier glyphe de chaque ligne, vers le modulo. Le dump est
un tableau de scores de 96 cellules, et chaque motif de bug y a sa signature.

*Page traduite de la version anglaise `book/solutions/lesson-050/ex1.md`,
révision `2230eb0`.*

<!-- translation-source: book/solutions/lesson-050/ex1.md @ 2230eb0 -->
