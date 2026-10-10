# Leçon 047 — plongée dans les caches

{{#include ../../stability-horizon.md}}

## Prose

Le bilan de la frame disait `render 0.435 ms` et, dedans, `sprites 0.001
ms`. Une microseconde pour dessiner un sprite ; quatre cents pour peindre
l'arrière-plan. Cette leçon répond à la question que pose cette
arithmétique : **combien coûte réellement la copie d'un octet ?** La réponse
n'est ni dans la norme C++ ni dans le manuel x86-64 non plus — elle est dans
les petites mémoires rapides logées entre le CPU et la grande mémoire lente.
C'est la *plongée dans les caches* : le concept d'abord, x86-64 comme
exemple travaillé (la machine que ce cours cible ; chaque concept ici est
celui de la machine, pas celui du jeu d'instructions), mesurée sur notre
propre boucle de copie, et se terminant là où se termine toute plongée de ce
cours — sur ce que la partie 5 en fera.

### La mémoire n'est pas des octets : c'est des lignes de cache

Le CPU ne va jamais chercher un seul octet. Il charge une **ligne de cache**
— sur cette machine, 64 octets, et vous pouvez poser la question directement
à la machine :

```
$ for i in /sys/devices/system/cpu/cpu0/cache/index*; do
>   echo "$i: L$(cat $i/level) $(cat $i/type) $(cat $i/size) line=$(cat $i/coherency_line_size)"
> done
/sys/devices/system/cpu/cpu0/cache/index0: L1 Data 48K line=64
/sys/devices/system/cpu/cpu0/cache/index1: L1 Instruction 64K line=64
/sys/devices/system/cpu/cpu0/cache/index2: L2 Unified 3072K line=64
/sys/devices/system/cpu/cpu0/cache/index3: L3 Unified 20480K line=64
```

Quatre faits de cette machine, droit sortis du noyau : un cache de données
de 48 Ko au plus près du cœur (**L1d**), un cache de 3 Mo partagé mais
proche (**L2**), un cache de dernier niveau de 20 Mo (**L3**), et des lignes
de 64 octets partout. Entre eux et la mémoire principale, la latence varie
d'un facteur cinquante ; entre les étages, c'est chaque fois de deux à
quatre fois. Les caches ne sont pas un détail d'optimisation — ils sont *le
système mémoire*, et la RAM est le magasin lent qu'ils dissimulent.

Une ligne chargée est l'unité de tout coût mémoire. Lisez un octet et la
machine en charge 64 ; touchez l'octet suivant et il est déjà là. Touchez un
octet à 64 octets de distance et la machine charge une *seconde* ligne. La
différence entre ces deux phrases est la différence entre les colonnes de
chaque tableau de cette leçon.

Traduisons les nombres de la frame en lignes : une ligne du framebuffer fait
`640 × 4 = 2560` octets = 40 lignes ; une ligne du sprite fait `16 × 3 = 48`
octets — dans une seule ligne. Les 768 octets du sprite entier font douze
lignes. Le framebuffer fait 19 200 lignes et ne peut tenir dans aucun cache
de cette machine. Ces deux phrases prédisent presque tout ce que les mesures
ci-dessous montrent.

### La sonde : la copie du blit, mesurée

L'étape de code ajoute un parcours de copie — des octets source vers des
octets destination, rien d'autre. C'est la copie interne de `BlitSprite`
avec le test de la clé et l'échange d'octets retirés, en deux formes :
**séquentielle** (`dst[i] = src[i]` pour chaque octet) et **pas de 64**
(`dst[i] = src[i]` pour chaque 64e octet — un octet utile par ligne de
cache). Chacune est chronométrée sur des ensembles de travail (working sets)
qui traversent les caches ci-dessus, et le tableau rapporte des Go/s
*utiles* — les octets que le parcours a réellement consommés, pas les octets
que la machine a chargés.

Construit avec `-O3` (un seul drapeau changé ; les deux leçons suivantes
expliquent pourquoi cela compte), épinglé à un cœur (`taskset -c 3`), sur la
machine dont les caches sont imprimés ci-dessus :

```
engine: cache probe — copy walk, useful GB/s per stride
engine: working set   sequential    stride 64
engine:       4 KB        102.5          3.0
engine:      64 KB         59.0          1.1
engine:    512 KB         52.1          0.9
engine:   4096 KB         14.7          0.4
engine:   8192 KB         16.3          0.3
engine:  12288 KB         16.9          0.3
```

Lisez la colonne séquentielle de haut en bas et vous regardez la hiérarchie
(un détail que le tableau porte à découvert : l'étiquette de ligne compte
les octets *copiés par tampon* — le parcours touche la source et la
destination, donc la mémoire qu'il foule vaut le double de l'étiquette ; et
les chiffres d'une exécution bougent de quelques pour cent d'une exécution à
l'autre, alors que ce sont les chutes qui constituent la thèse) :

- **4 Ko : 102 Go/s** — l'ensemble de travail vit tout entier dans L1d. La
  copie tourne à la vitesse du cœur ; la mémoire n'intervient plus après la
  première ligne.
- **64 Ko – 512 Ko : ~55 Go/s** — au-delà de L1d (48 Ko), encore dans L2.
  Deux fois et demie plus lent, et stable : chaque ligne est à un saut
  court.
- **4 Mo – 12 Mo : ~15 Go/s** — l'ensemble de travail (deux tampons, donc
  2× la taille de la ligne) a débordé L2 (3 Mo) puis L3 (20 Mo). C'est la
  vitesse de la mémoire principale : sept fois plus lent que L1d, et là où
  finit par vivre toute grande scène.

La colonne des pas de 64, c'est la même machine interrogée de travers :
3,0 Go/s dans L1d jusqu'à 0,3 Go/s en mémoire — **trente fois plus lent** à
ensemble de travail égal. Non pas parce que les pas seraient lents, mais
parce que chaque toucher fait entrer 64 octets pour n'en utiliser qu'un. Le
tableau rapporte les octets utiles, donc le gaspillage est exactement
visible : un pas de 64 octets dans le régime mémoire déplace 0,3 Go/s utiles
pendant que la machine charge 64 × 0,3 ≈ 19 Go/s de lignes de cache pour le
faire. Séquentiel et à pas touchent les mêmes lignes ; le séquentiel obtient
64 octets utiles par chargement.

Voilà tout le concept : **le coût se compte par ligne chargée, et la
localité, c'est le nombre d'octets utiles que vous obtenez par ligne.** Le
reste est du détail.

### La boucle est aussi un coût

La même sonde, construite avec le `-O0` par défaut — la construction que ce
cours livre — montre quelque chose que le tableau `-O3` cache :

```
engine: cache probe — copy walk, useful GB/s per stride
engine: working set   sequential    stride 64
engine:       4 KB          1.3          0.5
engine:      64 KB          1.4          0.6
engine:    512 KB          1.5          0.6
engine:   4096 KB          1.5          0.4
engine:   8192 KB          1.6          0.3
engine:  12288 KB          1.5          0.2
```

Plate à ~1,5 Go/s. Non pas que le système mémoire ait changé — mais à `-O0`,
la boucle de copie fait un octet par *séquence d'instructions* : charger un
octet, stocker un octet, incrémenter l'index, comparer, sauter. Le CPU passe
son temps dans la boucle, pas dans la mémoire ; on ne sollicite jamais les
caches assez fort pour qu'apparaisse un coude. Une copie a deux couches de
coût — le code qui tourne et la mémoire qu'il touche — et à `-O0`, la
première couche est toute l'histoire.

Ce n'est pas un défaut à corriger ici (aucune optimisation n'atterrit dans
la partie 2 — la correction, c'est le menu en trois passes de la partie 5).
C'est le fil que tirent les deux leçons suivantes : la leçon 048 lit ce que
le compilateur a réellement fait de la boucle, et la leçon 049 lit la forme
vectorisée qu'il atteint quand il y est autorisé. Le couple `-O0`/`-O3`
ci-dessus est le même code — la différence tient entièrement à ce que le
compilateur a émis.

### Ce que coûte réellement la copie du blitter

Retour à la frame. À chaque frame, le moteur déplace :

| Chemin | Octets | Où |
| ------ | ------ | --- |
| `ClearBuffer` | 1 228 800 écrits | framebuffer — 19 200 lignes, froides à chaque frame |
| `BlitSprite` | 768 lus + ≤1 024 écrits | sprite (12 lignes, toujours chaud) vers le framebuffer |
| `Present` | 1 228 800 lus | framebuffer vers la fenêtre |

Environ 2,4 Mo de trafic par frame — à 60 frames par seconde, à peu près
150 Mo/s face à une machine dont le système mémoire déplace 15 000 Mo/s et
dont le L1d déplace 100 000 Mo/s. En 640×480 avec un seul sprite, **les
caches ne sont pas le problème de cette frame**, et le `sprites 0.001 ms` du
bilan le dit : les douze lignes du sprite sont résidentes une fois que le
clear les a touchées.

Mais regardez d'où viendrait la pression. Le clear est une passe sur un
ensemble de travail qui tient dans L2 mais pas dans L1d — il tourne à la
vitesse de L2 (le voisinage de la ligne 512 Ko : des dizaines de Go/s) moins
la surcharge `-O0` de la boucle, ce qui explique exactement pourquoi le
bilan affiche 0,4 ms et non 0,04. Une scène avec cinquante sprites
multiplierait par cinquante les octets du chemin de blit — toujours petit.
Une scène avec une *carte* redessinée à chaque frame, ou un framebuffer
quatre fois plus grand, déplace l'ensemble de travail vers le bas de la
courbe — et le rapport de budget de frames de la partie 5 est l'endroit où
cela apparaît en chiffres plutôt qu'en peurs.

### Et voici ce que la partie 5 en fera

La plongée s'arrête à la mesure, comme toute plongée ici. Les hooks nommés
qui attendent dans la partie 5 :

- **La passe de profilage rejoue ces nombres** sur la scène réelle du jeu
  terminé — même sonde, mêmes colonnes — et nomme le top 2 des points
  chauds avant que quoi que ce soit ne soit corrigé (l'étape 1 du menu en
  trois passes).
- **Les phases nommées de l'enregistrement de frame** (les `sprites` de la
  leçon 046, et les phases texte et tilemap à venir) attribuent le coût aux
  sous-systèmes ; la courbe des caches explique la forme de chaque ligne.
- **Les corrections de l'étape 2 viennent des lignes de ce tableau** :
  copier moins (des régions sales au lieu de clears complets), copier plus
  rapproché (localité), copier plus large (vectorisation — la leçon
  suivante). Chacune est l'un des trois leviers que cette plongée a nommés ;
  la partie 5 les actionne, mesures à l'appui.

## Étape de code

Un seul changement pour cette leçon : `main.cpp` accueille la sonde de
caches — deux boucles de copie (`CopySequential`, `CopyStrided`), le
balayage des ensembles de travail et des pas, et l'arena agrandie à 32 Mo
pour que la sonde dépasse tous les caches de la machine. La sonde tourne une
fois au démarrage, avant la boucle de frames. Son état final est étiqueté
`lesson-047`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 1f8ad5c..18747c7 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -21,6 +21,72 @@ namespace engine {
    per-frame step. */
 constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
 
+/* Lesson 047: the caches deep dive's evidence — a copy walk over arena
+   memory at two strides, timed at working-set sizes that cross this
+   machine's caches. The walk is the blit's inner copy with the
+   bookkeeping removed: source bytes into destination bytes, nothing
+   else — so what it costs is what the blitter's copy costs. */
+
+static void CopySequential(unsigned char *dst, const unsigned char *src,
+                           size_t n)
+{
+    for (size_t i = 0; i < n; ++i)
+        dst[i] = src[i];
+}
+
+static void CopyStrided(unsigned char *dst, const unsigned char *src,
+                        size_t n, size_t stride)
+{
+    for (size_t i = 0; i < n; i += stride)
+        dst[i] = src[i];
+}
+
+static void CacheProbe(Arena &arena)
+{
+    const size_t sizes[] = { 4096, 65536, 524288, 4194304, 8388608,
+                             12582912 };
+    const size_t biggest = 12582912;
+
+    /* Two allocations the compiler cannot connect: source and destination
+       are separate arena blocks — 2 × size bytes of working set per walk,
+       and the copy loop is free to run wide. */
+    unsigned char *src = (unsigned char *)ArenaAlloc(arena, biggest, 64);
+    unsigned char *dst = (unsigned char *)ArenaAlloc(arena, biggest, 64);
+    if (!src || !dst) {
+        std::printf("engine: cache probe: no room in the arena\n");
+        return;
+    }
+    for (size_t i = 0; i < biggest; i += 4096) {
+        src[i] = (unsigned char)(i * 7); /* touch every page first */
+        dst[i] = 0;
+    }
+
+    std::printf("engine: cache probe — copy walk, useful GB/s per stride\n");
+    std::printf("engine: %10s %12s %12s\n", "working set", "sequential",
+                "stride 64");
+    for (unsigned s = 0; s < sizeof sizes / sizeof sizes[0]; ++s) {
+        double gbs[2];
+        for (unsigned mode = 0; mode < 2; ++mode) {
+            size_t stride = mode == 0 ? 1 : 64;
+            size_t per_rep = sizes[s] / stride;
+            long reps = (long)(8000000 / per_rep);
+            if (reps < 1)
+                reps = 1;
+            double t0 = platform::Now();
+            for (long r = 0; r < reps; ++r) {
+                if (mode == 0)
+                    CopySequential(dst, src, sizes[s]);
+                else
+                    CopyStrided(dst, src, sizes[s], 64);
+            }
+            double seconds = platform::Now() - t0;
+            gbs[mode] = (double)per_rep * reps / seconds / 1e9;
+        }
+        std::printf("engine: %7zu KB %12.1f %12.1f\n", sizes[s] / 1024,
+                    gbs[0], gbs[1]);
+    }
+}
+
 int Run(void)
 {
     platform::WindowResult opened =
@@ -44,9 +110,11 @@ int Run(void)
     }
 
     /* The engine's memory: one arena over one reservation. Everything the
-       engine allocates lives in here and is released together. */
+       engine allocates lives in here and is released together. Lesson 047
+       grows it past every cache this machine has, so the cache probe can
+       walk working sets bigger than all of them. */
     Arena arena;
-    ArenaInit(arena, 4 * 1024 * 1024);
+    ArenaInit(arena, 32 * 1024 * 1024);
     Framebuffer *fb = GetFramebuffer(arena);
 
     /* Lesson 044: the sprite is a file's bytes. It is loaded once, at
@@ -152,6 +220,9 @@ int Run(void)
     std::printf("engine: blit check: clip at -4,-4 landed %d pixels, %d wrong, %d touched outside\n",
                 landed, wrong, wrapped);
 
+    /* Lesson 047's evidence, measured before the loop starts. */
+    CacheProbe(arena);
+
     double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
     double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Les coudes de votre machine *(measure-the-performance)*

Six lignes montrent la forme ; une machine mérite sa propre carte. Étendez le
balayage entre les tailles de la leçon — ajoutez 16 Ko, 48 Ko, 128 Ko, 1 Mo,
2 Mo — et lancez en `-O3` sur votre propre machine. Avant de lancer,
prédisez où la bande passante devrait chuter, à partir des tailles de cache
de votre CPU (la commande `/sys` de la leçon fonctionne sur tout Linux).
Comparez ensuite : où sont les coudes que vous avez mesurés, où étaient ceux
que vous aviez prédits, et que dit l'*écart* entre un coude et votre
prédiction sur ce qui d'autre touche votre mémoire ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-047/ex1.md)

### Exercice 2 — Le pas qui ne rentre pas dans la ligne *(predict-the-output)*

Ajoutez une troisième colonne à la sonde : **un pas de 60**. Avant de lancer
quoi que ce soit, écrivez où vous attendez qu'elle se situe entre le pas de
64 et le séquentiel à chaque ensemble de travail — la ligne fait 64 octets
et 60 n'y divise pas proprement, alors réfléchissez au nombre de touchers
qui tombent dans chaque ligne avant d'arrêter un chiffre. Lancez ensuite en
`-O3` et réconciliez la colonne avec votre prédiction. La question que
posent les nombres : est-ce la *valeur du pas* qui coûte, ou les *octets
utiles par ligne* ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-047/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 046 — le sprite se déplace](lesson-046-movable-sprite.md) ·
**Suivante :** [Leçon 048 — l'assembleur compilé du blitter](lesson-048-assembly.md) ·
**Étiquette de code :** [`lesson-047`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-047)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-047-caches.md`,
révision `d843c56`.*

<!-- translation-source: book/lessons/part-2/lesson-047-caches.md @ d843c56 -->
