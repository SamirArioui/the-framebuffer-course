# Leçon 058 — la table du budget de frames

{{#include ../../stability-horizon.md}}

## Prose

L'instrumentation de la leçon 036 a été plantée exactement pour ça : la **table
du budget de frames** — le cumul attribué par sous-système, imprimé comme le
rapport que la clôture de la partie 5 fera grandir. C'est ce qu'O1 rapporte, et
la dernière leçon de la partie : pas une nouvelle capacité, mais la *reddition
de comptes* — le coût de la frame nommé, ligne par ligne, à partir de sommes
mesurées de vraies frames. Chaque nombre de la table a été payé par une frame
qui a réellement tourné.

### La table

La démo tourne, et à la fin le cumul s'imprime en budget :

```
engine: frame budget — 27 frames, avg 2.570 ms, worst 4.162 ms (frame 17)
engine:   subsystem   avg ms    share
engine:   update       0.023       1%
engine:   render       1.917      75%
engine:     sprites    0.001       0%
engine:     text       0.008       0%
engine:     tilemap    1.202      47%
engine:   present      0.630      25%
engine:   total        2.570     100%
```

L'en-tête est le plancher de la spécification, à dessein : **combien de
frames** ont été mesurées, **ce qu'elles coûtent en moyenne**, et **la pire par
numéro** — les trois faits par lesquels s'ouvre toute discussion de budget.
Chaque ligne en dessous est une somme mesurée divisée par le nombre de frames :

- **update, render, present** — les trois phases nommées par la leçon 036,
  exactement comme son enregistrement les mesure. Rien n'a changé dans les
  définitions de phases pour fabriquer la table ; la table est la raison d'être
  de l'enregistrement.
- **les lignes en retrait** — les sous-systèmes nommés de la phase render
  (leçons 046, 051, 053) : ce que coûtent le dessin du sprite, le texte et le
  parcours du tilemap, chacun, *à l'intérieur* du render. Elles expliquent la
  ligne au-dessus d'elles ; elles ne la remplacent pas.
- **les parts** — chaque ligne en pourcentage de la frame moyenne.

Chaque nombre est vérifiable contre le journal de l'exécution elle-même — et la
vérification de rédaction de cette leçon a fait exactement cela : la moyenne des
lignes `frame N:` sur la session reproduit la table ligne pour ligne (update
0.023, render 1.917, sprites 0.001, text 0.008, tilemap 1.202, present 0.630,
total 2.570). Un budget dont les nombres sont en désaccord avec son propre
registre est une décoration ; celui-ci est le registre.

### Ce que dit la table

Trois lectures, toutes déjà visibles dans des leçons antérieures — le travail
de la table est de les rendre impossibles à oublier :

1. **Le parcours du tilemap possède le render.** 1,2 ms des 1,9 ms de render,
   c'est 1 536 blits qui redessinent le monde à chaque frame (la leçon 053 a
   mesuré le coût du parcours à trois emplacements ; voici celui à l'écran). Si
   une scène doit un jour aller plus vite, c'est cette ligne qui paiera.
2. **La présentation est la ligne de la machine.** Les 25 % d'ici sont la copie
   de Xvfb ; sur un bureau, elle bouge avec le pilote et le compositeur (la
   leçon 036 de la partie 1 avait trouvé la même forme). C'est la ligne la
   moins contrôlée par le moteur et la plus contrôlée par la plateforme.
3. **Dessiner un sprite est gratuit ; dessiner le monde ne l'est pas.** sprites
   et text ensemble font 0,009 ms — sous le plancher de bruit. Le coût par
   objet du moteur de rendu n'est pas le problème à cette échelle ; le *nombre
   d'objets* (toute la carte, à chaque frame) l'est.

Et l'arithmétique de la frame se referme : `0.023 + 1.917 + 0.630 = 2.570` —
les trois phases rendent compte du total (le minuscule résidu, c'est la mesure
elle-même : l'horloge lit autour des phases). Les lignes de sous-système ne se
referment pas encore sur le render — la différence, c'est l'effacement et la
comptabilité des phases, et l'exercice 1 met un nombre dessus.

### Le format que la partie 5 fait grandir

Cette table est explicitement un *premier jet*. Ce que le rapport de budget de
frames de la clôture ajoute, et ce pour quoi ce format a déjà de la place :

- **plus de lignes à mesure que des sous-systèmes existent** — le son, les
  entités, le propre travail d'update du jeu (les parties 3-5 ajoutent leurs
  phases nommées exactement comme l'ont fait text et tilemap) ;
- **des colonnes avant/après** — le menu d'optimisation en trois passes
  (mesurer, corriger les 2 premiers, rapporter) rapporte chaque passe contre
  cette même table ;
- **la machine** — le rapport nomme son matériel et ses options de compilation,
  la règle que porte chaque mesure de ce cours depuis la partie 0.

Ce que le format ne changera *pas* : les lignes sont des sommes mesurées, les
parts sont celles de la frame moyenne, l'en-tête porte le compte, la moyenne et
la pire, et les phases nommées vivent à l'intérieur de leur phase au lieu de la
remplacer. Voilà les règles qui font que les nombres veulent dire ce qu'ils
disent.

### O1, livrée

L'obligation d'instrumentation du MVD est complète et démontrable : un
enregistrement par frame, une ligne de journal par enregistrement, un cumul qui
résume l'exécution, et maintenant le budget attribué par sous-système — le tout
sur l'horloge de la plateforme derrière la couture, le tout vérifiable contre
le journal. La leçon de profilage de la partie 5 part de cette table et ne peut
pas commencer sans elle.

## Étape de code

Un seul changement pour cette leçon : `frame.h` / `frame.cpp` accueillent
`PrintFrameBudget` — le cumul imprimé sous forme de table (le compte, la
moyenne et la pire de l'en-tête, et les sommes mesurées et parts des lignes) —
et le bloc de cumul de `main.cpp` devient l'appel. La boucle de démo, le monde
et le chemin de dessin ne sont pas touchés. Son état final est étiqueté
`lesson-058`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index 22cb968..3f9f2dd 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -4,6 +4,8 @@
 
 #include "frame.h"
 
+#include <cstdio>
+
 namespace engine {
 
 void AccountFrame(FrameStats &stats, const FrameRecord &frame)
@@ -22,4 +24,42 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     }
 }
 
+void PrintFrameBudget(const FrameStats &stats)
+{
+    if (!stats.frames)
+        return;
+    double n = (double)stats.frames;
+    double avg = stats.total_sum / n;
+
+    /* The account the spec requires: how many frames, what they cost on
+       average, and the worst one by number. */
+    std::printf("engine: frame budget — %ld frames, avg %.3f ms, worst %.3f ms (frame %ld)\n",
+                stats.frames, avg * 1e3, stats.worst * 1e3,
+                stats.worst_number);
+
+    /* The attribution: every row a measured sum, every share of the
+       average frame. The named phases live inside render — they say
+       where it went, they do not replace it. */
+    double update = stats.update_sum / n * 1e3;
+    double render = stats.render_sum / n * 1e3;
+    double sprites = stats.sprites_sum / n * 1e3;
+    double text = stats.text_sum / n * 1e3;
+    double tilemap = stats.tilemap_sum / n * 1e3;
+    double present = stats.present_sum / n * 1e3;
+    std::printf("engine:   subsystem   avg ms    share\n");
+    std::printf("engine:   update      %6.3f      %2.0f%%\n", update,
+                100.0 * update / (avg * 1e3));
+    std::printf("engine:   render      %6.3f      %2.0f%%\n", render,
+                100.0 * render / (avg * 1e3));
+    std::printf("engine:     sprites   %6.3f      %2.0f%%\n", sprites,
+                100.0 * sprites / (avg * 1e3));
+    std::printf("engine:     text      %6.3f      %2.0f%%\n", text,
+                100.0 * text / (avg * 1e3));
+    std::printf("engine:     tilemap   %6.3f      %2.0f%%\n", tilemap,
+                100.0 * tilemap / (avg * 1e3));
+    std::printf("engine:   present     %6.3f      %2.0f%%\n", present,
+                100.0 * present / (avg * 1e3));
+    std::printf("engine:   total       %6.3f     100%%\n", avg * 1e3);
+}
+
 } /* namespace engine */
diff --git a/src/frame.h b/src/frame.h
index 8f42a7d..8a998b4 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -43,6 +43,12 @@ struct FrameStats {
 
 void AccountFrame(FrameStats &stats, const FrameRecord &frame);
 
+/* Lesson 058: the frame-budget table — the account, attributed per
+   subsystem, as the report Part 5's finale grows. Every number in it is
+   a measured sum from the frames that actually ran; the shares are of
+   the average frame. */
+void PrintFrameBudget(const FrameStats &stats);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index bc94247..95f09af 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -275,19 +275,10 @@ int Run(void)
                     frame.total * 1e3);
     }
 
-    /* The account: what the frames actually cost, subsystem by subsystem —
-       the frame-budget table's first data (lesson 058 prints the table). */
-    if (stats.frames) {
-        double n = (double)stats.frames;
-        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, text %.3f, tilemap %.3f, present %.3f)\n",
-                    stats.frames, stats.total_sum / n * 1e3,
-                    stats.update_sum / n * 1e3, stats.render_sum / n * 1e3,
-                    stats.sprites_sum / n * 1e3, stats.text_sum / n * 1e3,
-                    stats.tilemap_sum / n * 1e3, stats.present_sum / n * 1e3);
-        std::printf("engine: worst frame %.3f ms (frame %ld); present is %.0f%% of the frame\n",
-                    stats.worst * 1e3, stats.worst_number,
-                    100.0 * stats.present_sum / stats.total_sum);
-    }
+    /* The account as the frame-budget table (lesson 058): the frame
+       count, the average, the worst frame — and the render attributed to
+       its subsystems, the report Part 5's finale grows. */
+    PrintFrameBudget(stats);
     std::printf("engine: arena: %zu of %zu bytes used\n", arena.used,
                 arena.memory.size);
 
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — La ligne qui n'est pas là *(extend-the-code)*

