# Leçon 049 — la lentille SIMD

{{#include ../../stability-horizon.md}}

## Prose

Même C++, à un drapeau près. La leçon 048 a lu la réponse fidèle `-O0` du
compilateur à `BlitSprite` ; celle d'aujourd'hui lit sa réponse *transformée*
— le code qui tourne dans toute construction de ce moteur à partir de `-O2`
— et, à travers elle, les **registres vectoriels** : la manière qu'a la
machine de déplacer seize octets là où vous en demandiez un. La discipline de
la plongée est dans le nom même de la tâche : la *lentille* SIMD. Ce cours
n'écrit jamais de SIMD (pas d'intrinsics, pas d'assembleur inline dans le
code du moteur) ; il lit ce que le compilateur est allé chercher, et en
chiffre le prix. Écrire du code vectoriel est l'affaire de la partie 5, si
c'est l'affaire de quelqu'un — et même là, la première étape est celle de
cette leçon : lire la réponse d'abord.

### Ce qu'est un registre vectoriel

Un registre généraliste du x86-64 contient 8 octets. À côté d'eux vivent les
**registres vectoriels** : `xmm0`–`xmm15`, larges de **16 octets** chacun ;
les registres `ymm` à **32 octets** sur les CPU dotés d'AVX ; les `zmm` à
**64** avec AVX-512. Le jeu d'instructions possède des déplacements et de
l'arithmétique qui opèrent sur le registre entier d'un coup : `movdqu`
(« move double quadword, unaligned ») déplace 16 octets en une instruction.
La copie de la leçon 048 demandait un chargement et un stockage *par octet* ;
la copie vectorisée demande un chargement et un stockage *par seize*.

La largeur que le compilateur a le droit d'utiliser est une décision de
construction, pas de code : la cible x86-64 de base inclut SSE2 (`xmm`,
16 octets) ; AVX (`ymm`) se déverrouille avec des drapeaux comme
`-march=native`. Le compilateur atteint ce que vous lui avez autorisé.

### Le recensement

L'étape de code dote `tools/disasm.sh` d'un mode qui compte ce que le code
compilé de chaque fonction utilise réellement :

```
$ CXXFLAGS="-std=c++17 -O3 -g -Wall -Wextra" ./build.sh
$ ./tools/disasm.sh --census
  188  engine::Run()
   14  engine::AccountFrame(engine::FrameStats&, engine::FrameRecord const&)
    6  platform::Now()
    5  engine::LoadSprite(engine::Arena&, char const*)
    2  engine::CopyStrided(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    2  engine::CopySequential(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    2  engine::ArenaInit(engine::Arena&)
```

Lisez-le avec une précaution que les nombres eux-mêmes enseignent : ce sont
des instructions qui *touchent des registres vectoriels* — et `xmm` est aussi
là où vit la virgule flottante ordinaire. Les 14 d'`AccountFrame` sont des
`movsd`/`addsd` sur les doubles de la frame (les sommes de la leçon 036), pas
du parallélisme de données. Le recensement est une lentille ; la liste est le
fait. Deux noms manquent entièrement à l'appel : **`BlitSprite` et
`ClearBuffer` n'utilisent aucune instruction vectorielle.** Gardez cette
pensée ; c'est le cœur honnête de cette leçon.

### La copie vectorisée, lue

`CopySequential` — le parcours de la leçon 047, `dst[i] = src[i]`, rien
d'autre — compilé en `-O3`. Sa liste est assez courte pour être lue en
entier ; les parties qui comptent :

```
    195c:	lea    -0x1(%rdx),%rdi
    1960:	cmp    $0x6,%rdi
    1964:	jbe    1a78 <engine::CopySequential(...)+0x128>
    196a:	lea    0x1(%rsi),%rax
    196e:	mov    %rcx,%r8
    1971:	sub    %rax,%r8
    1974:	xor    %eax,%eax
    1976:	cmp    $0xe,%r8
    197a:	ja     1998 <engine::CopySequential(...)+0x48>
```

D'abord, la **permission** : le compilateur ne vectorise pas une copie dont
il ne peut pas prouver la sûreté. `dst` et `src` sont des pointeurs qui
pourraient se recouvrir ; un déplacement large de 16 octets est faux quand
c'est le cas. Il *demande donc à l'exécution* — la distance entre les tampons
est comparée à 14, et s'ils sont trop proches, le code prend la route sûre :

```
    1980:	movzbl (%rsi,%rax,1),%edi
    1984:	mov    %dil,(%rcx,%rax,1)
    1988:	add    $0x1,%rax
    198c:	cmp    %rax,%rdx
    198f:	jne    1980 <engine::CopySequential(...)+0x30>
```

— la boucle octet par octet, un pixel de l'anatomie de la leçon 048,
conservée pour les copies que le chemin large ne peut pas prendre. Puis la
route qu'il préfère :

```
    19a5:	and    $0xfffffffffffffff0,%rdi
    ...
    19b0:	movdqu (%rsi,%rax,1),%xmm0
    19b5:	movups %xmm0,(%rcx,%rax,1)
    19b9:	add    $0x10,%rax
    19bd:	cmp    %rdi,%rax
    19c0:	jne    19b0 <engine::CopySequential(...)+0x60>
```

Lisez ces six lignes contre les soixante-seize de la leçon 048. Le `and`
arrondit le nombre d'octets vers le bas à un multiple de 16 — la boucle en
dessous déplace exactement 16 octets par tour. `movdqu (%rsi,%rax,1),%xmm0`
charge seize octets dans un registre ; `movups` les stocke de l'autre côté ;
`add $0x10` avance de seize. **Deux instructions mémoire font ce que
trente-deux faisaient dans la liste `-O0`.** Ce que l'arrondi a laissé de
côté (0–15 octets) est traité par les queues qui suivent — une paire de
`mov` de 8 octets, puis des octets — parce que le chemin vectoriel ne couvre
que des registres complets.

Voilà la lentille SIMD en une fonction : une garde, un repli scalaire, une
boucle large, et des queues. Toute boucle auto-vectorisée a cette forme, et
une fois que vous savez nommer les quatre parties, vous savez lire la réponse
de n'importe quel compilateur à « copie cette mémoire ».

### Ce que le compilateur n'a *pas* vectorisé

Maintenant les noms manquants. La boucle interne de `BlitSprite` est ceci, en
`-O3` :

```
    1772:	movzbl 0x2(%rax),%r13d
    1777:	cmp    %r14b,(%rax)
    177a:	jne    1720 <engine::BlitSprite(...)+0x80>
    177c:	movzbl 0x11(%rsi),%r14d
    1781:	cmp    %r14b,0x1(%rax)
    1785:	jne    1720 <engine::BlitSprite(...)+0x80>
```

Des registres au lieu d'emplacements de pile (la tempête de déversements de
la leçon 048 a disparu — le `× 3` replié dans `lea (%rax,%rax,2)`, le `× 4`
dans un index mis à l'échelle), mais la boucle reste **scalaire** :
comparer, brancher, pixel par pixel. Le test de la clé en est la raison, et
c'est une *bonne* raison : que voudrait même dire « transparence
vectorisée » ? La décision est par pixel et dépend des données — chaque pixel
écrit ou n'écrit pas. Les compilateurs peuvent en principe émettre des
déplacements masqués pour de telles boucles, et GCC décline ici. Le
recensement dit `0` ; la liste dit pourquoi.

`ClearBuffer` s'en sort pareil : quatre stockages d'octets par pixel même en
`-O3`, parce qu'un pixel est quatre octets *différents* (bleu, vert, rouge,
zéro) et que la boucle est un remplissage de motif, pas une copie. Le
compilateur n'a pas de déplacement large pour « ce motif de quatre octets
exact, répété » à ces réglages.

