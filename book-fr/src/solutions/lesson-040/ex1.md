# Solution : exercice 1 — Un octet, s'il vous plaît

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Un octet, s'il vous plaît](../../lessons/part-1/lesson-040-reservations.md) de la leçon 040.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-040/ex1.patch}}
```

## Visite guidée

Deux prédictions à faire avant de lancer : que dit `ReserveMemory(1)` de la
**taille** de la réservation, et quels sont les **premiers octets** d'une
mémoire que rien n'a encore écrite ?

```
$ DISPLAY=:99 ./build/game
engine: framebuffer reserved at 0x73ae9a0d4000 — 1228800 bytes = 300.00 pages (page-aligned: yes)
engine: asked for 1 byte: got 4096 bytes at 0x73ae9a618000 — first bytes 0 0 0
```

**Taille : 4096.** Un octet, c'est une page — le contrat disait « pages
entières », et un mappage n'est pas plus petit qu'une page. La réservation a
arrondi la demande au supérieur, la même arithmétique que le rapport mémoire
quand il compte les pages. (Si l'arrondi vous surprend, regardez l'adresse :
elle est alignée sur une page elle aussi, `...8000` à la fin. L'OS distribue
des pages ; les adresses sont là où commencent les pages.)

**Premiers octets : 0 0 0.** Pas « ce que le programme précédent a laissé » —
zéro. Les pages anonymes sont *mises à zéro à la demande* (demand-zero) : l'OS
ne donne pas du tout de mémoire physique à la réservation avant que les octets
soient touchés, et quand il en donne, ce sont des zéros. La sonde décrite dans
la prose de la leçon a mesuré exactement cela : l'ensemble résident n'a pas
grandi quand la réservation a été prise ni quand trois octets ont été lus ; il
a grandi des 1,2 Mo complets seulement quand chaque octet a été écrit.

Un octet demandé, une page d'espace d'adressage, zéro cadre de page physique,
trois octets à zéro à la première lecture. Telle est toute la forme d'une
réservation.

*Page traduite de la version anglaise `book/solutions/lesson-040/ex1.md`,
révision `e9826f2`.*

<!-- translation-source: book/solutions/lesson-040/ex1.md @ e9826f2 -->
