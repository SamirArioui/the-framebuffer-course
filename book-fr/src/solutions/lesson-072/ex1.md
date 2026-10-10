# Solution : exercice 1 — La table avec un trou

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La table avec un trou](../../lessons/part-4/lesson-072-load.md) de la leçon 072.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-072/ex1.patch}}
```

## Visite guidée

Le diff, c'est un fichier et un chargement : `assets/holed.txt`, les deux
définitions avec une ligne vide entre elles, chargé à côté de la table propre du
cours. La sonde imprime ce que le chargeur a répondu, combien de lignes il a
remises, et le compte d'octets utilisés de l'arena des deux côtés du chargement
— les trois choses que l'exercice vous demande de prédire, sur une seule ligne.

Les prédictions, avant toute exécution. **Le verdict est `TABLE_MALFORMED`.**
Les lignes du format finissent à la première ligne vide et la queue après elles
ne peut contenir que des lignes vides et rien d'autre ; la ligne `slime` après
le trou est une ligne là où une queue appartient, et le chargeur refuse le
fichier plutôt que de décider si cette ligne appartient à cette table ou à une
autre. **Zéro définition est remise** — un chargement refusé ne remet rien, pas
la partie qui s'est analysée. **L'arena est inchangée à travers le chargement**
— la marque descend avant que les lignes soient prises et le refus fait un
retour arrière jusqu'à elle.

L'exécution, de l'état final de cette leçon plus le patch :

```
engine: table: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/sprite.ppm
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm
engine: holed: malformed, 0 rows, arena 1252040 -> 1252040
```

Les trois tiennent. Maintenant la marche qui a décidé chaque réponse, qui est la
partie qu'il vaut la peine de savoir dire à voix haute :

- **La marche de comptage** a regardé la ligne de l'en-tête puis les lignes, et
  s'est arrêtée à la ligne vide. Elle a compté **une** ligne — `hero` — et a
  demandé à l'arena exactement les octets d'une définition. Elle n'a jamais vu
  `slime` comme une ligne ; pour le compte, les lignes du fichier finissaient au
  trou.
- **La marche de remplissage** a analysé l'en-tête et rempli la seule ligne
  qu'on lui avait promise, puis s'est arrêtée à la même ligne vide. C'est la
  vérification de la queue après elle — `trailing blank lines, and nothing else`
  — qui a rencontré la ligne `slime` et a dit non. Voilà le verdict : pas « une
  ligne manque », pas « le fichier est trop court », mais la règle du format sur
  ce qui peut suivre les lignes.
- **Le retour arrière** est ce qui rend le troisième nombre identique à
  l'avant-dernier. La seule ligne que le remplissage a écrite a été de vrais
  octets dans l'arena pendant un moment ; le refus les a rendus.

Cette dernière ligne est la raison pour laquelle la sonde prend la peine
d'imprimer l'arena. Sans la marque et le retour arrière, le rapport lirait
`arena 1252040 -> 1252140` — les cent octets d'une définition d'un fichier que
le chargeur a *dit* refuser, gardés pour le reste de l'exécution et comptés dans
chaque rapport d'arena après elle. Un chargement refusé ne terminerait pas le
jeu ; un millier d'entre eux (un écran de chargement qui réessaie un fichier
cassé, un éditeur de niveaux qui sonde des candidats) serait la mémoire du jeu,
disparue.

Ce que l'exercice ne change **pas**, c'est le verdict du bon fichier ou le jeu :
`assets/entities.txt` se charge exactement comme avant, et le rapport propre de
l'exécution n'est pas touché. La sonde est une vérification à côté du
chargement.

*Page traduite de la version anglaise `book/solutions/lesson-072/ex1.md`,
révision `2794ee0`.*

<!-- translation-source: book/solutions/lesson-072/ex1.md @ 2794ee0 -->
