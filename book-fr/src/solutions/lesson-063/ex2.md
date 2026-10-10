# Solution : exercice 2 — Le fondu vers le silence

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le fondu vers le silence](../../lessons/part-3/lesson-063-channel.md) de la leçon 063.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-063/ex2.patch}}
```

## Visite guidée

Le diff fait deux choses : le canal démarré à pleine échelle
(`AUDIO_VOLUME_FULL`), et le fondu — après chaque remplissage, le volume baisse
de 16 sur 256, jamais sous zéro, l'exécution nommant chaque nouveau volume.

La prédiction, avant toute exécution. La baisse a lieu après le remplissage,
donc le tampon du remplissage n est au volume 256 − 16(n−1) :

| remplissage | volume | le tampon |
| --- | --- | --- |
| 1 | 256 | l'échantillon à pleine échelle |
| 2 | 240 | chaque trame × 240 / 256, tronquée |
| … | … | seize pas vers le bas |
| 16 | 16 | un murmure — chaque trame fait 1/16 de l'échantillon |
| 17–30 | 0 | du silence — le canal joue encore |
| 31 | 0 | du silence, et le canal inactif dès la première trame du remplissage |

Le son s'épuise donc au remplissage 16, et la *lecture* s'épuise à la dernière
trame du remplissage 30 : les remplissages 17 à 30 sont du silence pur avec le
curseur qui avance encore — le volume ne participe pas à la condition du
remplissage, donc le canal continue de dépenser l'échantillon à pleine vitesse
tout en émettant des zéros. Le rapport de fin est inchangé : `22050 frames fed
in 30 buffers`, car ce qu'il compte, c'est la dépense du curseur, pas ce qu'un
auditeur entend.

Le journal de l'exécution, vu par les lignes du fondu lui-même :

```
engine: channel: playing 22050 frames at volume 256 of 256
engine: fade: volume 240 of 256
engine: fade: volume 224 of 256
...
engine: fade: volume 16 of 256
engine: fade: volume 0 of 256
engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
```

Seize lignes de fondu, de 240 jusqu'à 0, et la dix-septième baisse ne vient
jamais. Chaque ligne nomme le volume auquel est le tampon du remplissage
*suivant*, donc la dernière ligne ci-dessus suit le remplissage qui a tourné à
16 sur 256. Les trois nombres qui réconcilient tout : 16 remplissages ont porté
du son (11760 trames), 14 remplissages ont été du silence venant d'un canal qui
joue (10290 trames), et 11760 + 10290 = 22050 = `frame_count`. La fin est
arrivée où elle arrive toujours — le remplissage 30 dépense la dernière trame de
l'échantillon, le remplissage 31 s'en aperçoit — et le fondu n'y a jamais
touché.

Ce qui répond à la question du comptage. Une fois que les trames qu'il alimente
sont du silence, `frames fed` compte des **trames d'échantillons consommées** —
22050 d'entre elles, toutes dépensées par le curseur, seules les 11760 premières
audibles. Le compte ne bouge pas avec le volume, et c'est la preuve que ce sont
bien les trames de l'échantillon qui sont comptées, et pas le son.

La distinction qui est vraiment l'enjeu de l'exercice : `active` est une
position de lecture, pas un niveau sonore. Un canal au volume 0 est silencieux
et joue toujours — son curseur parcourt l'échantillon et sa fin arrive à l'heure
dite. Seul `frame_count` met fin à un canal, et seul un canal terminé est de
nouveau libre — le fait sur lequel s'appuie le pool de la leçon 065.

Une note d'honnêteté, gardée là où ce cours la garde : aucun haut-parleur de
cette machine n'a émis de son. Les seize pas du fondu sur 16 remplissages — 267
ms au rythme de l'horizon de 16,7 ms — sont vérifiés dans les trames et le
journal ; ce que donne à entendre une tonalité qui glisse de la pleine échelle
au silence en un quart de seconde est une question pour du vrai matériel.

*Page traduite de la version anglaise `book/solutions/lesson-063/ex2.md`,
révision `95a98ec`.*

<!-- translation-source: book/solutions/lesson-063/ex2.md @ 95a98ec -->
