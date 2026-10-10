# Solution : exercice 1 — Le compteur d'appuis

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le compteur d'appuis](../../lessons/part-1/lesson-033-latching.md) de la leçon 033.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-033/ex1.patch}}
```

## Visite guidée

La mémorisation répond à « s'est-elle enfoncée ? » — *au moins une fois*. Deux
frappes dans un même lot l'allument exactement comme une seule, et tout ce qui
compte les frappes (un double saut, une répétition de menu, un rythme de frappe)
ne peut pas faire la différence. La forme générale est un **compteur** au lieu
d'un drapeau : `presses[key]` est incrémenté à chaque front (relâchée →
enfoncée) et remis à zéro à la lecture par `KeyPressCount`. `KeyPressed` devient
ce qu'il a toujours été — `count > 0` — donc les anciens sites d'appel veulent
toujours dire la même chose, et le rapport du moteur montre désormais la
différence :

```
$ DISPLAY=:99 xdotool key --delay 0 --repeat 2 --window <id> space
engine: polled: -
engine: pressed space x2
engine: polled: -
```

Deux frappes, un lot, une scrutation — et le compteur dit deux. (Remarquez la
même situation d'un seul lot où l'exercice 2 de la leçon 032 montrait l'appui
perdu entièrement : l'*état* est toujours `-`.)

La mémorisation s'allume toujours sur le **front**, pas à chaque événement
d'appui — une touche maintenue en auto-repeat, c'est un appui et un incrément.
Si vous voulez les événements de répétition bruts (pour la saisie de texte,
disons), c'est une troisième sorte d'état avec son propre contrat : les
répétitions ne sont pas des appuis et les appuis ne sont pas des caractères.

*Page traduite de la version anglaise `book/solutions/lesson-033/ex1.md`, révision `3cf02bf`.*

<!-- translation-source: book/solutions/lesson-033/ex1.md @ 3cf02bf -->
