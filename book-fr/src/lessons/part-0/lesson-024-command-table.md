# Leçon 024 — la table de commandes à pointeurs de fonction

{{#include ../../stability-horizon.md}}

## Prose

Le code d'entrée a un problème de forme. Chaque touche que le jeu connaît est
un `else if` quelque part dans `OnByte`, mêlée aux détails de parsing et à la
politique du jeu : ajouter une touche veut dire trouver la bonne branche, et
la réponse à « que lie ce jeu ? » est étalée à travers un corps de fonction.
Cette leçon remplace les branches par une **table de commandes** — des lignes
de données associant des touches aux fonctions qui agissent sur elles — et
enseigne la seule caractéristique de C qui rend une telle table possible : le
*pointeur de fonction*. C'est la route C vers les interfaces, et l'aperçu au
niveau machine de la méthode virtuelle C++ que la leçon 025 construit.

D'abord, le type. Dans `void (*run)(void)`, lisez le nom de l'intérieur vers
l'extérieur : `run` est un pointeur (`*run`) vers quelque chose qui prend
`(void)` et renvoie `void` — l'adresse d'une fonction avec cette signature.
`commands[i].run()` appelle à travers cette adresse : le CPU saute vers
n'importe quelle fonction que la ligne contient. Un pointeur de fonction est
une valeur ordinaire — il peut vivre dans une structure, dans un tableau, être
passé et comparé — et le compilateur ne vérifie que la *signature*, pas quelle
fonction ce sera. C'est la couture sur laquelle toute la leçon repose : du
comportement stocké comme données.

La table elle-même est une structure et un tableau :

```c
struct Command {
    int key;
    void (*run)(void);
};
```

Chaque ligne lie un code de touche à une fonction de commande. `RunCommand`
balaie les lignes et exécute la première dont la touche correspond — la
répartition est désormais une recherche de données, et les fonctions qu'elle
appelle (`CmdQuit`, `CmdStart`, `CmdUp`, …) sont de simples actions de jeu
sans connaissance des claviers. Les quatre commandes de direction sont des
une-lignes qui transmettent à un `CmdTurn` partagé, parce que la signature de
pointeur de fonction est fixée à `void (void)` et qu'une commande qui veut un
argument l'emballe — l'idiome de l'adaptateur, qui vaut la peine d'être
reconnu dès maintenant, parce que le `std::function` de C++ et ses amis
existent exactement pour cette friction.

Pour que la table soit balayable, chaque touche a besoin d'un code, et les
codes ne doivent pas entrer en collision. Les touches ordinaires gardent leurs
octets (`'q'` est 113, `' '` est 32) ; les touches *nommées* — les flèches —
reçoivent des valeurs d'`enum KeyCode` commençant à 256, au-delà de tout octet
possible. Le travail du parseur se resserre en conséquence : `ParseByte`
*émet désormais des codes de touche* au lieu d'exécuter des actions — une
séquence de flèche terminée devient `KEY_UP`, un octet ordinaire devient
lui-même, `KEY_NONE` veut dire « pas encore une touche » — et `ProcessInput`
nourrit chaque code émis à `RunCommand`. Le parsing et la politique sont
enfin séparés : les gardes de la machine à états (l'espace seulement hors de
`PLAY`, pas de retournement dans le cou) ont déménagé dans les fonctions de
commande, où les règles du jeu appartiennent.

Un détail du mode brut se resserre en chemin : `EnterRawMode` efface aussi
`ICRNL`, le drapeau de la discipline de ligne qui réécrit CR en NL à l'entrée.
Avec une table de commandes, le programme se compare aux octets que le clavier
*envoie*, et les traductions silencieuses sont exactement le genre de surprise
qu'une table rend visible — l'exercice 2 a la surprise qui attend. La forme
générale mérite sa propre phrase : quand les touches sont des lignes de
données, la valeur d'octet réelle de chaque touche devient partie du contrat
de votre programme avec le terminal.

Pourquoi est-ce la route C vers les interfaces ? Le balayeur ne sait pas
quelle fonction une ligne contient — associez des données à du comportement,
échangez le comportement sans toucher à la répartition. C'est une interface en
tout sauf le support du langage, et quand la leçon 025 emballera cette table
dans une classe C++ avec une méthode virtuelle, la forme compilée ne changera
presque pas : une table d'adresses de fonctions, un appel indirect. Ce que vous
écrivez aujourd'hui en C est ce que le compilateur générera pour vous demain.

Ajouter une touche est désormais une ligne — essayez l'exercice 1 et sentez la
différence. Le balayage a un contrat qui vaut la peine d'être connu par cœur
(exercice 3), et les pointeurs peuvent être *inspectés* (exercice 4). La
commande de construction est inchangée :

```
gcc -std=c11 -O0 -g -Wall -Wextra snek.c -o snek
```

## Étape de code

Un seul changement pour cette leçon : `snek.c` remplace les branches d'entrée
par la table de commandes — codes de touche, commandes, et un répartiteur
balayeur. Son état final est étiqueté `lesson-024`.

```diff
diff --git a/sandbox/snek/snek.c b/sandbox/snek/snek.c
index 7b17290..d0ac1f7 100644
--- a/sandbox/snek/snek.c
+++ b/sandbox/snek/snek.c
@@ -1,6 +1,6 @@
 // snek.c — a terminal snake game, grown lesson by lesson.
 //
-// Lesson 023: the game — title, play, and death as explicit states.
+// Lesson 024: input dispatch — the function-pointer command table.
 #define _POSIX_C_SOURCE 200809L /* clock_gettime, nanosleep, termios: POSIX, not ISO C */
 #include <errno.h>
 #include <poll.h>
@@ -16,6 +16,13 @@ static const double FRAME_LEN = 1.0 / 30.0; /* frame cap: 30 frames per second *
 
 enum Direction { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
 enum GameState { TITLE, PLAY, DEAD };
+enum KeyCode {
+    KEY_NONE = 0,
+    KEY_UP = 256, /* named keys get codes no single byte can collide with */
+    KEY_DOWN,
+    KEY_LEFT,
+    KEY_RIGHT,
+};
 enum { GRID_ROWS = 20, GRID_COLS = 40 };
 enum { SNAKE_MAX = GRID_ROWS * GRID_COLS };
 
@@ -84,6 +91,7 @@ static void EnterRawMode(void)
         return; /* stdin is not a terminal (piped test input) — nothing to set */
     raw = saved_termios;
     raw.c_lflag &= ~(ICANON | ECHO);
+    raw.c_iflag &= ~ICRNL; /* Enter is CR: match the byte, not the translation */
     tcsetattr(STDIN_FILENO, TCSANOW, &raw); /* TCSANOW, not TCSAFLUSH: keep typed-ahead bytes */
     termios_saved = 1;
     atexit(RestoreTerminal);
@@ -168,35 +176,84 @@ static void AdvanceSnake(void)
         PlaceFood();
 }
 
-static void OnByte(unsigned char c)
+static void CmdQuit(void)
+{
+    running = 0;
+}
+
+static void CmdStart(void)
+{
+    if (state != PLAY)
+        StartGame();
+}
+
+static void CmdTurn(int new_dir)
+{
+    if (state != PLAY)
+        return;
+    /* a longer snake cannot reverse into its own neck */
+    if (snake_len > 1 &&
+        ((new_dir == DIR_UP && dir == DIR_DOWN) ||
+         (new_dir == DIR_DOWN && dir == DIR_UP) ||
+         (new_dir == DIR_LEFT && dir == DIR_RIGHT) ||
+         (new_dir == DIR_RIGHT && dir == DIR_LEFT)))
+        return;
+    dir = new_dir;
+}
+
+static void CmdUp(void) { CmdTurn(DIR_UP); }
+static void CmdDown(void) { CmdTurn(DIR_DOWN); }
+static void CmdLeft(void) { CmdTurn(DIR_LEFT); }
+static void CmdRight(void) { CmdTurn(DIR_RIGHT); }
+
+struct Command {
+    int key;
+    void (*run)(void);
+};
+
+static const struct Command commands[] = {
+    { 'q',        CmdQuit },
+    { ' ',        CmdStart },
+    { '\n',       CmdStart },
+    { KEY_UP,     CmdUp },
+    { KEY_DOWN,   CmdDown },
+    { KEY_LEFT,   CmdLeft },
+    { KEY_RIGHT,  CmdRight },
+};
+
+static void RunCommand(int key)
+{
+    for (size_t i = 0; i < sizeof commands / sizeof commands[0]; ++i) {
+        if (commands[i].key == key) {
+            commands[i].run();
+            return;
+        }
+    }
+}
+
+static int ParseByte(unsigned char c)
 {
     if (esc == 0) {
-        if (c == 0x1b)
+        if (c == 0x1b) {
             esc = 1;
-        else if (c == 'q')
-            running = 0;
-        else if (c == ' ' && state != PLAY)
-            StartGame();
-    } else if (esc == 1) {
-        esc = (c == '[') ? 2 : 0; /* a lone ESC eats the next byte */
-    } else {
-        esc = 0;
-        int new_dir = -1;
-        if (c == 'A') new_dir = DIR_UP;
-        else if (c == 'B') new_dir = DIR_DOWN;
-        else if (c == 'C') new_dir = DIR_RIGHT;
-        else if (c == 'D') new_dir = DIR_LEFT;
-        if (new_dir < 0 || state != PLAY)
-            return;
-        /* a longer snake cannot reverse into its own neck */
-        if (snake_len > 1 &&
-            ((new_dir == DIR_UP && dir == DIR_DOWN) ||
-             (new_dir == DIR_DOWN && dir == DIR_UP) ||
-             (new_dir == DIR_LEFT && dir == DIR_RIGHT) ||
-             (new_dir == DIR_RIGHT && dir == DIR_LEFT)))
-            return;
-        dir = new_dir;
+            return KEY_NONE;
+        }
+        return c; /* a plain key: its byte is its code */
     }
+    if (esc == 1) {
+        if (c == '[') {
+            esc = 2;
+            return KEY_NONE;
+        }
+        esc = 0; /* a lone ESC eats the next byte */
+        return KEY_NONE;
+    }
+    esc = 0;
+    if (c == 'A') return KEY_UP;
+    if (c == 'B') return KEY_DOWN;
+    if (c == 'C') return KEY_RIGHT;
+    if (c == 'D') return KEY_LEFT;
+    return KEY_NONE;
 }
 
 static void ProcessInput(void)
@@ -207,8 +264,11 @@ static void ProcessInput(void)
 
     unsigned char buf[64];
     ssize_t n = read(STDIN_FILENO, buf, sizeof buf);
-    for (ssize_t i = 0; i < n; ++i)
-        OnByte(buf[i]);
+    for (ssize_t i = 0; i < n; ++i) {
+        int key = ParseByte(buf[i]);
+        if (key != KEY_NONE)
+            RunCommand(key);
+    }
 }
 
 static void GridClear(void)
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — WASD, c'est quatre lignes *(extend-the-code)*

Liez les touches `w`, `a`, `s`, `d` à haut, gauche, bas, droite en n'utilisant
rien d'autre que la table de commandes — aucun changement de parseur, aucune
nouvelle fonction. Montrez les liaisons à l'œuvre avec une exécution minutée
qui presse chaque touche à son tour (un producteur comme
`( printf ' w'; sleep 0.4; printf 'a'; sleep 0.4; … ) | ./snek 60` narre les
virages dans la trace), et notez comment la taille du diff se compare à ce
qu'auraient coûté quatre branches de plus.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-024/ex1.md)

### Exercice 2 — Entrée est un retour chariot *(fix-the-crash)*

Sur un vrai terminal, appuyer sur Entrée sur l'écran titre ne fait rien — même
si vos tests par tube avec `'\n'` passent. Découvrez quel octet la touche
Entrée envoie réellement sur votre terminal et pourquoi le mode brut de cette
leçon le livre non traduit, puis corrigez la table de commandes pour que les
deux orthographes de « confirmer » fonctionnent. Démontrez la correction à
travers un pty avec la touche arrivant après que le mode brut est en place —
`script -qec` avec un producteur d'entrée retardé — et vérifiez que la trace
montre le démarrage de la partie.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-024/ex2.md)

### Exercice 3 — Deux lignes, une touche *(predict-the-output)*

Ajoutez une *seconde* ligne pour `'q'` en bas de la table, liée à une commande
qui imprime `second q row ran` sur stderr avant de quitter. Prédisez ce
qu'imprime `printf 'q' | ./snek 30`, et ce que la prédiction implique sur le
contrat entre `RunCommand` et la table — y compris ce qui changerait si la
nouvelle ligne était placée *au-dessus* de l'ancienne. Puis exécutez et
vérifiez. Que vous coûte ce contrat qu'un `switch` ne coûterait pas ?

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-024/ex3.md)

### Exercice 4 — Où vivent les fonctions *(explain-in-prose)*

Expliquez en prose ce que fait réellement `commands[i].run()` au niveau
machine : ce que contient un pointeur de fonction, en quoi se compile l'appel à
travers lui, et pourquoi le répartiteur peut appeler une fonction qu'il ne peut
pas nommer. Avant d'écrire, rendez les pointeurs visibles : imprimez
`commands[i].run` avec `%p` au site de répartition, exécutez quelques touches,
et comparez les adresses imprimées avec ce que `nm snek` rapporte pour
`CmdQuit`, `CmdStart` et `CmdUp`. Que prouvent les nombres, et quel rapport
avec les méthodes virtuelles C++ ?

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-024/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 023 — la machine à états : titre, jeu, mort](lesson-023-state-machine.md) ·
**Suivante :** [Leçon 025 — le sous-ensemble C++ : classes et vtables](lesson-025-cpp-subset.md) ·
**Étiquette de code :** [`lesson-024`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-024)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-024-command-table.md`, révision `bd716a9`.*

<!-- translation-source: book/lessons/part-0/lesson-024-command-table.md @ bd716a9 -->
