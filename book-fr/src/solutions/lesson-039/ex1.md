# Solution : exercice 1 — Trouvez votre mappage

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Trouvez votre mappage](../../lessons/part-1/lesson-039-virtual-memory.md) de la leçon 039.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-039/ex1.patch}}
```

## Visite guidée

Le moteur imprime l'adresse du framebuffer ; la machine imprime sa carte.
L'instrument comble l'écart en calculant les pages que les octets touchent
réellement — l'adresse arrondie *vers le bas* à une frontière de page, et
celle du dernier octet arrondie de même :

```
engine: page size 4096 bytes
engine: framebuffer at 0x64cc86d16040 — 1228800 bytes = 300.00 pages (page-aligned: no)
engine: framebuffer spans 0x64cc86d16000 .. 0x64cc86e43000 — 301 pages
```

300,00 pages *de données*, réparties sur 301 pages *d'espace d'adressage*. La
différence est le `0x40` à la fin de l'adresse : le tampon commence 64 octets
après le début de sa première page, donc il ne peut pas tenir dans 300 pages
entières — il déborde sur une 301e. C'est l'alignement que la leçon 007
enseignait, qui se voit dans la carte.

Maintenant, trouvez-la. Lancez le moteur et regardez sa carte pendant qu'il
tourne (les adresses diffèrent à chaque exécution — l'ASLR place les mappages à
un endroit neuf chaque fois, ce qui fait lui-même partie du propos) :

```
$ cat /proc/<pid>/maps | head -7
5923fe127000-5923fe128000 r--p 00000000 08:30 397590   .../build/game
5923fe128000-5923fe12b000 r-xp 00001000 08:30 397590   .../build/game
5923fe12b000-5923fe12c000 r--p 00004000 08:30 397590   .../build/game
5923fe12c000-5923fe12d000 r--p 00004000 08:30 397590   .../build/game
5923fe12d000-5923fe12e000 rw-p 00005000 08:30 397590   .../build/game
5923fe12e000-5923fe25a000 rw-p 00000000 00:00 0
59242e643000-59242e664000 rw-p 00000000 00:00 0        [heap]
```

Dans cette exécution, le framebuffer se trouvait à `0x5923fe12d040` — dans la
*cinquième* ligne, la page de données en lecture-écriture de l'exécutable
lui-même — et ses 300 pages d'octets débordaient sur la *sixième*, le mappage
anonyme qui va de `5923fe12e000` à `5923fe25a000` (une étendue de `0x12C000` =
1 228 800 octets, exactement la taille du framebuffer). La carte n'est pas une
photo de « le programme » : c'est une table de plages, et un même tampon peut
être à cheval sur deux lignes.

Lisez les colonnes : la **plage** (début et fin, fin exclue), les
**permissions** (`r--`, `r-x`, `rw-p` — le `p` veut dire « privé », copie sur
écriture), le **décalage** (où commence ce mappage dans le fichier — `00000000`
pour la mémoire anonyme), le **périphérique et l'inode** (le fichier derrière
le mappage, absent quand il n'y en a pas), et **le chemin** (vide pour la
mémoire anonyme, `[heap]`/`[stack]` pour les étiquettes du noyau lui-même).

Les cinq premières lignes de l'exécutable sont une par segment d'un même
fichier — notez la ligne `r-xp` au milieu : c'est du *code*, mappé en
lecture-exécution, jamais en écriture. Les protections imprimées par la carte
sont les bits de table de pages décrits par la plongée en profondeur, et voici
à quoi elles ressemblent de l'extérieur.

La rédaction à viser : les octets de la frame n'habitent pas « dans le
programme » comme un fait vague — ils habitent une plage précise d'adresses
virtuelles, adossée à des cadres de page que l'OS a assignés, et la carte est
le reçu.

*Page traduite de la version anglaise `book/solutions/lesson-039/ex1.md`,
révision `00824d2`.*

<!-- translation-source: book/solutions/lesson-039/ex1.md @ 00824d2 -->
