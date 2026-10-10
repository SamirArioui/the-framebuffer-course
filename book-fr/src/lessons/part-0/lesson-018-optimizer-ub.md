# Leçon 018 — l'optimiseur et le comportement indéfini

{{#include ../../stability-horizon.md}}

## Prose

La leçon 017 a livré une boucle de remplissage écrite naïvement exprès et a
promis que cette leçon découvrirait ce que l'optimiseur en fait. La facture
arrive aujourd'hui. C'est la leçon la plus importante de la seconde moitié de
la partie 0 : **un programme qui fonctionne à `-O0` n'est pas un programme
correct**, et l'écart entre « fonctionne » et « correct » est le comportement
indéfini.

D'abord, la nouvelle option de construction. `-O2` veut dire « optimiser le
code agressivement » : le compilateur réordonne, inline, élimine, et — le but
d'aujourd'hui — *prouve des choses* sur votre programme et agit sur ces
preuves :

```
gcc -std=c11 -O2 -g -Wall -Wextra paint.c -o paint
```

Construisez ainsi le programme de la leçon 017 et gcc s'objecte avant même que
vous ne l'exécutiez :

```
paint.c: In function ‘ClearBuffer’:
paint.c:165:10: warning: iteration 2147483647 invokes undefined behavior [-Waggressive-loop-optimizations]
  165 |         i++;
      |         ~^~
paint.c:162:14: note: within this loop
  162 |     while (i >= 0) {          /* keep going while the offset is positive */
      |            ~~^~
```

Voilà le compilateur qui pointe la boucle exacte que la leçon 017 a signalée :
le remplissage qui ne s'arrête que quand son compteur devient négatif. Lancez
le binaire `-O2` comme la leçon 017 le lançait — avec `timeout`, qui exécute
une commande avec un délai et la tue si le délai expire (son code de sortie 124
veut dire « le temps était écoulé ») :

```
$ timeout 5 ./paint
$ echo $?
124
```

Pas de sortie, pas de fichier, rien : le programme est entré dans `ClearBuffer`
et n'en est jamais sorti. La construction `-O0` du *même source* tourne bien et
imprime `clear ended at offset -2147483648`. Le source n'a pas changé. La
machine n'a pas changé. Une option du compilateur a changé ce que le programme
*est*.

Voici pourquoi. Le débordement d'entier signé en C est un **comportement
indéfini** : quand `i++` dépasserait `INT_MAX`, la norme n'impose *aucune
exigence du tout* sur ce qui arrive ensuite. Pas « ça retombe » — rien. Le
corollaire critique est ce que cela autorise le compilateur à faire : si le
débordement n'a jamais de conséquences définies, alors **le compilateur peut
supposer qu'il n'arrive jamais**, et réécrire le programme sous cette
hypothèse. gcc regarde `while (i >= 0) { … i++; }` et raisonne : `i` commence
à 0 et ne fait qu'augmenter, aussi `i` ne peut cesser d'être non négatif
qu'en débordant — ce qui ne peut pas arriver — donc *la condition de boucle est
toujours vraie* — donc la sortie est inatteignable — donc elle peut être
supprimée. La boucle est compilée comme infinie, ce qui est exactement le
blocage que vous avez mesuré. Le comportement indéfini ne veut pas dire « le
programme plante » ni « le programme retombe » ; il veut dire que le
compilateur peut faire quoi que ce soit — y compris supprimer votre code — et
avoir raison par les règles de la norme. (L'exercice 2 montre que même une
garde explicite contre le retournement est supprimée sur le même raisonnement.)

Pourquoi `-O0` avait-il l'air bien ? De la chance. À `-O0` le compilateur émet
de l'arithmétique machine simple, le matériel fait retomber `i` sur `INT_MIN`,
et cette valeur négative retombée fournit accidentellement la sortie dont la
boucle avait besoin. Le retournement est un fait sur l'arithmétique x86, pas
sur le C — le langage ne l'a jamais promis, aussi le programme était-il cassé
depuis le moment où il a été écrit ; `-O0` l'a seulement caché. C'est
l'habitude que cette leçon existe pour construire : quand le comportement
diffère entre niveaux d'optimisation, le bug n'est *jamais* l'optimiseur.

La correction est petite et totale : donnez à la boucle une vraie borne.
`ClearBuffer` devient `for (int i = 0; i < nbytes; i++)` — la boucle se
termine désormais parce que `i` atteint `nbytes`, ce qui est un comportement
défini à chaque niveau d'optimisation. La preuve est dans les exécutions :
construisez avec les deux commandes, lancez les deux, et les sorties sont
identiques (`clear covered 144 bytes` … `wrote paint.bmp (8x6, 24 bpp)`), et
les deux constructions écrivent le même `paint.bmp` valide — `file` rapporte
toujours `PC bitmap, Windows 3.x format, 8 x 6 x 24 … cbSize 198`. Aucun
avertissement, à aucun niveau.

Retirez-en trois règles générales. **Une :** le comportement indéfini n'est pas
un rare plantage ; c'est un contrat caduc — le compilateur a le droit de
supposer que votre pire cas ne peut pas survenir et d'optimiser en
conséquence. **Deux :** les gardes que vous écrivez contre les cas « impossibles
» sont supprimées avec l'impossibilité ; la seule vraie correction est du code
dont le comportement défini couvre chaque entrée. **Trois :** traitez chaque
avertissement sur le « comportement indéfini » comme un rapport de bug, pas
une note de style — le `-Waggressive-loop-optimizations` de gcc a attrapé
celui-ci au moment de la construction avant que le blocage ne tourne jamais.
La vitesse n'est *pas* le sujet d'aujourd'hui : `-O2` est ici comme
instrument de correction, et l'optimisation pour la performance arrive bien
plus tard dans le cours, avec les bons outils pour cela.

Commande de construction pour le programme corrigé — celui que le reste du
cours suppose :

```
gcc -std=c11 -O0 -g -Wall -Wextra paint.c -o paint
```

## Étape de code

Un seul changement pour cette leçon : `ClearBuffer` gagne une vraie borne de
boucle, et le remplissage naïf disparaît. Commité avec ce texte ; son état
final est étiqueté `lesson-018`.

```diff
diff --git a/sandbox/paint/paint.c b/sandbox/paint/paint.c
index 3aeb7bd..c1ef4ec 100644
--- a/sandbox/paint/paint.c
+++ b/sandbox/paint/paint.c
@@ -1,10 +1,8 @@
-// paint.c — Lesson 017: writing a real image file by hand.
+// paint.c — Lesson 018: the optimizer and undefined behavior.
 //
-// A pixel buffer is bytes (lesson 013), a file header is pinned bytes
-// (lesson 014), rectangles fold-clip (lesson 015), lines rasterize
-// (lesson 016).  Now the buffer becomes a real file: WriteBmp emits the
-// 54-byte header plus bottom-up, padded rows — and a small scene lands
-// in paint.bmp.
+// The lesson-017 ClearBuffer relied on signed overflow to stop.  This is
+// the fix: the loop gets a real bound, so -O0 and -O2 agree on what the
+// program does — and both write the same valid BMP.
 #include <stdio.h>
 #include <stdlib.h>
 
@@ -152,19 +150,14 @@ static void DrawLine(unsigned char *px, int w, int h,
     }
 }
 
-// ClearBuffer — fill the buffer with one byte value.  This fill is
-// written naively on purpose: its stop condition is the offset turning
-// negative, a stop that only a wrapping counter can deliver.
-// Lesson 018 finds out what the optimizer does with it.
+// ClearBuffer — fill the buffer with one byte value.  The loop bound is
+// the buffer size: no wraparound, no undefined behavior, so every
+// optimization level does the same thing.
 static void ClearBuffer(unsigned char *px, int nbytes, unsigned char v)
 {
-    int i = 0;
-    while (i >= 0) {          /* keep going while the offset is positive */
-        if (i < nbytes)       /* clip: never write past the buffer */
-            px[i] = v;
-        i++;
-    }
-    printf("clear ended at offset %d\n", i);
+    for (int i = 0; i < nbytes; i++)
+        px[i] = v;
+    printf("clear covered %d bytes\n", nbytes);
 }
 
 // BuildBmpHeader — lay out the 54-byte BMP header field by field.
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Prédisez les dégâts *(predict-the-output)*

Avant d'exécuter une seule commande, écrivez vos prédictions pour le programme
de la leçon 017 construit à `-O2` : (a) les mots exacts de l'avertissement
qu'imprime gcc au moment de la construction (quelle ligne de quelle fonction
incrimine-t-il ?), et (b) pour `timeout 5 ./paint` — qu'imprime l'exécution,
et que vaut `echo $?` ensuite ? Appliquez ensuite le patch de solution de
cette leçon (il remet le remplissage naïf, avec un `fprintf` marqueur à son
entrée pour que vous voyiez jusqu'où l'exécution va), construisez-le des deux
façons, lancez les deux, et accordez chaque prédiction — en particulier
l'apparition du marqueur dans l'exécution bloquée.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-018/ex1.md)

### Exercice 2 — La garde qui est supprimée *(explain-in-prose)*

Une *garde* de retournement semble devoir sauver la boucle naïve : s'arrêter
explicitement quand le décalage devient négatif. Réécrivez `ClearBuffer` en
boucle sans borne — `for (;;) { … i++; if (i < 0) break; }` — en gardant le
`break` explicite comme unique sortie. Prédisez ce que la construction `-O2`
fait désormais de ce `break`, lancez les deux constructions, et expliquez avec
vos propres mots pourquoi la garde ne sauve pas le programme : qu'a exactement
le droit de conclure le compilateur sur `i`, et de quelle autorité ? Puis dites
ce qui l'*aurait* sauvé.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-018/ex2.md)

### Exercice 3 — Relisez le remplissage d'un collègue *(fix-the-crash)*

Un collègue vous envoie son remplissage en dégradé à ajouter à `paint.c`,
« dans le même style que l'ancien vidage » :

```c
static void FillGradient(unsigned char *px, int w, int h)
{
    int i = 0;
    while (i >= 0) {
        if (i < w * h * 3)
            px[i] = (unsigned char)((i / 3) * 255 / (w * h - 1));
        i++;
    }
}
```

Appelé après `ClearBuffer`, il produit la même histoire que la leçon 017 :
bien à `-O0`, infini à `-O2` (confirmez les deux avant de corriger quoi que ce
soit). Corrigez la fonction proprement — une vraie borne, du comportement
défini partout, le dégradé dessiné correctement — ajoutez l'appel à `main`, et
vérifiez que le programme se comporte désormais de façon identique à `-O0` et
`-O2`.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-018/ex3.md)

### Exercice 4 — Prouvez la correction *(extend-the-code)*

L'affirmation de la leçon est que le programme corrigé est ennuyeux : même
comportement à `-O0` et `-O2`, même `paint.bmp` valide des deux. Faites
vérifier à la programme la première moitié de cela lui-même : juste après
`ClearBuffer`, vérifiez que chaque octet du tampon contient la valeur de fond
et imprimez le compte d'octets qui ne la contiennent pas. Faites ensuite le
reste à la main et rapportez : lancez les deux constructions vers deux
fichiers et `diff`-les, et lancez `file` sur le `paint.bmp` de chacune. Que
prouve réellement chaque vérification — et quelle classe de bug le `diff` des
sorties ne peut-il *pas* attraper ?

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-018/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 017 — écrire un vrai fichier image à la main](lesson-017-image-file.md) ·
**Suivante :** <a href="../../../lessons/part-0/lesson-019-game-loop.html">Leçon 019 — la boucle de jeu</a> *(en anglais ; traduction à venir)* ·
**Étiquette de code :** [`lesson-018`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-018)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-018-optimizer-ub.md`, révision `2aa5b9e`.*

<!-- translation-source: book/lessons/part-0/lesson-018-optimizer-ub.md @ 2aa5b9e -->
