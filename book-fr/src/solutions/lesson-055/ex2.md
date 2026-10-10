# Solution : exercice 2 — Le rectangle à la frontière

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le rectangle à la frontière](../../lessons/part-2/lesson-055-collision.md) de la leçon 055.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-055/ex2.patch}}
```

## Visite guidée

Le pilier occupe les cellules (8,6) et (9,7) ainsi que leurs voisines — les
pixels monde x = 128..159, y = 96..127. Un rectangle est *semi-ouvert* : il
couvre `x .. x+w−1`, `y .. y+h−1` — le bord gauche et le bord haut inclus, le
bord droit et le bord bas exclus. Cette seule décision est l'arithmétique
derrière les six prédictions :

| Rectangle | Couvre | Cellules | Réponse |
| --------- | ------ | -------- | ------- |
| (128, 96, 16, 16) | x 128..159, y 96..127 | (8,6) seule | **solide** — exactement le pilier |
| (160, 96, 16, 16) | x 160..175 | (10,6) | **libre** — à côté, sans le toucher |
| (159, 96, 1, 16) | x 159 seulement | (9,6) | **solide** — un pixel *à l'intérieur* du pilier |
| (160, 128, 16, 16) | x 160..175, y 128..143 | (10,8) | **libre** — en diagonale, sans le toucher |
| (159, 127, 1, 1) | le pixel de coin du pilier | (9,7) | **solide** |
| (160, 128, 1, 1) | le pixel au-delà du coin | (10,8) | **libre** |

L'exécution confirme chacune :

```
engine: boundary check: 6 of 6 edges behave as documented
```

Maintenant la dernière question de l'énoncé — quel caractère unique décide des
cas à un pixel. C'est le **`− 1`** dans la conversion en cellules :

```c++
    int cx1 = (x + w - 1) / TILE_SIZE;
    int cy1 = (y + h - 1) / TILE_SIZE;
```

`x + w` convertirait le bord *exclusif* du rectangle — le premier pixel qu'il
ne couvre **pas** — comme s'il était couvert. Le rectangle d'un pixel
`(159, 96, 1, 16)` : avec le `− 1`, son bord droit vaut `159 + 1 − 1 = 159` →
cellule 9 → pilier → solide. Sans lui, le bord droit calcule `160` → cellule
10 — et la vérification interrogerait deux cellules, dont l'une que le
rectangle ne touche jamais. Dans les directions libres, le bug se cache
(demander une cellule de plus ne fait que rendre les collisions *plus*
probables), et dans les directions serrées, il rapporte faux à chaque
frontière. C'est l'erreur de décalage (off-by-one) qui sort dans les jeux sous
la forme « tu n'arrives pas tout à fait à toucher ce mur » ou « tu entres en
collision avec le mur à côté de la porte » — et c'est pourquoi la table
ci-dessus existe.

L'habitude : quand une vérification peut se poser exactement sur une
frontière, posez-la là. Les cas intérieurs passent sous toutes les
implémentations ; seuls les cas à un pixel distinguent un rectangle correct
d'un rectangle presque correct — et une collision « presque correcte » est le
genre de bug que les joueurs ressentent bien avant que les tests ne le
trouvent.

*Page traduite de la version anglaise `book/solutions/lesson-055/ex2.md`,
révision `c39d430`.*

<!-- translation-source: book/solutions/lesson-055/ex2.md @ c39d430 -->
