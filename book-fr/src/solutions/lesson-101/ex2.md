# Solution : exercice 2 — La pire frame, prédite

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La pire frame, prédite](../../lessons/part-5/lesson-101-frame-budget.md) de la leçon 101.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-101/ex2.patch}}
```

## Visite guidée

**La prédiction, écrite d'abord.** La pire frame d'une exécution jouée
est une frame *chargée*, et le budget de la frame de jeu est dominé par
les deux lignes que le jeu dépense partout : le dessin de la carte et
le clear, avec l'attente de la couture qui suit tout ce qui se passe.
Donc la prédiction : la pire frame est une frame de jeu dont le `render`
prend la part du lion — la tilemap avant tout — avec `present` à peu près
doublé par rapport à sa moyenne (la couture pique quand elle pique), et
l'update visiblement plus gras que son `0.013 ms` (la frame d'apparition
d'une vague crée son effectif en une frame). L'hypothèse alternative — la
pire frame *est* celle de la couture — est l'intéressante à garder en
réserve.

**Le compte se souvient de la forme.** L'enregistrement entier de la pire
frame est gardé à côté de son coût (`stats.worst_record`), et le rapport
lui donne la même attribution que la table donne à la moyenne. De
l'exécution de dix manches de l'exercice lui-même, sur le jeu terminé
(2 583 frames) :

```
engine: frame budget — 2583 frames, avg 1.239 ms, worst 2.821 ms (frame 519)
engine:   budget      60 fps is 16.667 ms a frame — 0 of 2583 frames over it, worst 2.821 ms (17% of it)
engine:   worst was   2.821 ms (frame 519): update 0.110 (entities 0.092), audio 0.000, render 1.852 (clear 0.575, sprites 0.010, text 0.014, tilemap 1.253), present 0.858
```

**Le verdict contre la prédiction :** juste dans la forme, meilleur que
prévu dans le détail. `render 1.852` est la part du lion avec
`tilemap 1.253` au sommet — et `clear 0.575` au double de *sa* moyenne
aussi (un tampon froid sur cette frame). `present 0.858` fait bien
environ le double de sa moyenne de `0.438 ms` — la couture pique quand
elle pique. Et `update 0.110` avec `entities 0.092` fait sept fois la
moyenne de l'update — la frame d'apparition d'une vague, créant son
effectif en une frame, l'indice de la prédiction. La frame fait
2.821 ms — 17 % du budget — et la queue, c'est *le jeu qui est chargé*,
pas un sous-système qui se comporte mal.

**Le contrefactuel.** Si la pire frame était celle de la couture —
`present` dominant, les lignes propres du moteur à leurs moyennes — la
conclusion honnête serait que le pire cas appartient à la couche
plateforme : le menu figé ne peut pas le raccourcir (la copie de la
couture est le fichier du second OS, au registre du travail futur), et le
correctif devrait arriver sous la forme d'un present à double tampon ou
MIT-SHM avant que les nombres propres du moteur ne comptent à nouveau.
Dans cette exécution c'est l'inverse : le coût de la pire frame se
répartit sur le travail de dessin du moteur lui-même, ce qui veut dire
que c'est de la charge ordinaire — celle qui baisse exactement avec les
leviers que ce menu a déjà dépensés. C'est la différence entre « notre
queue » et « la queue de notre machine », et la ligne de la pire frame
est ce qui les distingue.

*Page traduite de la version anglaise `book/solutions/lesson-101/ex2.md`,
révision `ded4020`.*

<!-- translation-source: book/solutions/lesson-101/ex2.md @ ded4020 -->
