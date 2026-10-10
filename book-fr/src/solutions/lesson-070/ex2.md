# Solution : exercice 2 — Les deux populations de la ligne

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Les deux populations de la ligne](../../lessons/part-3/lesson-070-audio-row.md) de la leçon 070.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-070/ex2.patch}}
```

## Visite guidée

Le patch garde les deux populations séparées là où la phase est mesurée : une
frame qui a alimenté un tampon a mixé un tampon entier ; une frame qui n'en a
alimenté aucun n'a rien mixé. Le cumul les compte et fait la moyenne de
chacune — et l'exécution imprime les deux à côté des comptes de la démo.

Une exécution non pilotée a exactement une population, comme l'attente cadencée
le prédit :

```
engine: demo: 467 frames measured, 467 buffers fed, 21 effects fired, 2 music wraps
engine: audio: 467 feeding frames avg 0.035 ms, 0 quiet frames avg 0.000 ms
```

L'attente réveille la boucle à l'horizon du tampon, donc chaque frame réveillée
a un tampon à alimenter : 467 frames, 467 alimentations. La seconde population
apparaît au moment où quelque chose d'autre réveille la boucle plus tôt — ici,
une entrée scriptée sur les flèches :

```
engine: demo: 609 frames measured, 584 buffers fed, 25 effects fired, 3 music wraps
engine: audio: 584 feeding frames avg 0.035 ms, 25 quiet frames avg 0.000 ms
engine: frame budget — 609 frames, avg 1.953 ms, worst 4.060 ms (frame 278)
engine:   audio        0.033       2%
```

Maintenant, réconciliez trois choses. **Les frames d'alimentation et le
travail :** un tampon fait 735 trames de sortie à travers tous les
`AUDIO_MIXER_CHANNELS` = 16 canaux — 11 760 tirages de canal — et 0.035 ms sur
tout ça, c'est environ 3 ns par tirage, le même chiffre que la sonde de la
leçon 068 a mesuré pour le mixage. **Les frames silencieuses :** 0.000 ms à la
précision imprimée — les deux lectures d'horloge autour d'un test qui dit
qu'aucun tampon n'est dû. **La ligne et les populations :** les 0.033 ms de la
table sont les deux populations pondérées par leurs tailles —
(584 × 0.035 + 25 × 0.000) / 609 = 0.034 ms à partir des moyennes imprimées,
0.0334 ms à partir des lignes de frame du journal, le 0.033 de la ligne. La
ligne et les populations ne peuvent pas être en désaccord : la ligne est
*uniquement* leur somme divisée par leur compte. Ce que le découpage ajoute,
c'est la raison — 96 % des frames portent tout le coût, donc la ligne est
essentiellement le nombre des frames d'alimentation.

La question du budget, depuis le journal de la même exécution : la pire frame
est la frame 278 à 4.060 ms, et son `audio` lit 0.220 ms — environ 5 % de cette
frame, contre les 2 % de part de la phase dans la frame moyenne. La part de la
phase dans la pire est plus grande parce que la pire frame est un hoquet de
frame entière (son render est haut lui aussi), et sur une frame qui se trouve
alimenter, le coût du mixage s'y ajoute. Ce qui ferait de cette ligne la
première à grandir : plus de canaux de mixeur, plus de trames par tampon, une
fréquence d'échantillonnage plus haute — chacun multiplie les tirages que le
mixage fait — ou un périphérique dont la soumission attend, ce qui atterrirait
dans cette ligne exactement comme la synchronisation de la présentation
atterrit dans `present`.

*Page traduite de la version anglaise `book/solutions/lesson-070/ex2.md`,
révision `cc7fcb3`.*

<!-- translation-source: book/solutions/lesson-070/ex2.md @ cc7fcb3 -->
