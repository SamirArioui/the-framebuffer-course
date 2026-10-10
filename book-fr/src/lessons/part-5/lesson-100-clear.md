# Leçon 100 — passe 2b : corriger le clear

{{#include ../../stability-horizon.md}}

## Prose

Le deuxième et dernier correctif du menu. La passe 1 a nommé deux
points chauds ; la passe 2a a fait baisser le dessin de la carte de 43 % ;
cette leçon prend l'autre — **le clear de la frame** : `ClearBuffer`,
un appel par frame, à chaque frame, peignant les 307 200 pixels du
framebuffer — `clear 0.456 ms` d'une frame de jeu (23 %) et `0.473 ms`
d'une frame de panneau (**49 % de chaque frame qui ne dessine pas de
carte**), `1.33 s` (37.7 %) du CPU du profil. Une fonction, une boucle,
et le troisième levier des plongées en profondeur : **copier plus large**.

### Ce que coûte le clear aujourd'hui

La boucle écrit chaque pixel en quatre stockages d'octets à travers un
pointeur d'octets : `p[0] = b; p[1] = g; p[2] = r; p[3] = 0;` — quatre
stockages, une adresse calculée par pixel, 1,2 million de stockages
d'octets par frame. La leçon 049 a chiffré cette boucle exacte dans sa
table de résultats : « `ClearBuffer` s'en sort pareil : quatre stockages
d'octets par pixel même en `-O3`, parce qu'un pixel est quatre octets
*différents* … et la boucle est un remplissage de motif, pas une copie. »

Mais la lentille de cette leçon dit aussi quoi en faire : un remplissage
d'*une seule valeur répétée* est la forme la plus vectorisable qui
existe — si la boucle donne à la machine une seule valeur à répéter.

### Le correctif : un mot par pixel

Un pixel est quatre octets, et quatre octets sont un mot. Le correctif
écrit un `unsigned int` par pixel au lieu de quatre octets — et compose
le mot *à partir des octets du framebuffer lui-même* : un tableau de
quatre octets `{b, g, r, 0}` et un `memcpy` vers le mot. Quel que soit
l'ordre dans lequel cette machine stocke les mots, les quatre octets
atterrissent exactement là où les stockages d'octets les mettaient ; la
boucle ne repense plus jamais à l'ordre des octets. Mêmes pixels, un
stockage sur quatre.

Et la boucle de remplissage a maintenant la forme que lit la leçon 049 :
une valeur, une région contiguë, aucune décision. Le recensement
(`tools/disasm.sh --census` en `-O3`) dit ce que le compilateur en a
fait — `ClearBuffer` est passé de **0 instruction vectorielle à 5**, et
la liste est la leçon en quatre parties en miniature :

```
    5891:	movd   %ecx,%xmm1          ; the color, in a register
    589b:	pshufd $0x0,%xmm1,%xmm0    ; splat: four pixels per word-load
    ...
    58c0:	movups %xmm0,(%rax)        ; the wide loop: 32 bytes a turn
    58c3:	add    $0x20,%rax
    58c7:	movups %xmm0,-0x10(%rax)   ; two stores, eight pixels
    58cb:	cmp    %rdi,%rax
    58ce:	jne    58c0
```

— une garde pour les petits comptes, la boucle large avançant **huit
pixels par tour**, et les queues repliées après elle. Le cours n'écrit
jamais de SIMD ; il donne sa forme à la boucle et lit ce que le
compilateur a saisi (la règle de 049). Ici, le compilateur a saisi.

### Le coût mesuré baisse

Même exécution de mesure, même découpage `step` (1 551 frames, 1 415 de
jeu) :

```
                                 before (lesson-099)   after (lesson-100)
  play frames, clear               0.449 ms              0.239 ms
  panel frames, clear              0.446                 0.246
  play frames, total               1.505 ms              1.269 ms
```

Le point chaud nommé **baisse de 47 %** sur les frames qui comptent le
plus — une frame de panneau coûte maintenant `0.769 ms`, dont la moitié
était autrefois le clear. L'exécution profilée (2 583 frames) est
d'accord : le temps propre de `ClearBuffer` `1.05 s → 0.70 s`, le CPU de
toute l'exécution `2.28 s → 1.98 s`. Pas le quadruplement que suggère le
compte de stockages en `-O0` — la surcharge de boucle qui reste est la
comptabilité du compilateur, et en `-O3` la boucle large avance de huit
pixels par tour comme cité plus haut. Les deux nombres sont ceux de cette
compilation ; les deux ont baissé.

### Deux correctifs, un menu, et à quoi ressemble la frame maintenant

Le menu en trois passes est épuisé : la mesure en a nommé deux, les deux
sont corrigés, et rien d'autre n'a été touché. Le bilan de la frame de
jeu, de la première mesure à celle-ci :

```
play frames:            lesson-098   lesson-099   lesson-100
  tilemap                 0.981        0.559        0.539
  clear                   0.456        0.449        0.239
  sprites                 0.006        0.006        0.006
  text                    0.012        0.010        0.010
  update + audio          0.045        0.046        0.046
  present                 0.444        0.434        0.428
  total                   1.944        1.505        1.269
```

Les deux lignes nommées ont baissé de `1.437 → 0.778 ms` ensemble et la
frame a baissé avec elles (−35 %) ; les lignes que le menu n'a pas
nommées — le `present` de la couture avant tout — sont maintenant la plus
grosse ligne unique de la frame. Ce n'est pas un échec du menu ; c'*est*
le menu : tout ce que les passes n'ont pas nommé est du travail futur, et
le rapport de la passe 3 dira exactement cela, mesures à l'appui.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Le changement s'attaque au point chaud nommé et le coût mesuré
  baisse** — `clear 0.449 → 0.239 ms` sur 1 415 vraies frames de jeu (et
  `0.446 → 0.246 ms` sur les frames de panneau) ; `ClearBuffer` propre
  `1.05 s → 0.70 s` sur 2 583 vraies frames de la compilation profilée.
- **Le comportement du jeu est inchangé** — au niveau de l'octet : un
  harnais jetable a fait passer 400 effacements (couleurs aléatoires,
  tailles aléatoires, largeurs impaires incluses) dans les deux boucles :
  **zéro octet différent**. Au niveau du rapport : la transcription de
  l'exécution scriptée se réduit au même ensemble de rapports que celui
  de l'état précédent — les mêmes 106 gabarits (une ligne de sonde
  atterrissant quelques positions plus tôt dans la séquence du combat —
  le plancher de bruit que la leçon 097 a mesuré).
