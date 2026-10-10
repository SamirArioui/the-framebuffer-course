# Solution : exercice 1 — Ce que le profil ne peut pas voir

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Ce que le profil ne peut pas voir](../../lessons/part-5/lesson-098-measure.md) de la leçon 098.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-098/ex1.patch}}
```

## Visite guidée

La sonde est une ligne et une somme : `ReportSeam` (dans `report.*`, à côté des
autres rapports de l'exécution, appelé juste avant la table du budget de
frames) scinde le temps mural de la frame là où la couture scinde le travail —
les phases du moteur (`update + audio + render`, ce que le processus a calculé)
contre le present (ce que le processus a passé à attendre que l'affichage
prenne ses pixels). D'une vraie exécution du build patché :

```
engine: seam: 777 frames — engine work 1.323 ms/frame (update+audio+render), seam wait 0.425 ms/frame (present) — the flat profile sees the first, not the second
engine: frame budget — 777 frames, avg 1.748 ms, worst 2.994 ms (frame 489)
```

Les deux compartiments s'additionnent exactement — `1.323 + 0.425 = 1.748`, le
total de la table elle-même — parce que la frame n'a pas de troisième endroit
où le temps puisse aller.

Maintenant les réponses, avec les nombres de cette exécution. **Le profileur
est aveugle à `0.425 ms` par frame — 24 % du temps mural de la frame** —
l'attente de la couture. Et les pourcentages du profil plat décrivent le **CPU,
pas la frame** : le `59.77%` de `BlitSprite` est 59,77 % du temps du processus
profilé (3,53 s sur l'exécution), alors qu'en part du temps mural de la frame
le dessin de la carte fait `0.981 / 1.944 ≈ 50 %` d'une frame de jeu. Les deux
vues s'accordent sur les noms et l'ordre mais pas sur l'arithmétique, et la
raison est exactement ce découpage : `platform::Present` lit `0.28%` de CPU et
`23%` de la frame.

La règle pratique, à garder à côté de chaque profil que vous prendrez : **les
parts d'un profil sont des parts du travail du processus ; le coût d'une frame
est le travail du processus plus ses attentes.** Quand la couture est bon
marché (un affichage local, une copie rapide), les deux convergent ; quand elle
est chère (l'aller-retour au serveur X de cette machine), un quart de la frame
est simplement hors de l'image que le profileur peint. C'est pourquoi la leçon
nomme les points chauds depuis les deux instruments — et pourquoi `present` est
allé sur la liste des travaux futurs mesuré, pas deviné.

*Page traduite de la version anglaise `book/solutions/lesson-098/ex1.md`,
révision `17073ed`.*

<!-- translation-source: book/solutions/lesson-098/ex1.md @ 17073ed -->
