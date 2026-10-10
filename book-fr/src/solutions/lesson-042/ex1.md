# Solution : exercice 1 — Le second OS

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le second OS](../../lessons/part-1/lesson-042-contract.md) de la leçon 042.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-042/ex1.patch}}
```

## Visite guidée

La preuve qu'une couture est un contrat, c'est une seconde implémentation de
celui-ci — et celle-ci est la plus petite qui puisse exister : chaque fonction
que l'en-tête déclare, définie par le refus. Aucune fenêtre ne peut s'ouvrir
(`OPEN_NO_DISPLAY`), aucun fichier ne peut être lu (`FILE_NOT_FOUND`), aucune
mémoire ne peut être réservée (`MEMORY_NO_MEMORY`). Un OS stub qui ne fait
rien, honnêtement.

La partie intéressante n'est pas le stub. C'est la **ligne de liaison** :

```
$ g++ -std=c++17 -O0 -g -Wall -Wextra -Isrc \
      src/main.cpp src/framebuffer.cpp src/frame.cpp src/arena.cpp \
      tools/platform_stub.cpp -o build/game-stub
$ ./build/game-stub
engine: no display to open a window on
$ echo $?
1
```

Le moteur — chaque fichier du moteur, inchangé depuis l'état final de la leçon
— s'est compilé et lié contre la seconde implémentation. Aucune bibliothèque
X11 sur la ligne : `tools/platform_stub.cpp` est tout l'OS, et le moteur ne
remarque pas la différence. Il démarre, demande sa fenêtre, reçoit l'échec typé
que le stub rapporte, et sort par le même chemin d'erreur que sur n'importe
quelle machine sans affichage.

C'est le scénario de la spécification rendu exécutable : *quand une
implémentation pour un autre OS est ajoutée derrière l'interface, seuls les
fichiers de la couche plateforme changent*. Le fichier modifié ici est
l'implémentation de la plateforme (son propre fichier, même pas à côté de ceux
du moteur) ; `main.cpp`, `framebuffer.*`, `frame.*` et `arena.*` sont octet pour
octet l'état final de la leçon.

Une note honnête, la même que l'audit a consignée : le stub vit dans `tools/`
pour cet exercice parce que `build.sh` compile *chaque* source de `src/` — deux
implémentations y entreraient en collision à l'édition de liens, chacune
définissant les mêmes fonctions du contrat. La règle que l'audit consigne :
**une implémentation par build**. Si un portage veut les deux fichiers dans
l'arbre à la fois, la sélection de fichiers du build est la seule ligne au
courant — et ce n'est pas du code du moteur.

(Le stub se trouve aussi prouver une chose plus petite : la couture
n'entraîne avec elle aucun type d'OS. `struct Window` dans le stub est un type
différent, avec une définition différente de celle de X11, et rien à
l'extérieur ne s'en est soucié.)

*Page traduite de la version anglaise `book/solutions/lesson-042/ex1.md`,
révision `e4b35c3`.*

<!-- translation-source: book/solutions/lesson-042/ex1.md @ e4b35c3 -->
