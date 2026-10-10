# Leçon 023 — la machine à états : titre, jeu, mort

{{#include ../../stability-horizon.md}}

## Prose

Un jeu n'est pas un flux de contrôle — il en a plusieurs, et le programme est
toujours dans exactement l'un d'eux. `snek` montre un écran titre ; puis il
joue ; puis le serpent meurt et un écran de fin attend un redémarrage. Ce sont
des **états**, et l'étape de code d'aujourd'hui les rend explicites : un
`enum GameState { TITLE, PLAY, DEAD }`, une variable `state`, et chaque partie
du programme demandant « dans quel état sommes-nous ? » avant de faire quoi
que ce soit de spécifique à un état. C'est une machine à états, et c'est ainsi
que chaque jeu — et chaque parseur, protocole, et IU — empêche les modes de
s'effondrer en conditionnels imbriqués.

Pourquoi pas des drapeaux ? `int playing, dead;` *fonctionne* jusqu'à ce que
non : après cinq états il y a des combinaisons qu'aucun test de drapeau
n'exclut (`playing && dead`), chaque gestionnaire d'entrée doit raisonner sur
toutes, et la réponse à « que fait l'espace ? » vit dans des `if` dispersés.
Avec une variable `state` les combinaisons légales sont exactement les états
déclarés, chaque transition est une seule affectation que vous pouvez trouver
par recherche, et `Render` devient une branche par état dessinant chacune un
écran complet. La machine est aussi *testable* : la trace imprime `state=…`
chaque frame, si bien qu'une exécution scriptée montre les transitions comme
données.

L'état décide de tout. `Update` avance le serpent par tick seulement en
`PLAY` — en `TITLE` et `DEAD` le monde du jeu s'arrête pendant que la boucle et
l'horloge continuent. `OnByte` achemine les touches par état : `q` quitte
toujours, l'espace démarre une partie depuis `TITLE` ou en redémarre une
depuis `DEAD` (tout sauf `PLAY`), et les flèches ne gouvernent que pendant le
jeu. Notez une règle cuite dans la direction : un serpent de plus d'un segment
ne peut pas se retourner dans son propre cou — l'entrée est vérifiée contre la
direction courante et *ignorée* si c'est un demi-tour. Ce sont les règles du
jeu exprimées comme validation d'entrée ; laisser passer le retournement
tuerait le joueur au tick suivant.

Sous les règles se trouvent les données du jeu, toutes de simples
tableaux — l'idiome C pour un objet de jeu. Le serpent est deux tableaux `int`
de paires ligne/colonne plus une longueur, le segment 0 étant la tête ; le
mouvement est un décalage : vérifiez d'abord la cellule de tête *proposée* (un
mur termine la partie, une cellule du corps termine la partie), puis réécrivez
le tableau — chaque segment prend la cellule de son prédécesseur, la tête prend
la nouvelle, et l'ancienne queue est simplement écrasée. Manger la nourriture
fait grandir le serpent d'un segment (le décalage copie un segment de plus,
donc la queue survit), incrémente `score`, et place de la nouvelle nourriture.
`PlaceFood` balaie depuis un départ pseudo-aléatoire jusqu'à la première
cellule intérieure libre, en utilisant un petit générateur congruentiel
linéaire amorcé avec un nombre *fixe* — exprès, pour que chaque exécution de
`./snek 150` place la même nourriture et que les traces de test soient
reproductibles. Un jeu livré s'amorcerait sur l'horloge ; un jeu testé
s'amorce sur une constante.

