# Solution : exercice 2 — Ce que coûte la certitude

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Ce que coûte la certitude](../../lessons/part-0/lesson-005-leaks.md) de la leçon 005.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-005/ex2.patch}}
```

## Visite guidée

Sur une machine (gcc 13.3.0, x86-64), `time ./wordcount med.txt` sur un
fichier texte de 20 Mo :

```
fgetc, plain:       real 0m0.112s
fgetc, sanitizer:   real 0m0.174s
fread,  plain:      real 0m0.075s
fread,  sanitizer:  real 0m0.150s
```

La boucle par octet noyait effectivement une partie du signal, et le
changement de lecture par blocs — un `fread` dans un morceau de 4 Ko, puis le
même corps par octet sur le morceau — est ce qui sépare les coûts : le
sanitizer tourne environ 1,6× plus lentement sur la boucle `fgetc` et
environ 2× plus lentement sur la boucle par blocs. Où va le temps
supplémentaire : chaque chargement et chaque stockage dans le code
instrumenté consulte la carte de *shadow memory*, et chaque `malloc`,
`realloc` et `free` est celui de l'exécution, payant comptabilité, redzones
et une quarantaine de blocs récemment libérés. La mémoire coûte aussi — la
carte fantôme est une fraction de tout l'espace d'adresses, et chaque
allocation porte ses barrières. Les nombres disent que la certitude n'est
pas gratuite mais est assez bon marché pour rester branchée pendant toute la
phase de débogage — et à éteindre pour la construction de production, ce qui
est exactement pourquoi les deux commandes de construction vivent côte à côte.

Vos ratios différeront ; la méthode est le livrable : mesurez l'outil qui
surveille votre code avant de décider qu'il est trop lent à utiliser.

*Page traduite de la version anglaise `book/solutions/lesson-005/ex2.md`,
révision `a66c119`.*

<!-- translation-source: book/solutions/lesson-005/ex2.md @ a66c119 -->
