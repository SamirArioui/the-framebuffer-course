# Solution : exercice 2 — Pause et reprise

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Pause et reprise](../../lessons/part-3/lesson-066-music.md) de la leçon 066.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-066/ex2.patch}}
```

## Visite guidée

Le diff rend la pause réelle à côté de `MixerStop`. `MixerPause` suspend le
canal là où il est — inactif, pour que le mixage cesse de en tirer —
et garde tout : l'échantillon, le curseur, le volume et le drapeau de boucle.
Rien n'est jeté et rien n'est rembobiné. `MixerResume` rend le canal actif de
nouveau et le tirage suivant prend la trame sur laquelle la pause s'est
arrêtée. `MixerStop` est intact ; les deux diffèrent par ce qu'ils veulent
dire, pas par ce qu'ils écrivent.

La sonde dans `src/main.cpp` fait tourner les deux sur un mixeur d'essai : la
musique joue 300 tampons, se met en pause pendant 100, reprend pendant 50, et
le curseur est lu à chaque étape. La seconde moitié est le danger dont la
question en prose parle : un effet est démarré, son canal est mis en pause, et
un second effet demande un canal.

La prédiction, avant toute exécution. 300 tampons font 220500 trames — une
passe complète de la boucle de 132300 trames et 88200 trames dans la seconde —
donc le curseur à la pause est 88200. Pendant la pause, rien ne prélève, donc
le curseur y reste exactement, et les 100 tampons en pause sont du silence : le
canal de la musique était le seul son et le mixage n'a rien à sommer. Après la
reprise, 50 tampons de plus font 36750 trames, donc le curseur affiche 124950 —
la pause n'a coûté au son que du temps. Puis le danger : le canal de l'effet en
pause rapporte `active == false`, et le parcours du premier canal libre de
`MixerPlay` lit exactement ce drapeau.

Le journal de l'exécution :

```
engine: pause: cursor 88200 at the pause, 88200 while paused, 124950 after 50 more buffers; the paused buffers were silence
engine: pause: the paused effect's channel 1 looked free — the next effect took channel 1
```

Chaque curseur se réconcilie : 300 × 735 = 220500 et 220500 − 132300 = 88200 ;
88200 + 50 × 735 = 124950. Et la question en prose a sa réponse dans la seconde
ligne. **Le parcours du premier canal libre donne un canal d'effet en pause.**
Il pose une seule question — ce canal est-il actif — et un canal en pause
répond exactement comme un canal libre. Pire, le nouveau son passe par
`ChannelPlay`, qui démarre à la première trame de l'échantillon : le son en
pause n'est pas repris plus tard, il a disparu. Ce n'est pas ce qu'un jeu veut
d'un bouton pause.

Ce qu'un jeu veut est un état que l'allocateur peut distinguer : un drapeau de
pause à côté de `active`, le parcours sautant les canaux en pause et le vol
refusant d'y toucher. Le canal de la musique n'a jamais le problème — le
parcours commence après lui — c'est pourquoi mettre la musique en pause ici est
sûr et mettre un effet en pause ne l'est pas. La version de l'exercice est
honnête sur la différence : `MixerPause` est juste pour le canal de la musique
et fausse pour un effet du pool, et la seconde ligne du journal est l'erreur,
mesurée.

Ce que rien de tout cela ne vérifie, c'est l'écoute : les tampons en pause
contiennent zéro octet et la reprise continue l'arithmétique du curseur, deux
choses vérifiables ici ; ce que sonne une pause de la musique est à vous
d'entendre.

*Page traduite de la version anglaise `book/solutions/lesson-066/ex2.md`,
révision `70138ea`.*

<!-- translation-source: book/solutions/lesson-066/ex2.md @ 70138ea -->
