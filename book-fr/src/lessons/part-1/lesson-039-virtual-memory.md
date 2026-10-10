# Leçon 039 — plongée dans la mémoire virtuelle

{{#include ../../stability-horizon.md}}

## Prose

Le framebuffer, c'est 1 228 800 octets qui appartiennent au moteur, et les
lectures de fichiers de la leçon 038 ont fait entrer des octets depuis l'OS
dans de la mémoire que l'OS nous a donnée. Ces deux phrases cachent une
machine : *où vivent les octets, et qui décide ?* C'est la plongée en
profondeur du cours dans la mémoire virtuelle — pages, mappages, protection —
enseignée d'abord comme concept puis mesurée sur x86-64, la machine de
référence. Elle est ici à dessein : les réservations de la leçon 040 et les
arenas de la leçon 041 sont construits exactement avec ces pièces.

### Pages

La mémoire physique est un tas d'octets ; un programme en cours d'exécution ne
la voit pas. Ce que voit le programme est un **espace d'adressage virtuel** :
un autre tas d'adresses, plus grand, plus ordonné, qui n'appartient qu'à lui.
Entre les deux se trouve la **table de pages**, et son unité est la **page** —
un morceau d'espace d'adressage de taille fixe, mappé sur un morceau de mémoire
physique de même taille.

La taille de page est un fait machine, pas une convention. Sur cette machine :

```
$ getconf PAGESIZE
4096
```

et le moteur mesure le même nombre à travers la couture
(`platform::PageSize()`) :

```
engine: page size 4096 bytes
```

Des pages de 4 Kio sur x86-64 : l'adresse virtuelle fait 48 bits de large
(128 Tio par processus), et les 12 bits de poids faible de toute adresse sont
le décalage *dans* sa page. Les bits supérieurs sont ce que traduit la table de
pages. x86-64 effectue cette traduction à travers quatre niveaux de table —
l'adresse est découpée en quatre indices de 9 bits plus le décalage de 12 bits
— et le CPU garde les résultats récents dans un cache appelé **TLB**, car
sinon, quatre lectures en mémoire par accès seraient le prix de chaque octet
touché. (x86-64 a aussi des *huge pages* de 2 Mio et 1 Gio : le même mécanisme,
les niveaux du bas sautés. Le moteur n'en aura pas besoin ; savoir qu'elles
existent explique pourquoi « taille de page » est une question dont la réponse
n'est pas toujours « 4096 ».)

### Mappages

L'espace d'adressage d'un processus n'est pas un bloc d'un seul tenant — c'est
une **table de mappages**, chacun étant une plage de pages virtuelles avec une
source et un jeu de permissions. Le noyau tient cette table, et sous Linux il
l'imprime : `/proc/self/maps` est la carte du processus qui la lit. Voici celle
du moteur, mot pour mot, tirée d'une vraie exécution (son début) :

```
5923fe127000-5923fe128000 r--p 00000000 08:30 397590   .../build/game
5923fe128000-5923fe12b000 r-xp 00001000 08:30 397590   .../build/game
5923fe12b000-5923fe12c000 r--p 00004000 08:30 397590   .../build/game
5923fe12c000-5923fe12d000 r--p 00004000 08:30 397590   .../build/game
5923fe12d000-5923fe12e000 rw-p 00005000 08:30 397590   .../build/game
5923fe12e000-5923fe25a000 rw-p 00000000 00:00 0
59242e643000-59242e664000 rw-p 00000000 00:00 0        [heap]
```

Chaque ligne est un mappage : la **plage** d'adresses virtuelles (fin exclue),
les **permissions**, le **décalage** dans le fichier sous-jacent, le
**périphérique et l'inode** du fichier, et son **chemin**. Les cinq lignes qui
nomment `build/game` sont les segments de l'exécutable lui-même — un mappage par
partie d'un même fichier. Les lignes sans chemin sont des mappages
**anonymes** : de la mémoire qui ne sous-tend rien d'autre qu'elle-même. Deux
sortes de mappages, une seule table.

Le framebuffer du moteur vit dans cette table comme tout le reste. Son adresse
dans l'exécution ci-dessus était `0x5923fe12d040` — dans la cinquième ligne, et
débordant sur la sixième. Le rapport du moteur lui-même dit pourquoi ce
débordement se produit :

```
engine: page size 4096 bytes
engine: framebuffer at 0x5923fe12d040 — 1228800 bytes = 300.00 pages (page-aligned: no)
```

Exactement 300,00 pages d'octets — `640 × 480 × 4` se divise exactement en
pages de 4 Kio — mais le tampon commence à 64 octets dans sa première page,
donc il *touche* 301 pages (l'exercice 1 imprime la plage). Les mappages
comptent en pages ; les octets s'en moquent. Quand la leçon 040 dimensionnera
des tampons en pages entières, ce sera pour cette raison.

### Protection

Les permissions de chaque mappage — `r--`, `r-x`, `rw-p` — ne sont pas
indicatives. Ce sont des bits dans les entrées de la table de pages, vérifiés par
le matériel à chaque accès : **lecture**, **écriture**, **exécution**. Relisez
la ligne du milieu de l'exécutable : `r-xp` est son code, lisible et
exécutable, jamais en écriture — et un mappage qui tenterait d'être à la fois
en écriture et en exécution est le point faible classique que les systèmes
durcis refusent.

Touchez une page d'une façon que ses permissions interdisent — écrire dans le
segment de code, lire une page `---` — et le matériel arrête l'instruction, et
l'OS délivre **SIGSEGV**. Ce n'est pas une erreur de niveau langage, et aucune
vérification en C++ ne l'attrape ; la leçon 006 montrait le comportement
indéfini du côté du langage, ici c'est le refus de la machine elle-même. (Le
crash vaut la peine d'être vu de vos propres yeux — l'exercice 2 de cette série
de leçons a un endroit sûr pour le faire.)

Le `p` de `rw-p` est l'autre moitié de la protection : **privé**, c'est-à-dire
copie sur écriture (copy-on-write). Deux processus peuvent mapper en lecture
seule les pages d'un même fichier et partager les cadres de page physiques ; le
premier qui écrit obtient sa propre copie. C'est ce qui permet à un unique
`libc.so.6` de ne tenir en mémoire qu'une seule fois pour tous les processus de
la machine.

### Pourquoi ce n'est pas un intermède

Le moteur est sur le point de cesser de prendre la mémoire que le chargeur a
bien voulu lui laisser :

- **La leçon 040** demande de la mémoire à l'OS comme cette leçon dit qu'on
  obtient réellement de la mémoire : une *réservation* — un mappage anonyme
  d'une taille choisie par le moteur, en pages entières, avec des permissions
  choisies exprès. Le framebuffer va emménager dans l'une d'elles.
- **La leçon 041** construit un arena par-dessus : un pointeur d'avancement sur
  une réservation, avec alignement et retour arrière. La carte montrera un
  grand mappage anonyme là où il y avait avant des statiques et des allocations
  de bibliothèque — et maintenant, vous pouvez lire ce que cela veut dire.

Les deux puisent leur vocabulaire dans cette leçon. C'est pourquoi la plongée
en profondeur est ici et non dans une annexe.

## Étape de code

Un seul changement pour cette leçon : la couture gagne `PageSize`, et le moteur
imprime son rapport de mémoire virtuelle — la taille de page, l'adresse du
framebuffer, sa taille en pages et son alignement — mesurés depuis l'espace
d'adressage réel du processus. Son état final est étiqueté `lesson-039`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index fafec04..0eedfe1 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -111,6 +111,18 @@ int Run(int argc, char **argv)
     double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
     double started = platform::Now();
     double last = started;
+
+    /* The virtual-memory report: every byte the engine owns lives in a
+       mapping the OS keeps, counted in pages. This is what the deep dive
+       explains — the numbers here are measured, not illustrative. */
+    size_t page = platform::PageSize();
+    size_t fb_bytes = (size_t)fb->width * fb->height * 4;
+    std::printf("engine: page size %zu bytes\n", page);
+    std::printf("engine: framebuffer at %p — %zu bytes = %.2f pages (page-aligned: %s)\n",
+                (void *)fb->pixels, fb_bytes,
+                (double)fb_bytes / (double)page,
+                (size_t)fb->pixels % page == 0 ? "yes" : "no");
+
     std::printf("engine: arrow keys move the marker; close the window to stop\n");
     std::printf("engine: marker at %d,%d\n", (int)marker_x, (int)marker_y);
 
diff --git a/src/platform.h b/src/platform.h
index ae9ddca..1c9bf6c 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -64,6 +64,9 @@ bool HasFocus(const Window *window);
    clock Part 2's frame timing and Part 5's frame-budget report stand on. */
 double Now(void);
 
+/* The OS's memory page: the unit every mapping is counted in. */
+size_t PageSize(void);
+
 /* A file's complete bytes — or a typed failure. Never partial data
    presented as success. */
 enum FileError {
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index a569e4d..58c6339 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -221,6 +221,11 @@ double Now(void)
     return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
 }
 
+size_t PageSize(void)
+{
+    return (size_t)sysconf(_SC_PAGESIZE);
+}
+
 /* File I/O is the OS side too — POSIX here, Win32's own calls in a second
    implementation. The bytes the OS reads for us live in memory the OS
    gives us (its allocator) and leave through ReleaseFile. */
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Trouvez votre mappage *(explain-in-prose)*

Le rapport vous donne l'adresse du framebuffer ; la machine vous donne sa
carte. Faites grandir le rapport avec la *plage de pages* que le tampon touche
— l'adresse arrondie vers le bas à une frontière de page, et la page du dernier
octet arrondie de même — puis, pendant que le moteur tourne, retrouvez cette
plage dans `cat /proc/<pid>/maps`. Expliquez en prose chaque colonne de la ligne
(ou des lignes) qui la contient : la plage, les permissions, le décalage, le
périphérique et l'inode, le chemin. Pourquoi 300,00 pages de données
touchent-elles 301 pages d'espace d'adressage ? Et pourquoi les adresses de la
carte diffèrent-elles d'une exécution à l'autre ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-039/ex1.md)

### Exercice 2 — Le fichier qui ment sur sa taille *(fix-the-crash)*

Le lecteur de fichier entier de la leçon 037 fait confiance à la taille
rapportée par l'OS — et certains fichiers mentent. `/proc/self/maps` rapporte
**0 octet** et produit des kilooctets à la lecture ; notre lecteur renvoie
0 octet *présenté comme un succès*, exactement l'issue que le contrat interdit.
Prouvez-le (braquez le moteur sur le fichier de carte), puis corrigez le lecteur
pour que fichier entier veuille dire fichier entier même quand la taille n'est
qu'un indice : lisez jusqu'à la fin du fichier en faisant grandir le tampon au
besoin — le `realloc` de la leçon 008 attendait ce travail. Montrez le lecteur
corrigé renvoyant les vrais octets de la carte, et écrivez ce que « octets
complets » signifie désormais pour un fichier dont la taille change pendant
qu'on le lit.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-039/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 038 — écritures de fichier entier et aller-retour](lesson-038-file-write.md) ·
**Suivante :** [Leçon 040 — des tampons adossés à une réservation](lesson-040-reservations.md) ·
**Étiquette de code :** [`lesson-039`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-039)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-039-virtual-memory.md`,
révision `00824d2`.*

<!-- translation-source: book/lessons/part-1/lesson-039-virtual-memory.md @ 00824d2 -->
