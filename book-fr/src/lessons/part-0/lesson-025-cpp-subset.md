# Leçon 025 — le sous-ensemble C++ : classes et vtables

{{#include ../../stability-horizon.md}}

## Prose

C'est la dernière leçon de la partie 0, et elle remet le bac à sable au
langage dans lequel naît le moteur. Des leçons 019 à 024 vous avez écrit
`snek` en C ; le moteur de la partie 1 est du C++ — mais seulement un
sous-ensemble, choisi par une seule politique : **une caractéristique du
langage n'est admise que si nous savons expliquer en quoi elle se compile**.
Exactement cinq caractéristiques passent : les *références*, la
*surcharge*, les *espaces de noms*, `constexpr`, et les *classes avec
vtables*. Les modèles ne sont jamais admis dans le code du jeu. Les
exceptions, la STL, le trafic de `new`/`delete` — dehors jusqu'à ce que la
politique dise le contraire. La leçon d'aujourd'hui admet les cinq d'un coup
en convertissant `snek` sur place, et chaque caractéristique gagne sa place en
se compilant vers quelque chose que vous avez déjà construit à la main.

Le jeu ne change pas. Même écran titre, même mort déterministe au tick 9 avec
l'entrée scriptée, même `q` qui quitte. Ce qui change est le langage du
fichier où il vit : `snek.c` devient `snek.cpp`, et la commande de
construction devient :

```
g++ -std=c++17 -O0 -g -Wall -Wextra *.cpp -o snek
```

Lisez-la contre la commande de la leçon 001, option par option. `g++` exécute
le même pipeline — préprocesseur, compilateur, assembleur, éditeur de liens —
avec un compilateur C++ au milieu ; c'est le frère de `gcc`, pas un autre
genre d'outil (`clang++` est la même idée chez l'autre fournisseur).
`-std=c++17` épingle la version du langage comme le faisait `-std=c11` : le
compilateur est d'accord avec ce livre sur ce que signifie le code.
`-O0 -g -Wall -Wextra` veulent dire exactement ce qu'ils voulaient dire pour
`snek.c` — pas d'optimisation, des informations de débogage, des avertissements
qui sont du cours — et la construction reste à zéro avertissement. `*.cpp`
lie chaque source C++ du répertoire en un seul programme.

**Espaces de noms.** Tout ce dont le jeu est fait — chaque fonction, chaque
classe, l'état du jeu — vit désormais dans `namespace snek`. En quoi cela se
compile-t-il ? Des noms qualifiés, et rien d'autre. L'éditeur de liens voit
l'espace de noms comme partie du nom :

```
$ nm snek | grep RunEiPPc
0000000000002386 T _ZN4snek3RunEiPPc
```

Lisez le nom manglé comme un chemin préfixé par la longueur : `4snek` est
l'espace de noms, `3Run` est la fonction. À l'exécution, un nom qualifié coûte
exactement ce que coûte un nom non qualifié — rien ; la qualification a lieu à
la compilation et à l'édition de liens. Une fonction reste dehors : `main`
doit être le `main` de l'espace de noms *global*, parce que l'exécution C
cherche `::main` et rien d'autre. Le corps de `main` a donc déménagé en
`snek::Run` et le `main` global lui transmet un appel.

**`constexpr`.** `TICK_LEN`, `FRAME_LEN`, `GRID_ROWS`, `GRID_COLS` et
`SNAKE_MAX` sont désormais `constexpr` — des valeurs que le compilateur doit
pouvoir plier à la compilation. `SNAKE_MAX = GRID_ROWS * GRID_COLS` est
multiplié par le compilateur, pas par le CPU ; les tableaux ont leur taille
avant que le programme n'existe ; et là où les anciennes constantes `enum`
vivaient comme les ints de compilation du C, `constexpr` couvre aussi les
doubles. Le désassemblage des boucles de `Grid::Clear` montre le pliage — les
bornes sont des littéraux dans les instructions, pas des valeurs cherchées
quelque part :

```
    1c13:	addl   $0x1,-0x4(%rbp)
    1c17:	cmpl   $0x27,-0x4(%rbp)
    1c1b:	jle    1bec <_ZN4snek4Grid5ClearEv+0x1e>
    1c1d:	addl   $0x1,-0x8(%rbp)
    1c21:	cmpl   $0x13,-0x8(%rbp)
```

`0x27` est `GRID_COLS - 1`, `0x13` est `GRID_ROWS - 1` : le compilateur a fait
l'arithmétique et la machine se contente de comparer.

**Les classes, avant toute vtable.** La `struct Command` de la leçon 024 et
son balayeur `RunCommand` forment désormais une seule classe, `CommandTable` :
les lignes sont des données `Command` imbriquées exactement comme elles
étaient, et le balayeur est une fonction membre. En quoi se compile une
classe ? Les champs sont la même disposition de structure ; une fonction
membre est une fonction ordinaire qui reçoit l'adresse de l'objet comme
premier paramètre supplémentaire invisible — le pointeur `this`. Regardez
`Grid::Clear` s'ouvrir en le sauvegardant :

```
0000000000001bce <_ZN4snek4Grid5ClearEv>:
    1bce:	endbr64
    1bd2:	push   %rbp
    1bd3:	mov    %rsp,%rbp
    1bd6:	mov    %rdi,-0x18(%rbp)
```

`%rdi` est le registre du premier argument : `table.Run(key)` est
`Run(&table, key)` en tout sauf l'orthographe. Le constructeur est une
fonction ordinaire qui remplit les champs, et les membres privés portent un
souligné final — la convention que ce cours adopte pour eux. Rien de tout cela
ne coûte quoi que ce soit à l'exécution ; cela vous coûte un paramètre en forme
de pointeur.

**La surcharge.** `Grid` a deux fonctions `Put` : l'une écrit un seul `char`,
l'autre écrit une chaîne entière, et la surcharge de chaîne fait tourner ses
caractères vers celle de caractère. Même nom, même travail, deux types — le
compilateur choisit la fonction par le type de l'argument au site d'appel. En
quoi *cela* se compile-t-il ? Deux fonctions séparées avec deux noms d'éditeur
de liens séparés : encore le **name mangling**, cette fois encodant les types
des paramètres au lieu des espaces de noms.

```
$ nm snek | grep Put
0000000000001c8a T _ZN4snek4Grid3PutEiiPKc
0000000000001c2c T _ZN4snek4Grid3PutEiic
$ nm snek | grep Put | c++filt
0000000000001c8a T snek::Grid::Put(int, int, char const*)
0000000000001c2c T snek::Grid::Put(int, int, char)
```

`c++filt` dé-mangle ; les suffixes `Eiic` et `EiiPKc` sont les deux listes de
paramètres (`i i c` = int, int, char ; `PKc` = pointeur vers const char). C ne
pouvait pas faire cela — un nom, une fonction — et l'éditeur de liens a besoin
des noms séparés parce que les deux surcharges vivent dans le programme en
même temps.

**Les références.** Chaque `Draw` prend un `Grid &grid`. Une référence est un
pointeur que le compilateur déréférence pour vous : la signature se lit comme
un paramètre par valeur, la machine voit un `Grid *`, et chaque
`grid.Put(...)` à l'intérieur insère le déréférencement. Elle ne peut pas être
nulle et ne peut pas être réassignée, ce qui en fait le bon type pour
« dessine dans *ce* grid » — pas de copie des deux tampons de 800 octets, pas
de bruit d'adresse-prise aux sites d'appel. L'exercice 4 montre ce qu'une
référence empêche en silence.

**Les classes avec vtables — le chemin de dessin.** L'interface est une base
abstraite : `Drawable`, avec un `Draw` virtuel pur. Deux classes concrètes
l'implémentent — `GridView` dessine le bord, la nourriture et le serpent ;
`StatusView` dessine la ligne de statut et les invites — et `views` est une
petite table de lignes `Drawable *` que `Render` parcourt :

```c++
    for (size_t i = 0; i < sizeof views / sizeof views[0]; ++i)
        views[i]->Draw(grid);
```

La boucle ne sait pas ce qu'elle dessine. La répartition virtuelle se compile
en deux morceaux de données : un **vptr** dans chaque objet (son premier mot,
pointant vers la table de sa classe) et une **vtable** par classe — une table
de pointeurs de fonction. L'appel charge le premier mot de l'objet, charge
l'emplacement vers lequel il pointe, et appelle. Comparez avec la leçon 024,
caractéristique par caractéristique : les lignes associant les touches aux
fonctions sont la vtable ; le balayeur qui trouve la bonne ligne est l'indice
fixe ; `commands[i].run()` est l'appel. **La vtable est la table de commandes
de la leçon 024, automatisée** — le compilateur écrit la table une fois par
classe, plante le pointeur dans chaque objet, et vous ne voyez jamais la
machinerie sauf dans le binaire :

```
$ nm snek | grep _ZTV | grep snek | c++filt
0000000000004cc0 V vtable for snek::StatusView
0000000000004cd8 V vtable for snek::GridView
```

De vraies données, une par classe — l'exercice 3 suit un vptr jusqu'à un
pointeur de fonction. Une règle de la table : l'ordre de dessin est l'ordre de
la table, les lignes tardives peignant sur les précoces, ce qui explique que
la ligne de statut garde son ordre de superposition sous le serpent.

Ce qui n'est *pas* là compte autant : pas de modèles, pas d'exceptions, pas de
conteneurs STL, pas de `new`/`delete`. L'état du jeu est toujours des scalaires
et des tableaux simples, et les classes emballent du code et de la répartition
plutôt que de prendre possession de tout. C'est le sous-ensemble duquel part
le moteur de la partie 1 — chaque caractéristique ci-dessus, vous pouvez
désormais la lire comme de l'assembleur. L'étape de code est toute la
conversion en un seul diff.

## Étape de code

Un seul changement pour cette leçon : toute la conversion — `snek.c` renommé
en `snek.cpp`, la table de commandes emballée dans une classe, le chemin de
dessin reconstruit comme une interface dessinable avec une vtable. Son état
final est étiqueté `lesson-025`.

```diff
diff --git a/sandbox/snek/snek.c b/sandbox/snek/snek.cpp
similarity index 69%
rename from sandbox/snek/snek.c
rename to sandbox/snek/snek.cpp
index d0ac1f7..f7e8543 100644
--- a/sandbox/snek/snek.c
+++ b/sandbox/snek/snek.cpp
@@ -1,6 +1,6 @@
-// snek.c — a terminal snake game, grown lesson by lesson.
+// snek.cpp — a terminal snake game, grown lesson by lesson.
 //
-// Lesson 024: input dispatch — the function-pointer command table.
+// Lesson 025: the C++ subset — classes and vtables.
 #define _POSIX_C_SOURCE 200809L /* clock_gettime, nanosleep, termios: POSIX, not ISO C */
 #include <errno.h>
 #include <poll.h>
@@ -11,8 +11,12 @@
 #include <time.h>
 #include <unistd.h>
 
-static const double TICK_LEN = 1.0 / 10.0; /* fixed timestep: 10 updates per second */
-static const double FRAME_LEN = 1.0 / 30.0; /* frame cap: 30 frames per second */
+/* The whole game lives in namespace snek: qualified names, zero runtime cost.
+   Global main below is the one function the runtime insists on finding itself. */
+namespace snek {
+
+constexpr double TICK_LEN = 1.0 / 10.0; /* fixed timestep: 10 updates per second */
+constexpr double FRAME_LEN = 1.0 / 30.0; /* frame cap: 30 frames per second */
 
 enum Direction { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
 enum GameState { TITLE, PLAY, DEAD };
@@ -23,8 +27,9 @@ enum KeyCode {
     KEY_LEFT,
     KEY_RIGHT,
 };
-enum { GRID_ROWS = 20, GRID_COLS = 40 };
-enum { SNAKE_MAX = GRID_ROWS * GRID_COLS };
+constexpr int GRID_ROWS = 20;
+constexpr int GRID_COLS = 40;
+constexpr int SNAKE_MAX = GRID_ROWS * GRID_COLS; /* folded at compile time */
 
 static const char *dir_names[] = {"up", "down", "left", "right"};
 static const char *state_names[] = {"title", "play", "dead"};
@@ -46,10 +51,6 @@ static int food_row, food_col;
 static int score;
 static unsigned rng_state = 12345; /* fixed seed: every run is reproducible */
 
-static char back[GRID_ROWS][GRID_COLS];  /* drawn into, off-screen */
-static char front[GRID_ROWS][GRID_COLS]; /* what the terminal shows */
-static int front_valid;                  /* 0: the terminal needs a full redraw */
-
 static struct termios saved_termios;
 static int termios_saved;
 static volatile sig_atomic_t interrupted;
@@ -206,12 +207,34 @@ static void CmdDown(void) { CmdTurn(DIR_DOWN); }
 static void CmdLeft(void) { CmdTurn(DIR_LEFT); }
 static void CmdRight(void) { CmdTurn(DIR_RIGHT); }
 
-struct Command {
-    int key;
-    void (*run)(void);
+/* The command table of lesson 024, as a class: the rows are the data it was,
+   the scanner is now a member function over an invisible this pointer. */
+class CommandTable {
+public:
+    struct Command {
+        int key;
+        void (*run)(void);
+    };
+
+    CommandTable(const Command *commands, size_t count)
+        : commands_(commands), count_(count) {}
+
+    void Run(int key) const
+    {
+        for (size_t i = 0; i < count_; ++i) {
+            if (commands_[i].key == key) {
+                commands_[i].run();
+                return;
+            }
+        }
+    }
+
+private:
+    const Command *commands_;
+    size_t count_;
 };
 
-static const struct Command commands[] = {
+static const CommandTable::Command commands[] = {
     { 'q',        CmdQuit },
     { ' ',        CmdStart },
     { '\n',       CmdStart },
@@ -221,15 +244,7 @@ static const struct Command commands[] = {
     { KEY_RIGHT,  CmdRight },
 };
 
-static void RunCommand(int key)
-{
-    for (size_t i = 0; i < sizeof commands / sizeof commands[0]; ++i) {
-        if (commands[i].key == key) {
-            commands[i].run();
-            return;
-        }
-    }
-}
+static const CommandTable table(commands, sizeof commands / sizeof commands[0]);
 
 static int ParseByte(unsigned char c)
 {
@@ -267,67 +282,120 @@ static void ProcessInput(void)
     for (ssize_t i = 0; i < n; ++i) {
         int key = ParseByte(buf[i]);
         if (key != KEY_NONE)
-            RunCommand(key);
+            table.Run(key);
     }
 }
 
-static void GridClear(void)
+/* The off-screen buffer, as a class. The two Put overloads do one job for two
+   types; the compiler keeps them apart by mangling their names. */
+class Grid {
+public:
+    void Clear(void);
+    void Put(int row, int col, char ch);
+    void Put(int row, int col, const char *s);
+    void Flush(void);
+
+private:
+    char back_[GRID_ROWS][GRID_COLS];  /* drawn into, off-screen */
+    char front_[GRID_ROWS][GRID_COLS]; /* what the terminal shows */
+    int front_valid_;                  /* 0: the terminal needs a full redraw */
+};
+
+void Grid::Clear(void)
 {
     for (int row = 0; row < GRID_ROWS; ++row)
         for (int col = 0; col < GRID_COLS; ++col)
-            back[row][col] = ' ';
+            back_[row][col] = ' ';
 }
 
-static void GridPut(int row, int col, char ch)
+void Grid::Put(int row, int col, char ch)
 {
     if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS)
-        back[row][col] = ch;
+        back_[row][col] = ch;
 }
 
-static void GridText(int row, int col, const char *s)
+void Grid::Put(int row, int col, const char *s)
 {
     for (int i = 0; s[i]; ++i)
-        GridPut(row, col + i, s[i]);
+        Put(row, col + i, s[i]);
 }
 
-static void GridFlush(void)
+void Grid::Flush(void)
 {
-    if (!front_valid)
+    if (!front_valid_)
         fprintf(stdout, "\033[2J\033[H"); /* clear the screen before the first draw */
 
     for (int row = 0; row < GRID_ROWS; ++row) {
         for (int col = 0; col < GRID_COLS; ++col) {
-            if (front_valid && back[row][col] == front[row][col])
+            if (front_valid_ && back_[row][col] == front_[row][col])
                 continue;
-            fprintf(stdout, "\033[%d;%dH%c", row, col, back[row][col]);
-            front[row][col] = back[row][col];
+            fprintf(stdout, "\033[%d;%dH%c", row, col, back_[row][col]);
+            front_[row][col] = back_[row][col];
         }
     }
-    front_valid = 1;
+    front_valid_ = 1;
     fflush(stdout);
 }
 
-static void DrawBorder(void)
+static Grid grid;
+
+/* The draw path, as an interface: a drawable knows only that it must draw
+   itself into a Grid. A reference parameter — a pointer the compiler
+   dereferences for you. */
+class Drawable {
+public:
+    virtual void Draw(Grid &grid) const = 0;
+};
+
+class GridView : public Drawable {
+public:
+    void Draw(Grid &grid) const;
+};
+
+class StatusView : public Drawable {
+public:
+    void Draw(Grid &grid) const;
+};
+
+void GridView::Draw(Grid &grid) const
 {
     for (int col = 0; col < GRID_COLS; ++col) {
-        GridPut(1, col, '-');
-        GridPut(GRID_ROWS - 1, col, '-');
+        grid.Put(1, col, '-');
+        grid.Put(GRID_ROWS - 1, col, '-');
     }
     for (int row = 1; row < GRID_ROWS; ++row) {
-        GridPut(row, 0, '|');
-        GridPut(row, GRID_COLS - 1, '|');
+        grid.Put(row, 0, '|');
+        grid.Put(row, GRID_COLS - 1, '|');
     }
-    GridPut(1, 0, '+');
-    GridPut(1, GRID_COLS - 1, '+');
-    GridPut(GRID_ROWS - 1, 0, '+');
-    GridPut(GRID_ROWS - 1, GRID_COLS - 1, '+');
+    grid.Put(1, 0, '+');
+    grid.Put(1, GRID_COLS - 1, '+');
+    grid.Put(GRID_ROWS - 1, 0, '+');
+    grid.Put(GRID_ROWS - 1, GRID_COLS - 1, '+');
+
+    if (state == TITLE)
+        return; /* on the title screen the playfield is just the border */
+    grid.Put(food_row, food_col, '*');
+    for (int i = snake_len - 1; i >= 0; --i)
+        grid.Put(snake_row[i], snake_col[i], i == 0 ? '@' : 'o');
 }
 
-static void DrawSnake(void)
+void StatusView::Draw(Grid &grid) const
 {
-    GridPut(food_row, food_col, '*');
-    for (int i = snake_len - 1; i >= 0; --i)
-        GridPut(snake_row[i], snake_col[i], i == 0 ? '@' : 'o');
+    char msg[GRID_COLS + 1];
+
+    if (state == TITLE) {
+        grid.Put(0, 0, "SNEK");
+        grid.Put(9, 10, "press space to play");
+        grid.Put(11, 15, "q to quit");
+    } else if (state == DEAD) {
+        snprintf(msg, sizeof msg, "game over! score: %d", score);
+        grid.Put(0, 0, msg);
+        grid.Put(9, 9, "press space to play again");
+        grid.Put(11, 15, "q to quit");
+    } else {
+        snprintf(msg, sizeof msg, "score: %d", score);
+        grid.Put(0, 0, msg);
+    }
 }
 
 static void Update(double dt)
@@ -344,28 +412,17 @@ static void Update(double dt)
         running = 0;
 }
 
+/* Draw order is table order: later rows paint over earlier ones. */
+static GridView playfield;
+static StatusView status;
+static Drawable *const views[] = { &status, &playfield };
+
 static void Render(void)
 {
-    char msg[GRID_COLS + 1];
-
-    GridClear();
-    DrawBorder();
-    if (state == TITLE) {
-        GridText(0, 0, "SNEK");
-        GridText(9, 10, "press space to play");
-        GridText(11, 15, "q to quit");
-    } else if (state == DEAD) {
-        snprintf(msg, sizeof msg, "game over! score: %d", score);
-        GridText(0, 0, msg);
-        GridText(9, 9, "press space to play again");
-        GridText(11, 15, "q to quit");
-        DrawSnake();
-    } else {
-        snprintf(msg, sizeof msg, "score: %d", score);
-        GridText(0, 0, msg);
-        DrawSnake();
-    }
-    GridFlush();
+    grid.Clear();
+    for (size_t i = 0; i < sizeof views / sizeof views[0]; ++i)
+        views[i]->Draw(grid);
+    grid.Flush();
 
     if (test_mode)
         fprintf(stderr, "frame=%lu tick=%lu state=%s score=%d dir=%s at=%d,%d\n",
@@ -373,7 +430,7 @@ static void Render(void)
                 snake_row[0], snake_col[0]);
 }
 
-int main(int argc, char **argv)
+int Run(int argc, char **argv)
 {
     if (argc > 2) {
         fprintf(stderr, "usage: %s [FRAMES]\n", argv[0]);
@@ -416,3 +473,10 @@ int main(int argc, char **argv)
     fprintf(stderr, "done after %lu frames, %lu ticks\n", frame, tick);
     return 0;
 }
+
+} /* namespace snek */
+
+int main(int argc, char **argv)
+{
+    return snek::Run(argc, argv);
+}
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Une troisième vue *(extend-the-code)*

La table des vues a deux lignes et `Render` ne mentionne toujours jamais ce
qu'elles dessinent. Ajoutez une troisième `Drawable` concrète — une
`TallyView` qui dessine les compteurs de frames et de ticks (`f=NNN t=NNN`)
le long de la ligne de bord inférieure — enregistrez-la dans `views`, et
montrez-la à l'œuvre dans une exécution en mode test (`./snek 30`). Combien de
fonctions existantes a-t-il fallu toucher pour l'ajouter ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-025/ex1.md)

### Exercice 2 — Le mot caché *(predict-the-output)*

Les deux classes de vue ne portent aucune donnée du tout. Avant de toucher
quoi que ce soit, prédisez ce qu'impriment `sizeof(GridView)`,
`sizeof(StatusView)`, et le `sizeof` d'un jumeau simple vide — une
`struct PlainView` avec un `Draw` non virtuel et aucun membre — sur votre
machine. Ajoutez ensuite le `fprintf` d'instrumentation qui imprime les trois
tailles avant que la boucle ne commence, exécutez `./snek 5`, et accordez les
nombres. Qu'est-ce que le mot supplémentaire, et pourquoi chaque objet le
paie-t-il ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-025/ex2.md)

### Exercice 3 — La vtable sous la loupe *(explain-in-prose)*

Expliquez en prose en quoi se compile `views[i]->Draw(grid)` — dans les mêmes
termes machine que la leçon 024 employait pour `commands[i].run()`. Avant
d'écrire, rendez le mécanisme visible : imprimez le premier mot de l'objet
comme pointeur, imprimez le premier pointeur de fonction stocké là où ce mot
pointe, exécutez une frame, et comparez les deux adresses avec `nm snek` (via
`c++filt`) pour `GridView::Draw`. Que prouvent les nombres, et quelle partie
de la répartition de la leçon 024 le compilateur vient-il d'écrire pour vous ?

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-025/ex3.md)

### Exercice 4 — La copie qui ne peut pas exister *(fix-the-crash)*

Un collègue veut que `Render` garde la vue de statut en cache dans une locale —
`Drawable saved = status;` — et la construction meurt avec
`error: cannot allocate an object of abstract type 'snek::Drawable'`. Faites
fonctionner le cache : après votre correction, le texte de statut doit
toujours venir de `StatusView::Draw` même s'il est dessiné à travers le nom
mis en cache. Expliquez pourquoi le compilateur a refusé la copie, et ce que
le programme aurait fait si elle avait été permise.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-025/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 024 — la table de commandes à pointeurs de fonction](lesson-024-command-table.md) ·
**Suivante :** [Leçon 026 — la base de code naît](../part-1/lesson-026-birth.md) ·
**Étiquette de code :** [`lesson-025`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-025)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-025-cpp-subset.md`, révision `100464c`.*

<!-- translation-source: book/lessons/part-0/lesson-025-cpp-subset.md @ 100464c -->
