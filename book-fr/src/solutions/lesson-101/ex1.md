# Solution : exercice 1 — La ligne des 60 fps sur votre machine

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La ligne des 60 fps sur votre machine](../../lessons/part-5/lesson-101-frame-budget.md) de la leçon 101.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-101/ex1.patch}}
```

## Visite guidée

Le compte grandit d'un histogramme — 65 longs de sa propre mémoire, des
seaux de `0.25 ms` jusqu'au voisinage du budget, le dernier seau une
poubelle de débordement — et `AccountFrame` range chaque frame dans son
seau au fur et à mesure. Les percentiles s'y lisent en marchant sur le
compte cumulé : le seau où siège la frame médiane, où siège le 95e, où
siège le 99e. Aucune allocation (l'habitude de la loi du langage), aucun
tri, aucune frame retenue.

La ligne de cette machine, d'une vraie exécution de six manches du jeu
terminé (1 551 frames) :

```
engine:   by state    1415 play frames at 1.256 ms, 136 screen frames at 0.718 ms
engine:   budget      60 fps is 16.667 ms a frame — 0 of 1551 frames over it, worst 2.659 ms (16% of it)
engine:   percentiles p50 1.250 ms, p95 1.750 ms, p99 2.000 ms (0.25 ms buckets)
engine:   machine     WSL2, Xvfb :99, no sound hardware (the course's authoring machine)
```

Lisez la queue, pas la moyenne : la frame *médiane* est `1.250 ms`, le
pire un pour cent des frames reste sous `2.000 ms`, contre un budget de
`16.667 ms`. La moyenne (`1.225–1.3 ms`) siégeait entre la médiane et le
p95 — ce qui est exactement pourquoi la moyenne n'est pas l'instrument
d'une affirmation de fréquence de frames : **60 fps est une affirmation
sur la queue.** Une machine qui fait 2 ms en moyenne avec un p99 de 40 ms
laisse tomber des frames visiblement ; une machine qui fait 4 ms en
moyenne avec un p99 de 6 jamais.

Maintenant le portage — et ce que votre rapport doit contenir pour
répondre honnêtement à la ligne de la checklist :

1. **Le nom de votre machine là où est le nôtre** — la chaîne
   `RUN_MACHINE` dans `main.cpp` est à vous d'éditer. Un nombre qui a
   perdu sa machine est une rumeur.
2. **La forme de votre exécution, nommée** — une fenêtre sur votre bureau
   change structurellement deux de nos nombres : la ligne `present` (un
   affichage local coûte du CPU *au processus* là où notre serveur X le
   facturait comme attente) et la *fréquence* des frames (avec un
   périphérique sonore, l'horizon audio cadence votre boucle à ~60
   alimentations par seconde au lieu des 25 de nos secousses).
3. **Les percentiles à côté de la ligne de budget** — et la phrase dont
   la ligne de perf du MVD a besoin : *« ma machine tient 60 fps »*
   seulement si la ligne de budget dit `0 of N frames over it` **et** que
   le p99 siège sous le budget avec la marge qu'exigent les ambitions de
   votre jeu. Si des frames manquent, comptez-les, nommez la phase qui
   les a mangées (l'attribution de l'exercice 2), et dites honnêtement
   laquelle des deux c'est : du travail dont le moteur peut se délester,
   ou de l'attente que la couture possède.

Quand votre carte dira `0 of N over` sur du vrai matériel à de vraies
fréquences de frames, la ligne de perf de la checklist sera vérifiée
**jusqu'au bout** — et cela allait toujours être votre mesure à prendre,
pas celle de cette machine (D12).

*Page traduite de la version anglaise `book/solutions/lesson-101/ex1.md`,
révision `ded4020`.*

<!-- translation-source: book/solutions/lesson-101/ex1.md @ ded4020 -->
