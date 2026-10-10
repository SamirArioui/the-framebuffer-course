# Solution : exercice 2 — La politique qui refuse

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La politique qui refuse](../../lessons/part-3/lesson-065-allocation.md) de la leçon 065.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-065/ex2.patch}}
```

## Visite guidée

Le diff fait exister la politique rejetée. `MixerTryPlay` dans `src/audio.cpp`
parcourt le pool exactement comme `MixerPlay` — le premier canal d'effet libre,
le même `ChannelPlay`, la même estampille `started` — et s'arrête une étape plus
tôt : un pool plein renvoie `AUDIO_NO_CHANNEL` au lieu d'aller chercher le plus
ancien. `MixerPlay` lui-même est intact, donc les deux politiques côte à côte
peuvent être pilotées dans une même exécution. La démo dans `src/main.cpp` s'en
charge : la salve de seize effets de la leçon elle-même, inchangée, et la même
salve de nouveau sur son propre mixeur à travers `MixerTryPlay`.

Le journal de l'exécution, les deux politiques dedans :

```
engine: mix: effect 15 -> channel 15
engine: mix: effect 16 -> channel  1 (the oldest was stolen)
engine: mix: music channel 0 is reserved and was never stolen
engine: refuse: effect  1 -> channel  1
engine: refuse: effect  2 -> channel  2
...
engine: refuse: effect 15 -> channel 15
engine: refuse: effect 16 -> refused (every effect channel busy)
```

C'est au seizième appui que les politiques se séparent, et le rapport tient en
deux phrases. Sous le vol, le seizième son joue — il prend le canal 1 et l'effet
1 s'arrête à la trame où il était arrivé. Sous le refus, le seizième son n'a pas
lieu du tout — le pool répond nommément et les effets 1 à 15 continuent de
jouer jusqu'à leur fin. Les deux réponses sont légales ; elles diffèrent par
*qui paie* pour la seconde chargée. Le vol fait payer le son le plus récent avec
le plus ancien ; le refus fait payer le joueur, sans rien couper.

Rien d'autre dans l'exécution ne bouge : le rapport de fin reste 30 tampons de
son — la salve qui refuse parle à son propre mixeur et n'est jamais mixée dans
le flux — et le canal de musique est aussi réservé sous une politique que sous
l'autre.

Quelle politique livrer est la vraie question de l'exercice, et la réponse de la
leçon tient bon : pour les effets sonores d'un jeu, voler. Un bouton pressé doit
répondre par un son, la seconde chargée est exactement le moment où le retour
compte, et le son qui cède est celui déjà le plus entendu. Le refus n'est pas un
non-sens — c'est la bonne politique là où chaque son compte et où l'entendre
entier vaut mieux que l'entendre tout court : un signal que le joueur doit
attraper vaut mieux manqué que coupé, et un signal manqué peut être redemandé.
C'est le compromis que `MixerTryPlay` chiffre : il ne coupe jamais rien, et
c'est le seul des deux qui puisse répondre à un appui par le silence.

Ce que la sonde ne peut pas cacher sur elle-même : aucun haut-parleur n'a émis
de son — `null` jette les échantillons — donc « ce que le joueur vit » est
argumenté, pas entendu. Et les deux parcours sont des copies l'un de l'autre,
pas un seul parcours avec un commutateur de politique ; l'exercice demandait la
politique à côté de `MixerPlay`, et laisser la forme partagée implicite est ce
qui garde la comparaison honnête.

*Page traduite de la version anglaise `book/solutions/lesson-065/ex2.md`,
révision `74a32e1`.*

<!-- translation-source: book/solutions/lesson-065/ex2.md @ 74a32e1 -->