Les lignes de sous-système ne se recoupent pas avec la ligne render — et la
différence a un nom et une taille. Ajoutez la ligne manquante (`render` moins
les sous-systèmes nommés) à la table, lancez la démo et réconciliez
l'arithmétique. Répondez ensuite à la question que pose la ligne : la
différence est surtout le `ClearBuffer` de la frame — pourquoi le plus gros
coût pixel par frame n'est-il pas un sous-système nommé, et devrait-il l'être ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-058/ex1.md)

### Exercice 2 — La colonne de la pire frame *(predict-the-output)*

L'en-tête nomme la pire frame ; la table ne dit pas à quoi elle a passé son
temps. Avant de lancer quoi que ce soit, trouvez la pire frame dans les lignes
`frame N:` du journal et prédisez quel sous-système la possède — la pire frame
est-elle un événement de *rendu* ou autre chose ? Gardez ensuite
l'enregistrement de la pire frame et imprimez son attribution à côté de la
table ; réconciliez, et expliquez pourquoi la forme de la pire frame peut
différer de celle de la moyenne.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-058/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 057 — la démo de clôture : le monde, dessiné](lesson-057-demo.md) ·
**Suivante :** — ·
**Étiquette de code :** [`lesson-058`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-058)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-058-budget.md`,
révision `b62f67e`.*

<!-- translation-source: book/lessons/part-2/lesson-058-budget.md @ b62f67e -->
