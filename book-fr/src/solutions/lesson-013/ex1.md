# Solution : exercice 1 — Trois pixels, à la main

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Trois pixels, à la main](../../lessons/part-0/lesson-013-raw-bytes.md) de la leçon 013.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-013/ex1.patch}}
```

## Visite guidée

Le nouvel appel atterrit à la colonne 2 de la ligne 0, aussi son décalage est
`0 * (8 * 3) + 2 * 3 = 6` : trois octets à partir de la position 6 de la ligne.
Les douze premiers octets prédits sont donc

```
FF 00 00 00 FF 00 11 22 33 00 00 00
```

— rouge, vert, le nouveau `11 22 33`, puis trois pixels noirs intacts — et
l'exécution imprime exactement cela. Notez ce qui n'a *pas* bougé : les pixels
déjà dans le tampon gardent leurs octets, parce que `PutPixel` écrit
exactement trois d'entre eux et rien d'autre. Si votre prédiction était
fausse, vérifiez si vous avez compté le nouveau pixel à `2 * 3 = 6` ou à
`2 * 1 = 2` ; ce dernier est le bug du pas oublié de l'exercice 3, et c'est la
façon la plus courante dont cette arithmétique va mal. Compter des octets à la
main une fois, lentement, vaut plus que trois intuitions.

*Page traduite de la version anglaise `book/solutions/lesson-013/ex1.md`,
révision `66eaafc`.*

<!-- translation-source: book/solutions/lesson-013/ex1.md @ 66eaafc -->
