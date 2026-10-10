# Solution : exercice 2 — La page qui riposte

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La page qui riposte](../../lessons/part-1/lesson-040-reservations.md) de la leçon 040.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-040/ex2.patch}}
```

## Visite guidée

C'est un **état pédagogique délibéré** — le patch existe pour planter, une
fois, exprès, afin que le refus de la machine soit regardé au lieu d'être
imaginé. Il ajoute une fonction à la couture (`MakeInaccessible` — `mprotect`
avec `PROT_NONE` derrière l'interface) et une réservation sous garde, touchée
juste après.

```
$ DISPLAY=:99 ./build/game '!crash'
engine: touching a page with no permissions
Segmentation fault (core dumped)
$ echo $?
139
```

Lisez ce qui s'est passé au niveau machine. La réservation mappait une page en
lecture/écriture ; `MakeInaccessible` a demandé à l'OS de changer les entrées
de la table de pages de cette page vers *aucune* permission — `PROT_NONE`. Rien
dans le langage ne sait que c'est arrivé : `guarded.bytes[0]` est une lecture
d'unsigned char ordinaire, compilée en un `mov` ordinaire. Quand cette
instruction s'est exécutée, la traduction d'adresse du CPU a trouvé l'entrée de
la page, a vu que les bits de permission interdisaient la lecture, et a
**arrêté l'instruction**. L'OS a reçu le défaut, l'a examiné, et a délivré
`SIGSEGV` au processus — dont l'action par défaut est la mort que vous venez de
regarder.

Chaque couche a son mot à dire ici, et aucune ne pouvait sauver la lecture :
le C++ n'a aucune vérification pour cela, le compilateur ne peut pas le voir,
et aucune exception ne la porte. La leçon 006 montrait du comportement indéfini
qui *parfois* plante ; ici, c'est la machine elle-même qui refuse, de façon
déterministe, à chaque fois. Voilà ce que « protection » veut dire dans les
tables de pages de la leçon 039 : des bits, vérifiés par le matériel, à chaque
accès.

(Un détail à remarquer dans le patch : les impressions de la sonde vont sur
`stderr`. L'exécution meurt en pleine expression — le tampon de `stdout`
mourrait avec elle. L'enseignement de la leçon 001 sur l'ordre des flux, qui
prouve son utilité dans un crash.)

Quelle serait la correction, si ce n'était pas délibéré : ne jamais distribuer
de pages dont les permissions ne correspondent pas à leur usage. L'API de
réservation donne des pages en lecture/écriture parce que c'est ce dont les
tampons ont besoin — et c'est pourquoi l'accès que le moteur ne fait jamais est
celui que l'OS doit interdire au niveau de la table de pages quand cela compte
(les pages de garde de pile fonctionnent exactement ainsi).

*Page traduite de la version anglaise `book/solutions/lesson-040/ex2.md`,
révision `e9826f2`.*

<!-- translation-source: book/solutions/lesson-040/ex2.md @ e9826f2 -->
