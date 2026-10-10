# Solution : exercice 1 — Le refus qu'on ignore

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le refus qu'on ignore](../../lessons/part-4/lesson-074-store.md) de la leçon 074.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-074/ex1.patch}}
```

## Visite guidée

Le diff est la requête *au-delà* de la pleine, traitée comme le contrat le dit :
l'erreur lue d'abord, le refus pris en compte, et l'exécution qui continue avec
les 64 entités qu'elle a. La branche `else` est là à dessein — c'est la branche
qu'une exécution correcte ne prend jamais et qu'une exécution cassée prendrait,
et sa présence rend la vérification visible dans le code.

Maintenant le crash que l'exercice vous demande de produire d'abord. Sortir
l'`EntityResult` du script hors de la boucle, puis utiliser l'entité de la
dernière requête juste après —

```cpp
    made.entity->x += 1; /* the run moves the entity it just made */
```

— est une écriture à travers un pointeur que le contrat dit être 0.
Reconstruite avec l'instrument de la leçon 013
(`CXXFLAGS="-std=c++17 -O0 -g -Wall -Wextra -fsanitize=address"
LDFLAGS="-fsanitize=address -lX11 -lasound"`), l'exécution meurt avant la
première frame :

```
AddressSanitizer:DEADLYSIGNAL
=================================================================
==479187==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000010 (pc ... T0)
==479187==The signal is caused by a READ memory access.
==479187==Hint: address points to the zero page.
    #0 ... in engine::Run() src/main.cpp:268
```

L'adresse `0x10` n'a rien de mystérieux : c'est la page zéro plus le décalage de
`x` dans `Entity` — le champ `x` du pointeur nul, à seize octets. Le sanitizer
nomme la ligne ; l'adresse nomme ce que le code croyait avoir.

Un pli à connaître, car c'est ainsi que ce bug se cache. La *première* version du
code cassé n'a pas planté du tout : elle imprimait `made.entity->name` à travers
le même pointeur nul et le `printf` de glibc répondait `(null)` — un `%s` qui
arrive comme pointeur nul est imprimé, pas déréférencé. L'exécution continuait
comme si rien n'allait, et le bug attendait une ligne qui touche vraiment aux
champs de l'entité. Un contrat se vérifie en lisant l'erreur, jamais en le
sondant par l'accès qui se trouve survivre.

La correction est la vérification que le contrat demande, dans l'ordre où il la
demande : `if (past.error != ENTITY_OK)` avant que quoi que ce soit lise
`past.entity`. D'une vraie exécution de l'état final de cette leçon plus le
patch :

```
engine: store: live 64 of 64 — the hero and 63 from the script
engine: store: creation refused (full), arena 1253648 -> 1253648 — creation allocates nothing
engine: store: the request past the full one refused (full) — the run carries on with 64 live
engine: closed
```

L'exécution survit au magasin (store) plein, nomme le refus, et se termine
proprement —
`engine: closed`, la même fin que donne tout autre chemin. C'est ce que « le jeu
décide ce que signifie un refus » veut dire en pratique : la réponse du magasin
est une valeur, et son traitement par le jeu est du code ordinaire qui peut se
tromper de façons ordinaires — c'est exactement pourquoi la réponse est typée
depuis le début. Une erreur qui arrive sous forme de crash est une erreur que
personne ne peut traiter.

Rien ici ne touche au magasin, à la règle de création ou à la boucle du jeu : le
patch est une requête et son traitement, à côté de celles de la leçon.

*Page traduite de la version anglaise `book/solutions/lesson-074/ex1.md`,
révision `cc57259`.*

<!-- translation-source: book/solutions/lesson-074/ex1.md @ cc57259 -->
