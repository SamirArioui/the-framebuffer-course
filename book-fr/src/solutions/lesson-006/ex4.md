# Solution : exercice 4 — Le prix de la vérification

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Le prix de la vérification](../../lessons/part-0/lesson-006-undefined-behavior.md) de la leçon 006.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-006/ex4.patch}}
```

## Visite guidée

Sur une machine (gcc 13.3.0, x86-64), la construction simple `-O0` sur le
`med.txt` de 20 Mo — un octet envoyé par octet d'entrée, aussi `BufferAt`
tourne-t-il environ 20 millions de fois :

```
bounds-checked accessor:  real 0m0.138s   (0m0.139s on the second run)
raw indexing:             real 0m0.111s   (0m0.111s on the second run)
```

La vérification coûte environ un quart de l'exécution — abordable sur un
programme dont la construction sanitizer coûte déjà le double. Où va l'argent
à `-O0` : pas dans la comparaison. Le test de frontière est une comparaison
`size_t` ; le coût est que `BufferPush` fait un vrai appel de fonction à
`BufferAt` pour chaque octet, parce que `-O0` compile le source tel qu'écrit
et n'inline rien. C'est la comptabilité honnête pour les constructions de
débogage — et c'est aussi l'argument pour garder la vérification dans le
source au lieu d'écrire deux versions du code : les compilateurs qui
inlinent les petites fonctions plient la vérification dans l'appelant, et les
leçons de fin du programme `paint` de la partie 0 montrent ce que font les
constructions optimisées avec exactement cette sorte de code. Mesurez avant
de décider qu'une vérification de sécurité est trop chère ; la plupart sont
moins chères que le bug.

*Page traduite de la version anglaise `book/solutions/lesson-006/ex4.md`,
révision `51e4e75`.*

<!-- translation-source: book/solutions/lesson-006/ex4.md @ 51e4e75 -->
