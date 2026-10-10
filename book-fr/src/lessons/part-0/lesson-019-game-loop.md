# Leçon 019 — la boucle de jeu

{{#include ../../stability-horizon.md}}

## Prose

Cette leçon démarre le dernier programme de la partie 0 : `snek`, un jeu de
serpent en terminal qui grandit leçon après leçon jusqu'à être, à la fin de la
partie, un vrai jeu dans un seul fichier C. Aujourd'hui il n'est rien d'autre
que le squelette que chaque jeu du monde possède — la boucle de jeu — et la
machinerie qui rend une boucle de jeu testable. L'enjeu n'est pas le serpent.
L'enjeu est la forme de boucle que vous rencontrerez à nouveau dans chaque
moteur, y compris celui que ce cours s'apprête à construire.

Un jeu n'est pas un script qui démarre, fait une chose, et sort. C'est une
boucle qui tourne jusqu'à ce que le joueur parte : elle lit ce que le joueur a
fait, avance le monde d'un pas, dessine le résultat, et recommence. Les
bibliothèques de jeux Python et Ruby emballent cela dans un appel `run()` ou un
framework ; les jeux C l'épellent. La voici, dans `main` :
`while (running) { ProcessInput(); Update(); Render(); }`. Trois phases, dans
cet ordre, une fois par tour de boucle. Un tour est une *frame*, et le
compteur `frame` la nomme.

L'ordre n'est pas de la décoration. `ProcessInput` collecte ce qui s'est
passé — les frappes, et éventuellement plus — et rien d'autre. `Update`
avance la simulation : positions, scores, collisions, vies. `Render` lit
l'état courant et le transforme en quelque chose de visible, et *seulement*
cela. Deux règles suivent, et chaque moteur les applique : **update ne dessine
jamais**, et **render n'avance jamais l'état**. Cassez la première et la
vitesse du jeu dépend du taux de rafraîchissement ; cassez la seconde et une
frame peut faire avancer le monde deux fois — dans ce programme, le symptôme
serait un compteur de frames rapportant plus de mises à jour que la boucle n'a
tourné. Garder les phases séparées garde aussi la boucle testable : le
programme d'aujourd'hui imprime son état par frame au lieu de dessiner quoi que
ce soit, et vous pouvez prédire sa sortie exacte avant de l'exécuter.

L'état dans un jeu Python vit dans des attributs d'objets ; ici il vit dans
des variables de portée fichier. Le mot-clé `static` donne aux trois phases un
accès partagé à `running`, `frame` et `max_frames` tout en gardant ces noms
hors de tout autre fichier — et dans un programme d'un seul fichier, il se lit
simplement « ceci est l'état du jeu ». `running` est la condition de la
boucle : un simple drapeau `int` que les phases consultent. `Update` le met à
zéro quand le budget de frames est épuisé, ce qui est aussi la seule sortie de
la boucle — pour l'instant il n'y a pas de clavier pour quitter, aussi le
programme doit-il être prévenu de combien de temps vivre.

Ce budget vient de la ligne de commande : `./snek FRAMES`. Les vrais jeux
tournent pour toujours, ce qui les rend pénibles à tester ; borner le nombre de
frames transforme la boucle en quelque chose qu'un terminal peut exercer de
façon non interactive — chaque exécution de `./snek 3` est les mêmes trois
frames, et sa sortie est vérifiable. Le parsing est `strtoul`, le frère non
signé d'`atoi`, et il vaut son argument supplémentaire : `strtoul` convertit
les chiffres de tête de la chaîne et met `end` sur le premier caractère qu'il n'a
*pas* consommé, aussi le test `*end != '\0'` rejette-t-il des chaînes comme
`12x` qu'un simple `atoi` tronquerait discrètement à `12`. Un compte de zéro
frame est rejeté aussi : la boucle doit tourner au moins une fois. (Il y a une
famille de chaînes qui glisse à travers cette validation et fait paraître le
programme bloqué — l'exercice 2 l'attend.)

Une décision de flux à remarquer : la trace par frame va sur **stderr**, et la
ligne finale `done after N frames` aussi. stdout reste vide exprès. La leçon
001 a séparé les résultats des diagnostics, et la séparation est sur le point
de compter : à partir de la leçon 022, stdout porte l'affichage du jeu — la
frame que le terminal montre — tandis que la trace est *à propos* de
l'exécution, le genre de choses que vous voulez visibles en test et invisibles
en redirection. `./snek 3 2>/dev/null` ne doit donc rien imprimer du tout.

Enfin, la commande qui construit tout cela, depuis l'intérieur de
`sandbox/snek/` :

```
gcc -std=c11 -O0 -g -Wall -Wextra snek.c -o snek
```

Les options sont celles que vous connaissez déjà : `-std=c11` épingle la
version du langage, `-Wall -Wextra` activent les avertissements (toujours du
cours, toujours pris au sérieux), `-O0 -g` gardent le binaire débogable pour
`gdb`, et `-o snek` nomme le programme. Exécutez `./snek 3` et vous devriez
voir trois lignes de frame et le résumé de sortie.

## Étape de code

Un seul changement pour cette leçon : la totalité de `snek.c`, commitée avec ce
texte. Son état final est étiqueté `lesson-019`.

```diff
diff --git a/sandbox/snek/snek.c b/sandbox/snek/snek.c
new file mode 100644
index 0000000..a5bb4a5
--- /dev/null
+++ b/sandbox/snek/snek.c
@@ -0,0 +1,54 @@
+// snek.c — a terminal snake game, grown lesson by lesson.
+//
+// Lesson 019: the game loop — ProcessInput, Update, Render, frame by frame.
+#include <errno.h>
+#include <stdio.h>
+#include <stdlib.h>
+
+static int running;              /* the loop runs while this is true */
+static unsigned long frame;      /* frames since the loop started */
+static unsigned long max_frames; /* stop after this many frames */
+
+static void ProcessInput(void)
+{
+    // No keyboard yet — lesson 021 teaches the terminal.
+}
+
+static void Update(void)
+{
+    ++frame;
+    if (frame >= max_frames)
+        running = 0;
+}
+
+static void Render(void)
+{
+    fprintf(stderr, "frame=%lu\n", frame);
+}
+
+int main(int argc, char **argv)
+{
+    if (argc != 2) {
+        fprintf(stderr, "usage: %s FRAMES\n", argv[0]);
+        return 1;
+    }
+
+    char *end;
+    errno = 0;
+    max_frames = strtoul(argv[1], &end, 10);
+    if (*end != '\0' || max_frames == 0) {
+        fprintf(stderr, "%s: FRAMES must be a positive integer, got '%s'\n",
+                argv[0], argv[1]);
+        return 1;
+    }
+
+    running = 1;
+    while (running) {
+        ProcessInput();
+        Update();
+        Render();
+    }
+
+    fprintf(stderr, "done after %lu frames\n", frame);
+    return 0;
+}
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La vie de `strtoul` *(predict-the-output)*

Prédisez la sortie exacte de chacune de ces exécutions — toute, lignes
d'erreur comprises :

```
./snek 3
./snek 0
./snek abc
./snek 12x
./snek
```

Puis lancez chacune et accordez chaque différence entre prédiction et réalité.
Pour vérifier le *mécanisme*, ajoutez une ligne d'instrumentation juste après
l'appel à `strtoul` — `fprintf(stderr, "parsed=%lu end='%s'\n", max_frames, end);`
suffit — relancez les cinq cas, et expliquez chaque résultat en termes d'où
pointe `end` et de ce que voit le test de validation.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-019/ex1.md)

### Exercice 2 — La chaîne qui ne finit jamais *(fix-the-crash)*

`./snek -5` et `./snek 99999999999999999999` passent tous deux la validation
puis impriment des lignes de frame pour toujours — le programme paraît bloqué.
Découvrez ce que `strtoul` renvoie réellement pour chacune de ces chaînes et
pourquoi `*end != '\0'` ne peut pas le voir, puis corrigez le parsing pour que
les deux soient rejetées avec le message d'erreur habituel du programme. Un
vrai rejet, pas un plafond sur le compte de frames.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-019/ex2.md)

### Exercice 3 — Emballez l'état *(extend-the-code)*

Les trois phases atteignent leur état à travers des variables de portée
fichier. Mettez `running`, `frame` et `max_frames` dans un `struct Game`, et
donnez à chaque phase un paramètre `struct Game *g` ; `main` déclare une
instance et passe son adresse. Aucun comportement ne change — `./snek 3` doit
produire la même sortie qu'avant. C'est l'idiome C derrière les classes C++ qui
arrivent à la fin de la partie 0 : la structure est l'objet, les phases sont
ses méthodes, et le pointeur est `this`.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-019/ex3.md)

### Exercice 4 — Pourquoi trois phases *(explain-in-prose)*

Expliquez en prose pourquoi une boucle de jeu est
`ProcessInput(); Update(); Render();` et pas un autre ordre ni un autre
groupement : ce que chaque phase possède, pourquoi `Update` ne doit pas
dessiner et `Render` ne doit pas avancer l'état, et ce que la trace montrerait
si `Render` était appelé deux fois par tour. Avant d'écrire, faites en sorte
que le programme soit d'accord avec votre explication : ajoutez un `fprintf` en
tête de chaque phase imprimant le nom de la phase, exécutez `./snek 2`, et
vérifiez que l'ordre que vous voyez est l'ordre que vous vous apprêtez à
défendre.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-019/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 018 — l'optimiseur et le comportement indéfini](lesson-018-optimizer-ub.md) ·
**Suivante :** [Leçon 020 — la mesure du temps avec `clock_gettime`](lesson-020-timing.md) ·
**Étiquette de code :** [`lesson-019`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-019)

*Page traduite de la version anglaise `book/lessons/part-0/lesson-019-game-loop.md`,
révision `6f5430b`.*

<!-- translation-source: book/lessons/part-0/lesson-019-game-loop.md @ 6f5430b -->
