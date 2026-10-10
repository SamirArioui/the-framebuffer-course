# Solution : exercice 2 — Le pas qui ne rentre pas dans la ligne

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le pas qui ne rentre pas dans la ligne](../../lessons/part-2/lesson-047-caches.md) de la leçon 047.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-047/ex2.patch}}
```

## Visite guidée

D'abord la prédiction, car c'est là que vit réellement l'exercice. Un pas de
60 face à des lignes de 64 octets : les touchers tombent aux octets 0, 60,
120, 180, 240 … et les lignes couvrent 0–63, 64–127, 128–191 … Le premier
toucher (0) et le deuxième (60) partagent la ligne 0 ; le troisième (120)
est dans la ligne 1 ; le quatrième (180) dans la ligne 2. Certains touchers
réutilisent la ligne chargée par le précédent, d'autres en chargent une
nouvelle — et sur toute longue série, l'arithmétique se stabilise :
`size / 60` touchers répartis sur `size / 64` lignes font **1,07 toucher par
ligne**. Un pas de 64, c'est exactement 1. La prédiction est donc : le pas
de 60 atterrit *sur* le pas de 64 — les deux utilisent environ un octet utile
par ligne chargée, et aucun n'approche les 64 du séquentiel.

L'exécution en `-O3` :

```
engine: cache probe — copy walk, useful GB/s per stride
engine: working set   sequential    stride 64    stride 60
engine:       4 KB        108.6          3.3          3.0
engine:      64 KB         57.3          1.2          1.3
engine:    512 KB         45.5          1.0          1.1
engine:   4096 KB         14.0          0.4          0.4
engine:   8192 KB         16.7          0.3          0.4
engine:  12288 KB         15.3          0.3          0.3
```

Les pas de 60 et de 64 sont indiscernables à chaque ensemble de travail —
parfois un cheveu plus lent (4 Ko : 3,0 contre 3,3), parfois un cheveu plus
rapide (64 Ko : 1,3 contre 1,2), toujours à l'intérieur du plancher de
bruit. La prédiction tient.

La question que posent les nombres — valeur du pas ou octets utiles par
ligne ? — a donc sa réponse dans le tableau. Si la *valeur du pas* était le
coût, 60 se situerait entre 64 et le séquentiel, plus près du milieu. Ce
n'est pas le cas ; elle se pose sur 64. Ce que les deux pas partagent, c'est
le même gaspillage : environ un octet utile par ligne de 64 octets chargée.
Le coût de la machine se compte par *ligne*, et la seule question à laquelle
un motif d'accès mémoire peut bien répondre est « combien d'octets utiles
porte chaque ligne chargée ? »

Le corollaire mérite d'être écrit, car c'est toute la leçon de localité en
une phrase : **le nombre qui compte est `useful bytes / 64`, et tout ce qui
l'améliore — des types plus larges, des lignes plus serrées, parcourir la
mémoire dans l'ordre où elle est disposée — vous fait remonter le tableau
vers le séquentiel ; tout ce qui ne l'améliore pas, non.** Un pas de 8
porterait 8 octets utiles par ligne et se placerait entre les colonnes. La
copie du blit elle-même parcourt des lignes entières — c'est le mieux que
les nombres de ce tableau puissent être, et la raison pour laquelle, dans un
moteur de rendu, aucune discipline de pas ne bat la simple copie de la
mémoire dans l'ordre où la mémoire vit.

*Page traduite de la version anglaise `book/solutions/lesson-047/ex2.md`,
révision `7b4f18f`.*

<!-- translation-source: book/solutions/lesson-047/ex2.md @ 7b4f18f -->
