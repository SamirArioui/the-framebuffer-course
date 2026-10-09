# Solution : exercice 1 — Le calendrier des croissances

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le calendrier des croissances](../../lessons/part-0/lesson-008-dynarray.md) de la leçon 008.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-008/ex1.patch}}
```

## Visite guidée

La prédiction à battre : les capacités changent après les envois 1, 5 et 9 —
pas après 4 et 8. La branche de croissance tourne *avant* le stockage, sur
l'envoi qui trouve `len == cap` : l'envoi 5 arrive sur un bloc plein de quatre
emplacements et le double à huit, l'envoi 9 sur un bloc plein de huit et le
double à seize. Le `fprintf` de confirmation va sur `stderr`, aussi
`./ds-kit 2>growth.txt` garde-t-il les flux séparés et `growth.txt` contient
exactement :

```
grow 0 -> 4
grow 4 -> 8
grow 8 -> 16
```

Trois réallocations pour dix envois — et le motif des sauts est géométrique,
ce qui est tout l'argument des exercices suivants : quel que soit le nombre
d'envois, le calendrier de doublement maintient leur nombre à environ log₂
d'entre eux. Si vous avez prédit une croissance à 4 ou 8, regardez où dans
`DaPush` se situe le test de croissance par rapport à
`da->items[da->len++] = item` : la comparaison se fait contre la longueur
*avant* que le nouvel élément ne soit stocké.

*Page traduite de la version anglaise `book/solutions/lesson-008/ex1.md`,
révision `f4168c6`.*

<!-- translation-source: book/solutions/lesson-008/ex1.md @ f4168c6 -->
