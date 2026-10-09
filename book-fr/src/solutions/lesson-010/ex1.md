# Solution : exercice 1 — Les mêmes octets, autrement

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Les mêmes octets, autrement](../../lessons/part-0/lesson-010-void-pointer.md) de la leçon 010.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-010/ex1.patch}}
```

## Visite guidée

La prédiction sur n'importe quelle machine petit boutiste — ce que montre
cette exécution :

```
7 0
42 0
```

Chaque emplacement d'élément fait huit octets contenant un `long` : 7 est
`07 00 00 00 00 00 00 00`. Lire l'emplacement à travers `const int *` ne va
chercher rien de neuf — cela réinterprète les *mêmes* octets en deux entiers
de quatre octets, et sur une machine petit boutiste la moitié basse vient en
premier : `7`, puis la moitié haute toute en zéros, `0`. D'où `7 0`, et
`42 0` pour la même raison.

Sur une machine gros boutiste, les octets de 7 sont `00 00 00 00 00 00 00 07`,
aussi la prédiction bascule-t-elle vers `0 7` et `0 42` — même programme,
mêmes valeurs, réponse différente, et `void n'a pas d'opinion là-dessus. Rien
ici n'a touché une adresse invalide et aucun sanitizer ne se plaindra : les
octets étaient dans les limites, le transtypage était légal, et
l'interprétation est entièrement l'affaire de l'appelant. C'est exactement le
pouvoir et le danger du tableau générique — les octets se comportent
parfaitement et le type ne veut rien dire, et la leçon 006 a enseigné où
cette route finit.

*Page traduite de la version anglaise `book/solutions/lesson-010/ex1.md`,
révision `60d447b`.*

<!-- translation-source: book/solutions/lesson-010/ex1.md @ 60d447b -->
