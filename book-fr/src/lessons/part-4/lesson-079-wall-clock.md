# Leçon 079 — la mesure n'est pas mise à l'échelle

{{#include ../../stability-horizon.md}}

## Prose

La leçon 078 a donné au jeu un bouton et à l'update un pas mis à l'échelle.
Elle a aussi soulevé une question qu'elle a délibérément laissée ouverte :
qu'advient-il de tout ce qui *entoure* ce pas quand le bouton est à zéro ? Si
l'enregistrement de frame d'un jeu en pause disait `total 0.000 ms`, le budget
de frames rapporterait qu'un jeu en pause ne coûte rien — un mensonge contre
lequel la machine peut mesurer. L'idée de cette leçon est donc la discipline
qui garde l'enregistrement honnête : **l'échelle atteint le pas de la
simulation et rien d'autre**. L'horloge de la plateforme reste celle qui
mesure, et chaque phase de l'enregistrement de frame est une durée d'horloge
murale à toute échelle.

### L'unique non-durée de l'enregistrement

`FrameRecord` accueille un champ, et ce champ n'est délibérément pas une
phase :

```cpp
    /* ... every field above is wall-clock, at any scale ... */
    double step; /* the game-time step this frame advanced by */
```

`step` est le nombre que la marche a multiplié dans le mouvement — des secondes
de jeu, pas du temps machine. Il figure dans l'enregistrement parce que
l'avance du jeu est un fait dont le lecteur du journal a besoin (cette frame
était-elle en pause ? combien de temps de monde s'est écoulé ?), et il est
*hors* des phases parce qu'il répond à une autre question. Les phases disent ce
que la machine a fait ; le pas dit ce que le jeu a fait. L'ajouter à
l'enregistrement est ce qui rend la différence vérifiable en une ligne plutôt
que disputée.

### La ligne de la frame en pause

D'une vraie exécution de l'état final de cette leçon, la première frame que le
script de la démo a mise en pause :

```
frame 50: step 0.000 ms, update 0.020 ms, audio 0.000 ms, render 1.333 ms (sprites 0.006, text 0.007, tilemap 0.922), present 0.295 ms, total 1.649 ms
```

Lisez-la de gauche à droite. Le **pas est 0.000** — la simulation n'a pas
avancé d'une microseconde ; le héros est resté immobile, et la marche a
multiplié sa requête par exactement rien. Et ensuite les phases, sans rien de
remarquable : `update 0.020 ms`, `render 1.333 ms`, `present 0.295 ms`,
`total 1.649 ms` — des frames ordinaires, facturées comme la leçon 036 les a
toujours facturées, sur l'horloge de la plateforme. La pause a coûté à cette
machine 1,6 milliseconde de vrai travail : l'entrée a été lue, huit entités ont
été parcourues, toute la scène a été dessinée et copiée vers la fenêtre. Le jeu
était immobile ; la machine ne l'était pas.

Les frames d'à côté rendent la règle vivante. Pendant le hitstop, le pas se lit
`3.809 ms` — un quart du `15.2 ms` du pas mural, exactement le produit de la
leçon 078 — tandis que les phases lisent `1.267 ms` de render et `1.647 ms` de
total, la même forme que les frames de jeu autour d'elles. À pleine vitesse, le
pas est `15.2 ms`, le pas mural entier. Trois échelles, une mesure.

### Pourquoi l'enregistrement ne doit pas suivre le jeu

L'enregistrement de frame n'est pas un système de jeu ; c'est un instrument.
Toute sa valeur depuis la leçon 036 est que ses nombres sont *ceux de la
machine*, pris sur l'unique horloge que possède la couture (la leçon 035 :
monotone, fine, l'horloge murale ne peut pas la déplacer). Deux consommateurs
en dépendent et tous deux casseraient autrement :

- **La table du budget de frames** (leçon 058) attribue le coût réel par
  sous-système. Un enregistrement mis à l'échelle ferait passer le jeu le moins
  cher possible — un jeu en pause — pour le plus rapide : zéro partout, et une
  table de budget qui dit que le moteur ne coûte rien quand le jeu s'arrête. La
  vérité est l'inverse et plus utile : *mettre la simulation en pause est
  gratuit pour le jeu et pas pour la machine*, et la différence est le coût de
  la présentation — la chose dont un écran de pause est fait.
- **L'attribution que la leçon 081 ajoute** (le travail d'entités de l'update)
  est une part de temps réel. Si les durées de l'update se mettaient à
  l'échelle avec le temps de jeu, un hitstop ferait paraître le travail
  d'entités bon marché exactement quand il y en a le plus à l'écran.

Et le contrat de la couture est le troisième consommateur, celui qui n'a jamais
signé pour rien de tout ça : `platform::Now()` est celle qui mesure. Si
l'échelle atteignait l'horloge, chaque calendrier bâti dessus — l'horizon du
tampon de l'étape audio, les horodatages de l'exécution elle-même — hériterait
du temps de jeu sans l'avoir demandé. La réponse du moteur est la forme que la
leçon 078 a introduite et que celle-ci garde : l'horloge mesure, le pas se met
à l'échelle, et les deux ne se rencontrent jamais.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

D'une vraie exécution de l'état final de cette leçon sur cette machine, pilotée
par une entrée scriptée pendant que le script de la démo tournait le bouton à
trois, cinq et sept secondes :

- **Les phases de l'enregistrement de frame sont de l'horloge murale à
  l'échelle 0.** Les frames en pause ci-dessus : `step 0.000 ms` avec
  `update 0.020`, `render 1.333`, `present 0.295`, `total 1.649` — chaque phase
  une durée réelle, et l'arithmétique du journal de frames se referme encore
  (`update + audio + render + present ≈ total`).
- **Le pas est celui du jeu, à toute échelle.** Les frames du hitstop lisent
  `step 3.809 ms` contre des pas muraux de `15.2 ms` — le quart, réconcilié —
  et celles du jeu lisent le pas mural entier.
- **L'échelle n'atteint pas la couture.** Les horodatages de l'exécution
  (`t=…` sur les rapports du héros et de la caméra) sont de l'horloge murale à
  travers la fenêtre de pause — le script de la démo a tourné le bouton à
  trois, cinq et sept secondes *murales* — et le calendrier de l'étape audio
  n'a pas été touché.

Ce que cette leçon ne fait **pas**, c'est faire apparaître le pas dans la table
du budget : la table est une table de durées et `step` n'en est pas une. La
leçon 081 fait grandir la table avec l'attribution d'entités de l'update — une
durée mesurée, comme chaque ligne avant elle.

## Étape de code

Un seul changement pour cette leçon, de l'argument à la preuve : `src/frame.h`
accueille `FrameRecord.step` — l'avance en temps de jeu, documentée comme
l'unique non-durée de l'enregistrement — et `src/main.cpp` l'enregistre là où
l'update le calcule et l'imprime en premier dans le journal de frames, l'avance
du jeu à côté des durées de la machine. Le service de l'échelle, la marche, le
mover, le magasin, le son et la table du budget sont intacts. Son état final
est étiqueté `lesson-079`.

```diff
diff --git a/src/frame.h b/src/frame.h
index f4b45e3..45d4d88 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -27,6 +27,13 @@ struct FrameRecord {
     double sprites; /* sprite draws through the blit */
     double text;    /* lesson 051: text drawing — glyphs through the blit */
     double tilemap; /* lesson 053: the map's walk — tiles through the blit */
+
+    /* Lesson 079: the game-time step this frame advanced the simulation
+       by — not a duration. Every field above is wall-clock, at any
+       scale: the measurement is the machine's, not the game's. This one
+       is where game time is visible, so a paused frame reads `step
+       0.000 ms` beside wall-clock phases that took what they took. */
+    double step;
 };
 
 /* The running account: every frame measured so far. */
diff --git a/src/main.cpp b/src/main.cpp
index ee2f1b4..1904f32 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -413,8 +413,11 @@ int Run(void)
 
         /* Lesson 078: the update advances by game time — the wall
            clock's step, scaled. Everything the simulation does with dt
-           is scaled; nothing else is. */
+           is scaled; nothing else is. Lesson 079: the step is recorded
+           beside the phases — the one field in the record that is game
+           time, and the rest are wall clock at any scale. */
         double dt = GameTimeStep(game_time, wall_dt);
+        frame.step = dt;
 
         /* Lesson 076: the hero's intent — polled input state, read once
            per frame and written to the hero's own movement request. The
@@ -672,9 +675,12 @@ int Run(void)
 
         /* The frame log: one line per record — the format grows its named
            fields, one per subsystem, as the parts name them. The audio
-           phase (lesson 060) joins in the record's own order. */
-        std::printf("frame %ld: update %.3f ms, audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
-                    frame.number, frame.update * 1e3, frame.audio * 1e3,
+           phase (lesson 060) joins in the record's own order, and
+           lesson 079's step leads it: the game's advance beside the
+           machine's durations. */
+        std::printf("frame %ld: step %.3f ms, update %.3f ms, audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
+                    frame.number, frame.step * 1e3, frame.update * 1e3,
+                    frame.audio * 1e3,
                     frame.render * 1e3,
                     frame.sprites * 1e3, frame.text * 1e3,
                     frame.tilemap * 1e3, frame.present * 1e3,
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La frame en pause, prédite *(predict-the-output)*

Avant de lancer quoi que ce soit, notez par écrit la ligne du journal de frames
pour une frame qui tourne pendant que l'échelle est à 0 — chaque champ, autant
des vrais nombres que vous pouvez prédire et la *forme* du reste — puis notez à
quoi ressembleraient les lignes de la table du budget de frames après une
exécution qui se termine pendant la pause, comparée à la même exécution à
pleine vitesse. Quels nombres bougent entre les deux exécutions et lesquels ne
bougent pas ? Lancez les deux (terminer une exécution en pleine pause est une
question de fermer la fenêtre à la bonne seconde) et réconciliez — puis dites
en une phrase ce que la table de l'exécution en pause prouve sur la différence
entre le jeu et la machine.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-079/ex1.md)

### Exercice 2 — Ce que coûte un jeu en pause *(measure-the-performance)*

« La pause est gratuite » est une affirmation, et cette leçon dit qu'elle est
fausse. Mesurez-la : comptabilisez les frames de chaque fenêtre d'échelle et
leur coût en horloge murale, et rapportez la frame moyenne en jeu, en hitstop
et en pause — sur votre machine, avec vos nombres. Où passe le temps de la
frame en pause (les phases de l'enregistrement vous le diront), et que
coûterait à un jeu d'*arrêter aussi* la présentation pendant la pause ?
Répondez ensuite à la question de design que les nombres soulèvent : si un
écran de pause veut être moins cher, que doit-il changer — l'échelle, le
dessin, ou le calendrier de la machine ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-079/ex2.md)

---

**Partie :** [Partie 4 — les services](../../index.md) ·
**Précédente :** [Leçon 078 — l'échelle du temps de jeu](lesson-078-game-time.md) ·
**Suivante :** [Leçon 080 — la tranche verticale](lesson-080-slice.md) ·
**Étiquette de code :** [`lesson-079`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-079)

*Page traduite de la version anglaise `book/lessons/part-4/lesson-079-wall-clock.md`,
révision `4eaa176`.*

<!-- translation-source: book/lessons/part-4/lesson-079-wall-clock.md @ 4eaa176 -->
