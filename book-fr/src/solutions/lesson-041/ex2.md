# Solution : exercice 2 — Le bug qu'ASan ne voit pas

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le bug qu'ASan ne voit pas](../../lessons/part-1/lesson-041-arenas.md) de la leçon 041.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-041/ex2.patch}}
```

## Visite guidée

Un autre **état pédagogique délibéré** : cet exercice corrompt la mémoire exprès
pour montrer que l'outil auquel le cours fait confiance depuis la leçon 005 a
ici un angle mort. Le patch fait un retour arrière sur une allocation, garde
son pointeur, alloue de nouveau — et écrit à travers le pointeur périmé.

Construisez-le avec le sanitizer, comme la leçon 005 l'a enseigné :

```
$ CXXFLAGS="-std=c++17 -O0 -g -Wall -Wextra -fsanitize=address" \
  LDFLAGS="-lX11 -fsanitize=address" ./build.sh
$ DISPLAY=:99 ./build/game '?asan'
engine: stale write landed on the new block: 90 (same memory: yes)
$ echo $?
0
```

L'écriture à travers le pointeur périmé **a atterri sur la nouvelle
allocation** — `90`, c'est `0x5A`, l'octet écrit à travers le pointeur périmé,
relu à travers le pointeur frais. Deux allocations « différentes » sont la même
mémoire, et l'une a silencieusement écrasé l'autre. Et AddressSanitizer n'a
rien imprimé du tout : pas d'ERROR, aucun rapport sur les shadow bytes, code de
sortie 0.

Pourquoi ce silence ? ASan surveille les *allocations qu'il connaît* — le
trafic `malloc`/`free` qu'il intercepte — en marquant dans une shadow memory
les octets libérés et ceux jamais encore alloués. La mémoire de l'arena n'a
rien à voir avec cela : c'est un grand mappage anonyme que l'OS nous a remis
(leçon 040), et chaque « allocation » à l'intérieur est de l'arithmétique. Les
octets que `stale` désigne sont *mappés et possédés* à chaque instant de la vie
de ce programme ; du point de vue de la machine, rien d'illégal ne s'est
produit. ASan ne peut pas distinguer « l'espace libre de l'arena » de « l'espace
utilisé de l'arena », parce que l'arena ne le lui a pas dit — et un allocateur
à bump pointer n'appelle pas `free`.

Ce qui *l'attraperait*, par ordre de coût :

1. **Marques et discipline** — un pointeur après retour arrière est mort par
   convention, de la même façon que `free` rend un pointeur mort. Bon marché,
   et c'est la règle que le contrat de l'arena énonce déjà.
2. **L'empoisonnement** — une arena peut délibérément marquer sa région
   inutilisée comme inaccessible (l'exercice 2 de la leçon 040,
   `MakeInaccessible`) ou la remplir d'une sentinelle et la vérifier. Le
   service d'arena de la partie 4 pourra porter l'empoisonnement de débogage
   exactement comme le font les vrais moteurs.
3. **Un sanitizer qui sait** — les hooks d'allocateur personnalisé d'ASan
   existent pour cela (`__asan_poison_memory_region`) ; un build de débogage de
   l'arena peut leur parler. C'est l'aboutissement honnête, et cela vaut la
   peine de savoir que ça existe.

La rédaction à viser : les outils voient ce qu'on leur dit. Le sanitizer de la
leçon 005 était honnête au sujet du tas ; l'arena n'est pas le tas, et le même
bug est désormais invisible — ce qui est exactement pourquoi la limite est
enseignée ici plutôt que découverte en partie 4.

*Page traduite de la version anglaise `book/solutions/lesson-041/ex2.md`,
révision `3f18b81`.*

<!-- translation-source: book/solutions/lesson-041/ex2.md @ 3f18b81 -->