La carte complète des sorts qu'une boucle peut rencontrer — gardez-la à côté
du recensement :

| Boucle | Sort | Preuve |
| ------ | ---- | ------ |
| `CopySequential` | **vectorisée** — `movdqu`, 16 octets/instruction | 2 au recensement, la liste large |
| `CopyStrided` | à peine vectorisée | 2 au recensement — le pas coûte la largeur |
| `LoadSprite` | partiellement vectorisée | 5 au recensement — copies larges plus queues scalaires |
| `BlitSprite` | scalaire, allouée en registres | 0 au recensement ; des branchements par pixel |
| `ClearBuffer` | scalaire | 0 au recensement ; quatre stockages d'octets par pixel |

### Ce que vaut la génération de code

Tout ce qui précède n'a changé *aucune ligne source*, et les nombres ont
bougé en conséquence (tous mesurés sur cette machine, le banc d'essai du
sprite de l'exercice 1 de la leçon 048 toujours dans l'arbre) :

| Mesure | `-O0` | `-O3` | Changement |
| ------ | ----- | ----- | ---------- |
| dessin de sprite, par sprite | 881,9 ns | 187,7 ns | **4,7×** |
| phase `render` de la frame (moy.) | 0,435 ms | 0,173 ms | 2,5× |
| parcours de copie, ligne 4 Ko | 1,5 Go/s | 102,5 Go/s | **68×** |
| parcours de copie, ligne 12 Mo | 1,5 Go/s | 16,9 Go/s | 11× |

