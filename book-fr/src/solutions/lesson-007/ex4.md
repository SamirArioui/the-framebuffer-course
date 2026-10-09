# Solution : exercice 4 — La disposition de l'autre machine

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — La disposition de l'autre machine](../../lessons/part-0/lesson-007-struct-layout.md) de la leçon 007.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-007/ex4.patch}}
```

## Visite guidée

Avec un `long` de 4 octets, la règle d'alignement donne 20 octets pour
`struct Item` avec `value` au décalage 16 : la clé occupe les décalages 0-15,
et 16 est déjà un multiple du nouvel alignement (4), aussi aucun remplissage
n'apparaît. `struct Scattered` rétrécit à 12 — `tag` à 0, `score` à 4,
`flag` à 8, taille arrondie à l'alignement de structure de 4. Le contraste
24/16 de cette leçon se comprime aussi, mais réordonner gagne toujours.

La ligne d'empreinte donne à chaque machine une identité d'une ligne. La
machine de l'auteur, un x86-64 Linux, imprime :

```
fingerprint: char=1 int=4 long=8 ptr=8 item=24
```

Une construction 32 bits devrait imprimer `long=4 ptr=4 item=20`, et Windows
64 bits `long=4 ptr=8 item=20` — `long` y reste à 4 octets même si les
pointeurs font 8. Cette machine n'a pas d'en-têtes 32 bits, aussi ces deux
lignes sont-elles des prédictions, pas des exécutions : votre travail est de
les confirmer ou de les réfuter contre le calcul à la main ci-dessus. Cette
variance est pourquoi les programmeurs C se tournent vers `<stdint.h>` quand
une taille ne doit pas varier — la leçon 013 fait exactement cela. Si
`gcc -m32` fonctionne sur votre machine, exécutez le programme dans les deux
sens et regardez un seul source imprimer deux dispositions.

*Page traduite de la version anglaise `book/solutions/lesson-007/ex4.md`,
révision `fa0fc1a`.*

<!-- translation-source: book/solutions/lesson-007/ex4.md @ fa0fc1a -->