`StartGame` construit un serpent neuf de trois segments au milieu du terrain,
prêt à bouger vers la droite — une nouvelle vie est une remise à zéro complète
du monde que la machine gère. (De combien complète, exactement, est l'affaire
de l'exercice 2.) L'écran de mort garde le plateau final visible sous son
message — la mort est un état avec une image, pas une sortie.

Tester la machine entière veut dire fournir de l'entrée dans le *temps* : le
shell est heureux d'être un joueur scripté, parce qu'un producteur de pipeline
peut décider quand chaque touche arrive —
`( printf ' \033[A'; sleep 3.5; printf ' ' ) | ./snek 150` démarre la partie,
dirige vers le haut, et appuie à nouveau sur espace seulement après que le
serpent est mort. La trace narre alors la vie de la machine : état, score,
direction, et la cellule de la tête à chaque frame. L'exercice 1 prédit une
telle histoire avant que vous ne l'exécutiez.

La commande de construction est inchangée :

```
gcc -std=c11 -O0 -g -Wall -Wextra snek.c -o snek
```

## Étape de code

Un seul changement pour cette leçon : `snek.c` gagne le jeu — états, serpent,
nourriture, score, et les règles qui les font bouger. Son état final est
étiqueté `lesson-023`.

```diff
diff --git a/sandbox/snek/snek.c b/sandbox/snek/snek.c
index 3e6ed8b..7b17290 100644
--- a/sandbox/snek/snek.c
+++ b/sandbox/snek/snek.c
@@ -1,6 +1,6 @@
 // snek.c — a terminal snake game, grown lesson by lesson.
 //
-// Lesson 022: the display — a double-buffered character grid.
+// Lesson 023: the game — title, play, and death as explicit states.
 #define _POSIX_C_SOURCE 200809L /* clock_gettime, nanosleep, termios: POSIX, not ISO C */
 #include <errno.h>
 #include <poll.h>
@@ -15,9 +15,12 @@ static const double TICK_LEN = 1.0 / 10.0; /* fixed timestep: 10 updates per sec
 static const double FRAME_LEN = 1.0 / 30.0; /* frame cap: 30 frames per second */
 
 enum Direction { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
+enum GameState { TITLE, PLAY, DEAD };
 enum { GRID_ROWS = 20, GRID_COLS = 40 };
+enum { SNAKE_MAX = GRID_ROWS * GRID_COLS };
 
 static const char *dir_names[] = {"up", "down", "left", "right"};
+static const char *state_names[] = {"title", "play", "dead"};
 
 static int running;              /* the loop runs while this is true */
 static unsigned long frame;      /* frames since the loop started */
@@ -28,7 +31,13 @@ static unsigned long max_frames; /* stop after this many frames (0: until q) */
 static int test_mode;            /* a frame budget was given: trace every frame */
 static int dir = DIR_RIGHT;      /* where the snake is heading */
 static int esc;                  /* escape-sequence parser state */
-static int mark_row = 10, mark_col = 20; /* the marker's cell */
+static int state = TITLE;        /* title, play, or dead */
+
+static int snake_row[SNAKE_MAX], snake_col[SNAKE_MAX]; /* segment 0 is the head */
+static int snake_len;
+static int food_row, food_col;
+static int score;
+static unsigned rng_state = 12345; /* fixed seed: every run is reproducible */
 
 static char back[GRID_ROWS][GRID_COLS];  /* drawn into, off-screen */
 static char front[GRID_ROWS][GRID_COLS]; /* what the terminal shows */
@@ -81,6 +90,84 @@ static void EnterRawMode(void)
     signal(SIGINT, OnInterrupt);
 }
 
+static unsigned RngNext(void)
+{
+    rng_state = rng_state * 1103515245u + 12345u;
+    return (rng_state >> 16) & 0x7fff;
+}
+
+static int SnakeAt(int row, int col)
+{
+    for (int i = 0; i < snake_len; ++i)
+        if (snake_row[i] == row && snake_col[i] == col)
+            return 1;
+    return 0;
+}
+
+static void PlaceFood(void)
+{
+    int start = (int)(RngNext() % (GRID_ROWS * GRID_COLS));
+    for (int i = 0; i < GRID_ROWS * GRID_COLS; ++i) {
+        int cell = (start + i) % (GRID_ROWS * GRID_COLS);
+        int row = cell / GRID_COLS, col = cell % GRID_COLS;
+        if (row < 2 || row > GRID_ROWS - 2 || col < 1 || col > GRID_COLS - 2)
+            continue; /* the border and the status row */
+        if (SnakeAt(row, col))
+            continue;
+        food_row = row;
+        food_col = col;
+        return;
+    }
+}
+
+static void StartGame(void)
+{
+    snake_len = 3;
+    for (int i = 0; i < snake_len; ++i) {
+        snake_row[i] = GRID_ROWS / 2;
+        snake_col[i] = GRID_COLS / 2 - i;
+    }
+    score = 0;
+    PlaceFood();
+    state = PLAY;
+}
+
+static void AdvanceSnake(void)
+{
+    int new_row = snake_row[0], new_col = snake_col[0];
+    if (dir == DIR_UP) --new_row;
+    else if (dir == DIR_DOWN) ++new_row;
+    else if (dir == DIR_LEFT) --new_col;
+    else if (dir == DIR_RIGHT) ++new_col;
+
+    if (new_row < 2 || new_row > GRID_ROWS - 2 ||
+        new_col < 1 || new_col > GRID_COLS - 2) {
+        state = DEAD; /* the wall */
+        return;
+    }
+    for (int i = 0; i < snake_len - 1; ++i) {
+        if (snake_row[i] == new_row && snake_col[i] == new_col) {
+            state = DEAD; /* itself */
+            return;
+        }
+    }
+
+    int grow = (new_row == food_row && new_col == food_col);
+    if (grow) {
+        ++score;
+        if (snake_len < SNAKE_MAX)
+            ++snake_len;
+    }
+    for (int i = snake_len - 1; i > 0; --i) {
+        snake_row[i] = snake_row[i - 1];
+        snake_col[i] = snake_col[i - 1];
+    }
+    snake_row[0] = new_row;
+    snake_col[0] = new_col;
+    if (grow)
+        PlaceFood();
+}
+
 static void OnByte(unsigned char c)
 {
     if (esc == 0) {
@@ -88,14 +175,27 @@ static void OnByte(unsigned char c)
             esc = 1;
         else if (c == 'q')
             running = 0;
+        else if (c == ' ' && state != PLAY)
+            StartGame();
     } else if (esc == 1) {
         esc = (c == '[') ? 2 : 0; /* a lone ESC eats the next byte */
     } else {
         esc = 0;
-        if (c == 'A') dir = DIR_UP;
-        else if (c == 'B') dir = DIR_DOWN;
-        else if (c == 'C') dir = DIR_RIGHT;
-        else if (c == 'D') dir = DIR_LEFT;
+        int new_dir = -1;
+        if (c == 'A') new_dir = DIR_UP;
+        else if (c == 'B') new_dir = DIR_DOWN;
+        else if (c == 'C') new_dir = DIR_RIGHT;
+        else if (c == 'D') new_dir = DIR_LEFT;
+        if (new_dir < 0 || state != PLAY)
+            return;
+        /* a longer snake cannot reverse into its own neck */
+        if (snake_len > 1 &&
+            ((new_dir == DIR_UP && dir == DIR_DOWN) ||
+             (new_dir == DIR_DOWN && dir == DIR_UP) ||
+             (new_dir == DIR_LEFT && dir == DIR_RIGHT) ||
+             (new_dir == DIR_RIGHT && dir == DIR_LEFT)))
+            return;
+        dir = new_dir;
     }
 }
 
@@ -124,6 +224,12 @@ static void GridPut(int row, int col, char ch)
         back[row][col] = ch;
 }
 
+static void GridText(int row, int col, const char *s)
+{
+    for (int i = 0; s[i]; ++i)
+        GridPut(row, col + i, s[i]);
+}
+
 static void GridFlush(void)
 {
     if (!front_valid)
@@ -141,17 +247,27 @@ static void GridFlush(void)
     fflush(stdout);
 }
 
-static void MoveMarker(void)
+static void DrawBorder(void)
 {
-    if (dir == DIR_UP) --mark_row;
-    else if (dir == DIR_DOWN) ++mark_row;
-    else if (dir == DIR_LEFT) --mark_col;
-    else if (dir == DIR_RIGHT) ++mark_col;
+    for (int col = 0; col < GRID_COLS; ++col) {
+        GridPut(1, col, '-');
+        GridPut(GRID_ROWS - 1, col, '-');
+    }
+    for (int row = 1; row < GRID_ROWS; ++row) {
+        GridPut(row, 0, '|');
+        GridPut(row, GRID_COLS - 1, '|');
+    }
+    GridPut(1, 0, '+');
+    GridPut(1, GRID_COLS - 1, '+');
+    GridPut(GRID_ROWS - 1, 0, '+');
+    GridPut(GRID_ROWS - 1, GRID_COLS - 1, '+');
+}
 
-    if (mark_row < 1) mark_row = GRID_ROWS - 1;
-    else if (mark_row >= GRID_ROWS) mark_row = 1;
-    if (mark_col < 0) mark_col = GRID_COLS - 1;
-    else if (mark_col >= GRID_COLS) mark_col = 0;
+static void DrawSnake(void)
+{
+    GridPut(food_row, food_col, '*');
+    for (int i = snake_len - 1; i >= 0; --i)
+        GridPut(snake_row[i], snake_col[i], i == 0 ? '@' : 'o');
 }
 
 static void Update(double dt)
@@ -160,7 +276,8 @@ static void Update(double dt)
     while (tick_accum >= TICK_LEN) {
         tick_accum -= TICK_LEN;
         ++tick;
-        MoveMarker();
+        if (state == PLAY)
+            AdvanceSnake();
     }
     ++frame;
     if (max_frames > 0 && frame >= max_frames)
@@ -169,17 +286,31 @@ static void Update(double dt)
 
 static void Render(void)
 {
-    static const char banner[] = "snek - arrows to steer, q to quit";
+    char msg[GRID_COLS + 1];
 
     GridClear();
-    for (int i = 0; banner[i]; ++i)
-        GridPut(0, i, banner[i]);
-    GridPut(mark_row, mark_col, '@');
+    DrawBorder();
+    if (state == TITLE) {
+        GridText(0, 0, "SNEK");
+        GridText(9, 10, "press space to play");
+        GridText(11, 15, "q to quit");
+    } else if (state == DEAD) {
+        snprintf(msg, sizeof msg, "game over! score: %d", score);
+        GridText(0, 0, msg);
+        GridText(9, 9, "press space to play again");
+        GridText(11, 15, "q to quit");
+        DrawSnake();
+    } else {
+        snprintf(msg, sizeof msg, "score: %d", score);
+        GridText(0, 0, msg);
+        DrawSnake();
+    }
     GridFlush();
 
     if (test_mode)
-        fprintf(stderr, "frame=%lu tick=%lu dir=%s at=%d,%d\n", frame, tick,
-                dir_names[dir], mark_row, mark_col);
+        fprintf(stderr, "frame=%lu tick=%lu state=%s score=%d dir=%s at=%d,%d\n",
+                frame, tick, state_names[state], score, dir_names[dir],
+                snake_row[0], snake_col[0]);
 }
 
 int main(int argc, char **argv)
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Compter jusqu'au mur *(predict-the-output)*

Prédisez la trace de `printf ' \033[A' | ./snek 60` à partir de la géométrie
seule : l'espace démarre une partie, la flèche du haut tourne le serpent, et le
test de mur dans `AdvanceSnake` décide la fin. À quel **tick** le serpent
meurt-il, et pourquoi celui-là ? Sur quelle cellule la tête se fige-t-elle ?
Écrivez la prédiction avant d'exécuter quoi que ce soit. Puis lancez-le et
rendez compte de chaque tick d'écart. Pour regarder chaque pas au moment où il
est tenté, ajoutez une ligne d'instrumentation en tête de `AdvanceSnake` —
`fprintf(stderr, "head %d,%d\n", new_row, new_col);` — et servez-vous-en pour
régler tout débat sur les comptes de mouvement.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-023/ex1.md)

### Exercice 2 — Le redémarrage qui n'en était pas un *(fix-the-crash)*

Jouez une vie, mourez, et appuyez à nouveau sur espace :

```
( printf ' \033[A'; sleep 3.5; printf ' ' ) | ./snek 150
```

Le redémarrage ramène le serpent — mais il meurt presque exactement comme il
est mort la première fois, comme si la seconde vie vivait encore la fin de la
première. Trouvez le terrain de jeu qui survit au « nouveau départ » dans
`StartGame`, corrigez la remise à zéro, et relancez le même script pour montrer
la différence dans la trace. (Le producteur a besoin de son `sleep` : le second
espace doit arriver pendant que le jeu est sur l'écran de mort.)

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-023/ex2.md)

### Exercice 3 — Pause *(extend-the-code)*

Ajoutez un état `PAUSE` basculé avec `p` : en pause le serpent se fige où il
est et la grille le dit ; `p` à nouveau reprend le jeu. La pause ne doit pas
déborder dans les autres états — `p` sur l'écran titre ou de mort ne fait rien.
Montrez le gel à l'œuvre avec un script minuté comme
`( printf ' p'; sleep 1.5; printf 'p' ) | ./snek 75`, et expliquez ce qui est
arrivé au compteur de ticks pendant que le serpent restait immobile.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-023/ex3.md)

### Exercice 4 — Pourquoi des états, pas des drapeaux *(explain-in-prose)*

Expliquez en prose pourquoi ce jeu garde une variable `state` au lieu de
drapeaux indépendants `playing`/`dead`/`paused` : quelles combinaisons les
drapeaux rendent exprimables, ce que coûte une transition dans chaque
conception, et comment la prochaine fonctionnalité atterrirait dans chacune.
Avant d'écrire, rendez les transitions visibles : enveloppez chaque changement
d'état dans un auxiliaire `SetState` qui journalise `state <ancien> -> <nouveau>`
sur stderr, lancez un script jouer-mourir-redémarrer, et décrivez ce que le
journal montre de la forme de la machine.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-023/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 022 — la grille de caractères à double tampon](lesson-022-double-buffer.md) ·
**Suivante :** [Leçon 024 — la table de commandes à pointeurs de fonction](lesson-024-command-table.md) ·
**Étiquette de code :** [`lesson-023`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-023)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-023-state-machine.md`, révision `94b8aa1`.*

<!-- translation-source: book/lessons/part-0/lesson-023-state-machine.md @ 94b8aa1 -->