Le 68×, c'est la largeur de la copie vectorisée plus l'effondrement du
nombre d'instructions ; le 4,7× du dessin de sprite n'est *pas* du SIMD (le
blit est resté scalaire) — c'est de l'allocation de registres et de
l'arithmétique repliée, le trafic de pile de la leçon 048 disparu. Deux
causes différentes, un seul tableau : voilà pourquoi les phases nommées de
l'enregistrement de frame (leçon 046) et ces plongées vont ensemble. La
phase `sprites` affiche toujours `0.001 ms` dans les deux constructions — le
plancher honnête d'un sprite 16×16 face à une horloge à résolution
microseconde ; le banc d'essai est ce qui lui donne un prix.

Et le drapeau qui élargit les registres : avec
`CXXFLAGS="-std=c++17 -O3 -march=native …"`, la boucle de copie devient
`vmovdqu (%rsi,%rdi,1),%ymm0` — **32 octets par instruction** — et la ligne
4 Ko mesure 169,6 Go/s contre 102,5. Même C++ ; on a autorisé le compilateur
à utiliser davantage de registres de la machine, et il s'en est servi.

### La lentille, et où elle pointe

Lire le SIMD est une compétence de la partie 2 ; l'*écrire* n'est pas un
travail de la partie 2. Le code du moteur garde son honnêteté d'une seule
boucle de copie, et les plongées tiennent leur promesse : mesurer, lire,
nommer le point de retour. Le retour de la partie 5 est concret — le top 2
des points chauds du profileur inclura très probablement quelque chose du
tableau de cette leçon, et les trois leviers sont déjà nommés : copier
moins, copier plus rapproché, copier plus large. Le troisième levier est
celui que vous savez maintenant lire avant que quiconque l'écrive.

## Étape de code

Un seul changement pour cette leçon : les parcours de copie de la sonde de
caches perdent leur `static` — le compilateur doit émettre chacun comme une
fonction nommée, pour que la lentille ait une liste à lire — et
`tools/disasm.sh` gagne le mode `--census` (instructions sur registres
vectoriels par fonction). Aucun comportement du moteur ne change. Son état
final est étiqueté `lesson-049`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 18747c7..887d551 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -25,17 +25,18 @@ constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
    memory at two strides, timed at working-set sizes that cross this
    machine's caches. The walk is the blit's inner copy with the
    bookkeeping removed: source bytes into destination bytes, nothing
-   else — so what it costs is what the blitter's copy costs. */
+   else — so what it costs is what the blitter's copy costs.
+   Lesson 049: the walkers lose their `static` so the compiler must emit
+   each one as a named function — the SIMD lens needs a listing to read. */
 
-static void CopySequential(unsigned char *dst, const unsigned char *src,
-                           size_t n)
+void CopySequential(unsigned char *dst, const unsigned char *src, size_t n)
 {
     for (size_t i = 0; i < n; ++i)
         dst[i] = src[i];
 }
 
