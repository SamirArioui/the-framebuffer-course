# Solution : exercice 1 — Les coudes de votre machine

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Les coudes de votre machine](../../lessons/part-2/lesson-047-caches.md) de la leçon 047.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-047/ex1.patch}}
```

## Visite guidée

D'abord la prédiction, à partir de la description que la machine donne
d'elle-même. Ce CPU annonce L1d 48 Ko, L2 3 Mo, L3 20 Mo, des lignes de
64 octets. Le parcours touche deux tampons, donc une ligne étiquetée *n* Ko
foule *2n* Ko de mémoire — les coudes devraient tomber là où `2n` franchit un
cache :

- **L1d (48 Ko)** — entre la ligne 16 Ko (32 Ko touchés) et la ligne 48 Ko
  (96 Ko touchés).
- **L2 (3 Mo)** — entre 1 Mo (2 Mo touchés) et 2 Mo (4 Mo touchés).
- **L3 (20 Mo)** — à la dernière ligne : 12 Mo copiés touchent 24 Mo,
  au-delà.

Le balayage étendu en `-O3`, épinglé à un cœur :

```
engine: cache probe — copy walk, useful GB/s per stride
engine: working set   sequential    stride 64
engine:       4 KB        108.3          3.0
engine:      16 KB        116.5          4.6
engine:      48 KB         53.5          1.2
engine:      64 KB         56.3          1.2
engine:     128 KB         42.8          0.9
engine:     512 KB         49.7          0.9
engine:    1024 KB         40.4          0.9
engine:    2048 KB         23.7          0.5
engine:    4096 KB         19.2          0.4
engine:    8192 KB         16.9          0.3
engine:   12288 KB         11.5          0.3
```

Les trois coudes tombent là où ils étaient prédits — et la forme entre eux
vaut autant que les chutes :

- **16 Ko → 48 Ko : 116 → 53 Go/s.** Le coude de L1d, exactement à
  l'heure : 32 Ko touchés tiennent, 96 Ko non.
- **1 Mo → 2 Mo : 40 → 24 Go/s.** Le coude de L2 — doux, parce que L3
  absorbe la différence avant la mémoire principale.
- **8 Mo → 12 Mo : 17 → 11,5 Go/s.** Au-delà de L3, la vitesse de la
  mémoire principale.

Maintenant la partie sur laquelle l'énoncé interroge — l'*écart* entre la
prédiction et la mesure. Il n'est pas nul, et les lignes vacillent : 16 Ko se
lit *plus vite* que 4 Ko (116 contre 108), et 512 Ko plus vite que 128 Ko
(50 contre 43). Rien dans la hiérarchie des caches ne dit que cela devrait
arriver. Ce qu'est ce bruit :

- **Les caches sont partagés.** Sur cette puce, L2 et L3 sont unifiés et
  partagés avec les autres cœurs ; tout ce que la machine exécute d'autre
  pendant la sonde (et il y a toujours quelque chose) déplace des lignes.
- **La fréquence n'est pas constante.** Les fréquences boost, l'état
  thermique et les limites de puissance font bouger la vitesse du cœur sous
  charge soutenue — un balayage de 12 Mo qui tourne pendant des secondes est
  exactement la charge qui le provoque.
- **La sonde elle-même a des coûts fixes** — la mise en place de la boucle,
  les lectures d'horloge — amortis sur la mesure ; les petites lignes les
  sentent davantage.

La règle de la partie 0, reformulée pour les caches : *une mesure sans sa
machine est une rumeur, et une mesure sans son bruit est un mensonge.*
Épinglez le cœur (`taskset`), prenez la forme, et traitez les écarts d'un
seul chiffre entre lignes voisines comme de la météo. Les chutes d'un
facteur deux ou sept, c'est le climat.

*Page traduite de la version anglaise `book/solutions/lesson-047/ex1.md`,
révision `7b4f18f`.*

<!-- translation-source: book/solutions/lesson-047/ex1.md @ 7b4f18f -->