- **La réponse du compilateur est lue, pas supposée** — les 5
  instructions vectorielles du recensement et la liste ci-dessus sont la
  preuve que la boucle élargie existe tout court en `-O3` ; en `-O0` (la
  compilation de ce cours) le gain est le compte de stockages, et il est
  mesuré, pas argumenté.

Ce que cette exécution n'a **pas** vérifié, c'est quoi que ce soit de
l'ordre des octets ou de l'alignement de *votre* machine — l'argument de
sûreté du remplissage par mot (pourquoi les octets atterrissent là où les
stockages d'octets les mettaient ; pourquoi le pointeur de mot est
autorisé) est celui de l'exercice 2, et il vaut la peine de le faire de
vos propres yeux, parce qu'un remplissage rapide qui ment sur ses pixels
est pire qu'un lent. Et les nombres restent ceux de cette machine (WSL2,
Xvfb `:99`, `-O0`) : la baisse est réelle ici, sa taille est celle de
cette machine.

## Étape de code

Un changement : le point chaud n° 2. Le `ClearBuffer` de
`src/framebuffer.cpp` écrit un mot par pixel — le mot composé à partir
des quatre octets du framebuffer lui-même (`{b, g, r, 0}` via `memcpy`,
si bien que l'ordre des octets est celui de la machine et les pixels sont
ceux du moteur) — dans un remplissage contigu que le compilateur peut
élargir. Les octets écrits sont les octets qu'écrivait l'ancienne boucle.
`BlitSpriteFrame`, `PutPixel` et le reste du framebuffer ne sont pas
touchés. Son état final est étiqueté `lesson-100`.