-static void CopyStrided(unsigned char *dst, const unsigned char *src,
-                        size_t n, size_t stride)
+void CopyStrided(unsigned char *dst, const unsigned char *src, size_t n,
+                 size_t stride)
 {
     for (size_t i = 0; i < n; i += stride)
         dst[i] = src[i];
diff --git a/tools/disasm.sh b/tools/disasm.sh
index f54a09c..794c8b5 100755
--- a/tools/disasm.sh
+++ b/tools/disasm.sh
@@ -10,6 +10,7 @@
 # Usage:
 #   ./tools/disasm.sh                 # the blitter (BlitSprite)
 #   ./tools/disasm.sh ClearBuffer     # any other symbol by substring
+#   ./tools/disasm.sh --census        # vector-register instructions per function
 #
 # build/ must be current: run ./build.sh first. To read the optimized
 # build's instructions (lesson 049), build with the flags first:
@@ -28,5 +29,23 @@ fi
 
 # Only definition lines start at column 0 with an address and end in ':' —
 # call sites name the symbol too, and those are not what we are reading.
-objdump -d -C --no-show-raw-insn build/game |
-    sed -n "/^[0-9a-f]* <.*${SYMBOL}.*>:/,/^\$/p"
+listing() {
+    objdump -d -C --no-show-raw-insn build/game |
+        sed -n "/^[0-9a-f]* <.*${1}.*>:/,/^\$/p"
+}
+
+if [ "$SYMBOL" = "--census" ]; then
+    # The SIMD lens: how many vector-register instructions (xmm/ymm/zmm)
+    # each function's compiled code actually uses.
+    objdump -d -C --no-show-raw-insn build/game |
+        awk '/^[0-9a-f]+ <.*>:$/ { name = $0
+                                  sub(/^[0-9a-f]+ </, "", name)
+                                  sub(/>:$/, "", name)
+                                  next }
+             /%[xyz]mm/ { count[name]++ }
+             END { for (n in count) printf "%5d  %s\n", count[n], n }' |
+        sort -rn
+    exit 0
+fi
+
+listing "$SYMBOL"
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Deux remplissages, prédits *(predict-the-output)*

Le tableau de la leçon contient un cinquième sort qu'elle n'a fait
qu'effleurer. Ajoutez deux fonctions nommées au fichier de la sonde :
`FillPattern`, une boucle qui écrit quatre octets *différents* par pixel
(comme celle de `ClearBuffer`), et `FillUniform`, une boucle qui écrit une
seule valeur d'octet partout. Avant de lancer quoi que ce soit, écrivez ce
que le compilateur fera de chacune — vectorisée, scalaire, ou autre chose —
et pourquoi. Lancez ensuite le recensement et lisez les deux listes, puis
réconciliez : l'une de vos deux fonctions ne revient pas du tout comme une
boucle.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-049/ex1.md)

### Exercice 2 — La largeur de registre de votre compilateur *(port-to-your-own-machine)*

Le recensement compte les instructions vectorielles ; faites-lui aussi
rapporter la **largeur** du registre le plus large que chaque fonction touche
— 16 octets pour `xmm`, 32 pour `ymm`, 64 pour `zmm` — pour que la portée
d'une liste se voie d'un coup d'œil. Lancez-le ensuite deux fois sur votre
machine : le `-O3` par défaut, et
`CXXFLAGS="-std=c++17 -O3 -march=native -g -Wall -Wextra" ./build.sh`.
Prédisez d'abord quelles fonctions s'élargiront et ce que fera la ligne 4 Ko
de la sonde ; puis mesurez. Quels nombres ont bougé avec la largeur des
registres, et lesquels non ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-049/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 048 — l'assembleur compilé du blitter](lesson-048-assembly.md) ·
**Suivante :** [Leçon 050 — la police bitmap comme asset](lesson-050-font.md) ·
**Étiquette de code :** [`lesson-049`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-049)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-049-simd.md`,
révision `8c45291`.*

<!-- translation-source: book/lessons/part-2/lesson-049-simd.md @ 8c45291 -->
