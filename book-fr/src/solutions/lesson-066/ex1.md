# Solution : exercice 1 — La couture, prédite

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La couture, prédite](../../lessons/part-3/lesson-066-music.md) de la leçon 066.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-066/ex1.patch}}
```

## Visite guidée

Le diff est une sonde : un mixeur d'essai qui joue la musique seule, alimenté
tampon par tampon à travers le `MixBuffer` du moteur lui-même, avec le curseur
du canal de la musique et le compteur de retournements imprimés autour de la
couture, et les trames que le mixage y contient. La sonde mixe les mêmes tampons
que l'exécution mixe — un appel à `MixBuffer` par tampon — et cesse simplement
de compter une fois la couture passée.

La prédiction, avant toute exécution. Les tampons font 735 trames et la boucle
132300 — exactement 180 tampons de 735, c'est pourquoi la couture tombe sur une
frontière de tampon plutôt qu'à l'intérieur. Après `b` tampons le curseur
affiche `b * 735` jusqu'à ce que cela atteigne `frame_count` : après le tampon
179, le curseur se pose sur 132300, la fin de l'échantillon. Le retournement
est **paresseux** — il survient au premier tirage au-delà de la fin, pas à
la fin elle-même — donc c'est le tampon 180 qui le porte : son premier
tirage ramène le curseur à 0 et prend la trame 0, et 735 tirages plus
tard le tampon laisse le curseur à 735. C'est le pas en arrière que l'exécution
guette, et le compteur de retournements passe à 1 là.

Les trames de la couture découlent de la même arithmétique, et le canal joue
seul à plein volume, donc le mixage **est** les trames de l'échantillon
lui-même. Le tampon 179 contient les trames 131565 à 132299 de l'échantillon —
ses quatre dernières sont les quatre dernières du fichier — et le tampon 180
contient les trames 0 à 734. Les premières trames du fichier sont ses propres
faits, dans la ligne de l'exécution : `0 277 554 831 …`, et sa dernière trame
est `0`. La couture est donc `… -1 -1 0 0 | 0 277 …` — la dernière trame
rejoint la première comme 0 rejoint 0, la seule jointure du fichier où rien ne
fait le moindre écart.

Le journal de l'exécution :

```
engine: seam: buffer 179: cursor 131565 -> 132300, wraps 0
engine: seam: buffer 179's last 4 frames: -1 -1 0 0
engine: seam: buffer 180: cursor 132300 -> 735, wraps 1
engine: seam: buffer 180's first 4 frames: 0 277 554 831
engine: seam: the sample's last 4 frames: -1 -1 0 0, and its first 4: 0 277 554 831
```

Chaque ligne se réconcilie avec l'arithmétique du curseur, et la dernière ligne
est la preuve que le mixage n'a eu besoin d'aucun traitement de couture
spécial : les quatre dernières trames du tampon 179 sont identiques octet pour
octet aux quatre dernières de l'échantillon, et les quatre premières du tampon
180 à ses quatre premières. Le retournement est une ligne d'arithmétique de
curseur à l'intérieur du tirage ; rien à l'extérieur du canal ne sait que
la frontière a existé.

Deux détails gagnent leur place ici.

**Le retournement est paresseux, et le curseur peut se poser sur la fin.**
Entre le dernier tirage du tampon 179 et le premier du tampon 180, le
curseur est égal à `frame_count` — le même état dans lequel se termine un canal
non bouclé. La différence n'est que ce que le tirage *suivant* en fait :
terminer, ou se retourner. Toute la leçon tient dans une valeur de curseur.

**« Frames played » est la somme des retournements et du curseur.** Le
compteur de retournements de la sonde dit combien de passes complètes la boucle
a faites, et le curseur dit où elle en est dans la courante : `wraps *
frame_count + cursor`. La ligne de retournement de la leçon elle-même — `wrap
1, 133035 trames played, cursor 735` — est exactement `1 * 132300 + 735`, et le
nombre de tampons confirme : 133035, c'est 181 tampons de 735.

Ce que la sonde ne peut pas dire, c'est ce que la couture sonne. Elle montre
les octets que la couture rencontre — `0` à `0` — et ceux-là sont vérifiables ;
l'écoute de la jointure d'une boucle est à vous, sur une machine qui fait du
son.

*Page traduite de la version anglaise `book/solutions/lesson-066/ex1.md`,
révision `70138ea`.*

<!-- translation-source: book/solutions/lesson-066/ex1.md @ 70138ea -->
