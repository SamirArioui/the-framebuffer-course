# Solution : exercice 2 — Ce que le contrôle ne peut pas voir

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Ce que le contrôle ne peut pas voir](../../lessons/part-1/lesson-042-contract.md) de la leçon 042.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-042/ex2.patch}}
```

## Visite guidée

Un autre **état pédagogique délibéré** : ce patch franchit la frontière exprès,
et s'en tire à bon compte. `std::system("true")` est l'OS — exécution de
processus, shell compris — appelé depuis du code du moteur, un seul appel,
aucun en-tête d'OS requis.

```
$ ./tools/check-boundary.sh
boundary: engine code must not name an OS
boundary: OK — OS headers and OS calls appear only in src/platform_x11.cpp
boundary: the contract a second OS implements is declared in src/platform.h
boundary: 14 single-line declarations there (multi-line ones are in the header)
$ echo $?
0
$ DISPLAY=:99 ./build/game '#leak'
engine: calling the OS behind the seam's back
```

Le contrôle a validé un arbre contenant un franchissement bien réel. Ce n'est
pas un bug du contrôle — c'est le contrôle qui est ce qu'un contrôle est :
**une liste d'erreurs auxquelles quelqu'un a pensé**. Ses deux listes (en-têtes
d'OS, appels d'OS) attrapent les fuites ordinaires — un `XOpenDisplay` par-ci,
un `<unistd.h>` par-là — et n'attrapent rien hors de la liste. `system` n'y
figure pas. Pas plus que `syscall`, l'assembleur inline, une fonction de la
libc qui fait discrètement le travail de l'OS, ou une macro qui se développe en
l'un d'eux.

Quelle classe de fuites aucun grep ne peut-il attraper ? Tout ce qui atteint
l'OS par *indirection* : un pointeur de fonction rempli on ne sait où, un appel
de bibliothèque dont l'implémentation parle au noyau (comme `printf` le fait
déjà), un template dans l'outillage qui génère un appel d'OS. La frontière est
une propriété du *sens*, et le sens n'est pas de la syntaxe.

C'est exactement pourquoi les conventions disent ce qu'elles disent : le
sous-ensemble C++ est **imposé par la revue, pas par l'outillage**, et la
frontière est imposée de la même façon — le contrôle est le premier filtre,
bon marché, exécuté à chaque fois, qui attrape les faux pas ; la revue est ce
qui attrape le reste. Un outil qui passe n'est pas une preuve ; c'est une chose
de moins que le relecteur doit chercher.

La rédaction à viser : nommez votre fuite la plus sournoise (écrivez-la, faites
passer le contrôle dessus), puis décrivez ce qu'un relecteur doit comprendre
pour l'attraper et que le contrôle ne pourra jamais comprendre.

*Page traduite de la version anglaise `book/solutions/lesson-042/ex2.md`,
révision `e4b35c3`.*

<!-- translation-source: book/solutions/lesson-042/ex2.md @ e4b35c3 -->
