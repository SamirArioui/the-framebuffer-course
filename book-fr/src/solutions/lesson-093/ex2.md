# Solution : exercice 2 — La courbe, prédite

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La courbe, prédite](../../lessons/part-5/lesson-093-bursts-easing.md) de la leçon 093.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-093/ex2.patch}}
```

## Visite guidée

**La prédiction, écrite d'abord.** `EaseOutQuad` est la courbe de pose :
`1 − (1 − t)²` du trajet à la fraction `t` du temps — et ses extrémités sont
bornées, si bien que la valeur se tient à sa cible dès que `t` atteint 1. Pour la
ligne d'étincelle livrée (`accel 400`, `range 64`) :

- **À la moitié de la pose** (`t = 0.5`) : la fraction est `1 − 0.5² = 0.75` —
  la courbe ease-out est aux trois quarts du chemin à la moitié du temps.
  L'étincelle est donc à `0.75 × 64 = 48 px`. Pas à 32 : c'est toute la
  différence entre une courbe et une droite.
- **À la fin** (`t = 1`) : exactement `64 px` — la `range` de la ligne. Le
  bornage à 1 fait de la dernière frame la cible elle-même, pas une valeur qui
  s'en est simplement beaucoup approchée.

**L'exécution** — la sonde imprimant la valeur avec easing à chaque frame sur 17
chiffres — dit exactement cela :

```
engine: probe: spark traveled 4.582819037655689 at t 0.036
engine: probe: spark traveled 25.74903114634359 at t 0.227
engine: probe: spark traveled 48.857177534509155 at t 0.514
engine: probe: spark traveled 59.596867977757853 at t 0.738
engine: probe: spark traveled 63.646876510750438 at t 0.926
engine: probe: spark traveled 63.999110491731322 at t 0.996
engine: probe: spark traveled 64 at t 1.000
engine: spark settled at 384,232 — 64 px out, its row's range 64 (exact)
```

La frame la plus proche de la moitié lit `48.86 px` à `t 0.514` — les `48`
prédits à `t 0.5`, une frame d'easing plus tard (les frames de l'exécution font
~43 ms d'une pose de 400 ms, si bien que `t` tombe à `0.1` d'écart ; le `t`
imprimé est arrondi à trois chiffres et la valeur ne l'est pas). Chaque
intermédiaire se tient là où `64 × (1 − (1 − t)²)` le met. Et les dernières
frames répondent à la question sur laquelle tourne l'effet :
`63.999110491731322 at t 0.996` — la valeur qui *s'approche* — puis `64 at
t 1.000`, imprimé à dix-sept chiffres significatifs comme **`64`** : pas
`63.99999999999999`, pas à une largeur d'arrondi près. La dernière frame atterrit
sur la cible, et la comparaison propre de l'exécution dit `exact`.

Pourquoi cela compte au-delà de l'arithmétique : une valeur qui ne fait que
s'approcher de sa cible (l'easing de l'`accel` du héros, l'approche géométrique
de la leçon 085) est juste pour le *ressenti* — du poids, de l'élan, jamais tout
à fait l'arrivée. Une valeur qui doit *être* quelque part — un compteur qui
montre le vrai score, un fondu qui se termine grand ouvert, une étincelle là où
l'impact l'a mise — a besoin du bornage. La boîte à outils livre les deux
vocabulaires exprès : l'ensemble d'easing pour les arrivées, et l'approche
géométrique pour le poids. Choisir lequel une valeur demande fait partie du design du
ressenti, et le moteur a maintenant des noms pour les deux.

*Page traduite de la version anglaise `book/solutions/lesson-093/ex2.md`,
révision `3c9d3c2`.*

<!-- translation-source: book/solutions/lesson-093/ex2.md @ 3c9d3c2 -->
