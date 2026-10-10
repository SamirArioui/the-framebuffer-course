# Leçon 022 — la grille de caractères à double tampon

{{#include ../../stability-horizon.md}}

## Prose

Le programme sait maintenant ce que le joueur a pressé ; il ne peut toujours
rien montrer. Cette leçon construit l'affichage — et la façon dont il est
construit est l'enjeu. Un jeu qui dessine droit sur l'écran passe chaque frame
à effacer et repeindre tout le monde, et le joueur voit chaque état
intermédiaire comme du scintillement. La correction est le *double tampon* :
composer la frame quelque part d'invisible, puis déplacer seulement ce qui a
changé vers le vrai écran. Chaque moteur fait cela ; le framebuffer du titre
de ce cours est exactement le « quelque part d'invisible ».

La surface invisible de `snek` est un simple tableau `char`
bidimensionnel — le *tampon arrière* — dimensionné `GRID_ROWS` × `GRID_COLS`,
un caractère par cellule. Une grille de terminal est le framebuffer le plus
honnête et le moins technologique qui soit : les pixels sont des octets, et
dessiner est de l'indexation de tableau. `GridClear` efface le tampon arrière
en espaces et `GridPut` stocke un caractère à une cellule (vérifié en limites,
pour qu'une pièce de logique de jeu une cellule hors du bord dégénère en rien
au lieu de corrompre la mémoire). `Render` y compose la scène : une banderole
sur la ligne du haut, le marqueur — le proto-serpent d'une cellule
d'aujourd'hui, `@` — à sa position. Le marqueur bouge d'une cellule par tick
dans la direction courante et revient en boucle aux bords ; ce mouvement,
depuis le pas fixe de la leçon 020, est la première animation du jeu.

Le second tableau est le *tampon avant* : il n'est pas dessiné vers le
terminal au moment du vidage d'une façon astucieuse — c'est simplement une
**affirmation sur ce que le terminal montre actuellement**. `GridFlush`
parcourt les deux tableaux cellule par cellule et n'écrit que les cellules où
l'affirmation contredit la scène. Chaque telle cellule est un échappement de
position de curseur — `ESC [ row ; col H` déplace le curseur vers une cellule
— suivi du caractère lui-même. Quand le parcours ne trouve rien, la frame
coûte zéro octet, et le terminal reste exactement sur ce qu'il montrait déjà :
aucun scintillement n'est possible quand rien n'est effacé.

L'affirmation a besoin d'une exception : avant le premier vidage, `front`
décrit un écran sur lequel le programme n'a jamais dessiné, aussi le drapeau
`front_valid` est-il à zéro et *chaque* cellule compte-t-elle comme changée.
Ce premier vidage nettoie l'écran avec `ESC [ 2 J` et peint toute la grille ;
chaque vidage après est en différentiel seul. Si quoi que ce soit d'autre
gribouille jamais sur le terminal — un message d'erreur, un autre programme —
l'affirmation est périmée et l'affichage corrompu jusqu'à ce que les tampons
soient de nouveau invalidés. Tenir cet invariant honnêtement est l'essentiel de
la discipline du double tampon.

Les octets comptent ici aussi. Le vidage différentiel n'est pas seulement
anti-scintillement ; c'est de la compression. Un redessin complet d'une grille
de 20×40 fait des centaines d'échappements de curseur par frame ; le
différentiel est habituellement zéro ou deux cellules. L'exercice 3 mesure le
ratio, et l'exercice 4 compresse davantage l'encodage. Un détail de plomberie
garde les octets en circulation : la `stdout` de C est *mise en tampon* —
quand la sortie va vers un tube ou un fichier elle s'accumule dans la
bibliothèque C jusqu'à ce que le tampon se remplisse — aussi `GridFlush`
finit-elle par `fflush(stdout)`. Sans cela l'affichage arrive par morceaux
quand le tampon en a envie, et une exécution redirigée produit un fichier par
rafales.

Rien de la boucle de jeu n'a changé : `ProcessInput` lit les touches, `Update`
avance les ticks et le marqueur, `Render` compose et vide désormais la grille
au lieu d'imprimer une ligne. La trace va toujours sur stderr en mode test et
rapporte maintenant aussi la cellule du marqueur, si bien que
`./snek 60 2>/dev/null` montre les octets de l'affichage seuls tandis que
`./snek 60 2>&1 >/dev/null` montre l'état seul. La commande de construction
est inchangée :

```
gcc -std=c11 -O0 -g -Wall -Wextra snek.c -o snek
```

## Étape de code

Un seul changement pour cette leçon : `snek.c` gagne les tampons arrière et
avant, les fonctions de dessin et de vidage de la grille, et le marqueur en
mouvement. Son état final est étiqueté `lesson-022`.

```diff
diff --git a/sandbox/snek/snek.c b/sandbox/snek/snek.c
index 251f6ef..3e6ed8b 100644
--- a/sandbox/snek/snek.c
+++ b/sandbox/snek/snek.c
@@ -1,6 +1,6 @@
 // snek.c — a terminal snake game, grown lesson by lesson.
 //
-// Lesson 021: raw terminal input — termios, poll, and escape sequences.
+// Lesson 022: the display — a double-buffered character grid.
 #define _POSIX_C_SOURCE 200809L /* clock_gettime, nanosleep, termios: POSIX, not ISO C */
 #include <errno.h>
 #include <poll.h>
@@ -15,6 +15,7 @@ static const double TICK_LEN = 1.0 / 10.0; /* fixed timestep: 10 updates per sec
 static const double FRAME_LEN = 1.0 / 30.0; /* frame cap: 30 frames per second */
 
 enum Direction { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
+enum { GRID_ROWS = 20, GRID_COLS = 40 };
 
 static const char *dir_names[] = {"up", "down", "left", "right"};
 
@@ -27,6 +28,11 @@ static unsigned long max_frames; /* stop after this many frames (0: until q) */
 static int test_mode;            /* a frame budget was given: trace every frame */
 static int dir = DIR_RIGHT;      /* where the snake is heading */
 static int esc;                  /* escape-sequence parser state */
+static int mark_row = 10, mark_col = 20; /* the marker's cell */
+
+static char back[GRID_ROWS][GRID_COLS];  /* drawn into, off-screen */
+static char front[GRID_ROWS][GRID_COLS]; /* what the terminal shows */
+static int front_valid;                  /* 0: the terminal needs a full redraw */
 
 static struct termios saved_termios;
 static int termios_saved;
@@ -105,12 +111,56 @@ static void ProcessInput(void)
         OnByte(buf[i]);
 }
 
+static void GridClear(void)
+{
+    for (int row = 0; row < GRID_ROWS; ++row)
+        for (int col = 0; col < GRID_COLS; ++col)
+            back[row][col] = ' ';
+}
+
+static void GridPut(int row, int col, char ch)
+{
+    if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS)
+        back[row][col] = ch;
+}
+
+static void GridFlush(void)
+{
+    if (!front_valid)
+        fprintf(stdout, "\033[2J\033[H"); /* clear the screen before the first draw */
+
+    for (int row = 0; row < GRID_ROWS; ++row) {
+        for (int col = 0; col < GRID_COLS; ++col) {
+            if (front_valid && back[row][col] == front[row][col])
+                continue;
+            fprintf(stdout, "\033[%d;%dH%c", row, col, back[row][col]);
+            front[row][col] = back[row][col];
+        }
+    }
+    front_valid = 1;
+    fflush(stdout);
+}
+
+static void MoveMarker(void)
+{
+    if (dir == DIR_UP) --mark_row;
+    else if (dir == DIR_DOWN) ++mark_row;
+    else if (dir == DIR_LEFT) --mark_col;
+    else if (dir == DIR_RIGHT) ++mark_col;
+
+    if (mark_row < 1) mark_row = GRID_ROWS - 1;
+    else if (mark_row >= GRID_ROWS) mark_row = 1;
+    if (mark_col < 0) mark_col = GRID_COLS - 1;
+    else if (mark_col >= GRID_COLS) mark_col = 0;
+}
+
 static void Update(double dt)
 {
     tick_accum += dt;
     while (tick_accum >= TICK_LEN) {
         tick_accum -= TICK_LEN;
         ++tick;
+        MoveMarker();
     }
     ++frame;
     if (max_frames > 0 && frame >= max_frames)
@@ -119,9 +169,17 @@ static void Update(double dt)
 
 static void Render(void)
 {
+    static const char banner[] = "snek - arrows to steer, q to quit";
+
+    GridClear();
+    for (int i = 0; banner[i]; ++i)
+        GridPut(0, i, banner[i]);
+    GridPut(mark_row, mark_col, '@');
+    GridFlush();
+
     if (test_mode)
-        fprintf(stderr, "frame=%lu tick=%lu dir=%s\n", frame, tick,
-                dir_names[dir]);
+        fprintf(stderr, "frame=%lu tick=%lu dir=%s at=%d,%d\n", frame, tick,
+                dir_names[dir], mark_row, mark_col);
 }
 
 int main(int argc, char **argv)
@@ -145,7 +203,7 @@ int main(int argc, char **argv)
 
     EnterRawMode();
     if (!test_mode)
-        fprintf(stderr, "snek — arrows to steer, q to quit\n");
+        fprintf(stderr, "snek - arrows to steer, q to quit\n");
 
     running = 1;
     double prev = Now();
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Compter le vidage *(predict-the-output)*

Prédisez combien de cellules `GridFlush` écrit à chacun de ces vidages : le
tout premier vidage d'une exécution ; un vidage sur une frame qui tombe entre
deux ticks ; un vidage sur une frame où le marqueur a bougé d'une cellule.
Ajoutez ensuite un compteur d'instrumentation à `GridFlush` — un `int cells`
incrémenté par cellule écrite, rapporté avec
`fprintf(stderr, "flush cells=%d\n", cells);` à la fin — exécutez `./snek 10`,
et vérifiez chaque prédiction contre les vrais comptes.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-022/ex1.md)

### Exercice 2 — Un de trop, deux fois *(fix-the-crash)*

Lancez le jeu sur un vrai terminal et tout le terrain se dessine une cellule
plus bas et à droite de là où la banderole et le marqueur devraient être —
tandis que le flux d'octets contient des adresses de curseur comme
`ESC [ 0 ; 0 H` qu'aucun terminal ne compte comme légales. Trouvez le bug
d'adressage dans `GridFlush`, et corrigez-le au seul endroit où les
coordonnées du programme rencontrent celles du terminal — sans changer une
seule coordonnée à l'intérieur du programme.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-022/ex2.md)

### Exercice 3 — Ce que le différentiel économise *(measure-the-performance)*

Mesurez le gain du double tampon. Lancez `./snek 60 2>/dev/null | wc -c` pour
le compte d'octets du différentiel. Donnez ensuite à `GridFlush` un
interrupteur — une vérification `getenv` pour `SNEK_FULL` qui lui fait traiter
chaque cellule comme changée — et mesurez la même exécution avec l'interrupteur
activé. Combien de fois plus d'octets coûte le redessin complet à 60 frames, et
comment ce ratio évolue-t-il à 120 ? D'où vient le coût supplémentaire, et
qu'en voit le joueur ?

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-022/ex3.md)

### Exercice 4 — Des suites, pas des cellules *(extend-the-code)*

Le vidage paie un échappement d'adresse de curseur par cellule changée même
quand les cellules changées sont voisines. Étendez `GridFlush` pour envoyer
chaque *suite* de cellules changées consécutives d'une ligne comme un seul
échappement suivi des caractères — le terminal avance le curseur tout seul à
mesure que les caractères arrivent. Mesurez
`./snek 60 2>/dev/null | wc -c` avant et après. De combien le premier vidage a
rétréci, et pourquoi est-ce là que sont les gains ?

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-022/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 021 — la saisie brute au terminal avec les codes d'échappement](lesson-021-terminal-input.md) ·
**Suivante :** [Leçon 023 — la machine à états : titre, jeu, mort](lesson-023-state-machine.md) ·
**Étiquette de code :** [`lesson-022`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-022)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-022-double-buffer.md`, révision `eea0461`.*

<!-- translation-source: book/lessons/part-0/lesson-022-double-buffer.md @ eea0461 -->