```diff
diff --git a/src/framebuffer.cpp b/src/framebuffer.cpp
index 88080d5..b36981f 100644
--- a/src/framebuffer.cpp
+++ b/src/framebuffer.cpp
@@ -9,6 +9,8 @@
 
 #include "framebuffer.h"
 
+#include <cstring>
+
 namespace engine {
 
 static Framebuffer framebuffer = { 0, FRAME_WIDTH, FRAME_HEIGHT };
@@ -25,14 +27,23 @@ Framebuffer *GetFramebuffer(Arena &arena)
 void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
                  unsigned char b)
 {
+    /* Lesson 100: the clear is the frame's second-hottest work (the
+       measure pass, lesson 098) — 307,200 pixels every frame — and its
+       loop writes four bytes per pixel through a byte pointer. The deep
+       dives' lever is "copy wider": one *word* per pixel, in a
+       contiguous fill the compiler can widen (lesson 049's lens reads
+       the answer below). The word is composed from the framebuffer's
+       own bytes — bytes through memcpy, so whatever order this machine
+       stores words in, the four bytes land exactly as the byte stores
+       put them. Same pixels, one store in four. */
+    const unsigned char bytes[4] = { b, g, r, 0 };
+    unsigned int color;
+    std::memcpy(&color, bytes, sizeof color);
+
+    unsigned int *pixels = (unsigned int *)fb.pixels;
     int count = fb.width * fb.height;
-    for (int i = 0; i < count; ++i) {
-        unsigned char *p = fb.pixels + i * 4;
-        p[0] = b;
-        p[1] = g;
-        p[2] = r;
-        p[3] = 0;
-    }
+    for (int i = 0; i < count; ++i)
+        pixels[i] = color;
 }
 
 void PutPixel(Framebuffer &fb, int x, int y, unsigned char r,
```

## Exercices

Deux défis qui achèvent l'éducation du correctif — la mesure avant/après
avec votre propre instrument, et l'argument de sûreté qu'un remplissage
rapide doit à ses pixels. Chacun se termine par sa solution — un diff
contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Le prix d'un pixel *(measure-the-performance)*

La leçon cite la baisse de la ligne `clear` ; la ligne est le coût d'une
frame, pas d'un pixel. Chiffrez le pixel : un banc au démarrage qui
chronomètre `ClearBuffer` sur le vrai tampon à plusieurs tailles (les
640×480 de la frame jusqu'à une petite région) et imprime des
nanosecondes par pixel. Le patch du banc s'applique aussi bien à
`lesson-099` qu'ici — les fichiers qu'il touche ne sont pas touchés par
l'étape de code de cette leçon — alors lancez-le aux deux étiquettes et
rapportez le prix par pixel avant et après, à côté de la prédiction par
comptage d'instructions que fait l'argument de la leçon (quatre stockages
d'octets et une adresse chacun → un stockage de mot chacun). Lequel de
vos nombres appartient au compte de stockages, et lequel à la boucle que
le compilateur laisse derrière lui en `-O0` ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-100/ex1.md)

### Exercice 2 — Ce que le mot sait *(explain-in-prose)*

Un remplissage rapide qui ment sur ses pixels est pire qu'un lent, et le
remplissage par mot fait trois affirmations dont il doit l'argument : le
pointeur de mot a le droit d'écrire cette mémoire (alignement), les
octets du mot atterrissent là où les stockages d'octets les mettaient
(ordre des octets), et l'écriture peut passer par un type différent de
celui par lequel le tampon était écrit avant (aliasing). Faites
l'argument — dans vos propres mots, chaque affirmation, ce que le moteur
garantit et ce que la machine garantit — et rendez-le vérifiable : une
sonde qui vérifie chaque affirmation sur le vrai tampon au démarrage
(l'alignement du pointeur, et un clear relu à travers `GetPixel` et
à travers les octets bruts aux coins et au milieu), imprimant ce qu'elle
a vérifié. Dites ensuite ce que vous changeriez pour une machine où l'une
des affirmations ne tient pas.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-100/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 099 — passe 2a : corriger le dessin de la carte](lesson-099-map-draw.md) ·
**Suivante :** [Leçon 101 — passe 3 : le rapport de budget de frames](lesson-101-frame-budget.md) ·
**Étiquette de code :** [`lesson-100`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-100)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-100-clear.md`,
révision `72d9add`.*

<!-- translation-source: book/lessons/part-5/lesson-100-clear.md @ 72d9add -->
