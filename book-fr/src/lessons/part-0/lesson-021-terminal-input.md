# Leçon 021 — la saisie brute au terminal avec les codes d'échappement

{{#include ../../stability-horizon.md}}

## Prose

Un terminal ressemble à un appareil avec des touches dessus. Ce n'en est pas
un. C'est un **flux d'octets** qui se trouve être attaché à un clavier : quand
vous appuyez sur `a`, un octet (`0x61`) arrive ; quand vous appuyez sur la
flèche du haut, *trois* octets arrivent — ESC, `[`, `A`. Il n'y a
d'événement « touche pressée » nulle part dans le pipeline, et les codes
d'échappement qui dessinent l'écran sont faits de la même matière que les
codes qui décrivent les touches. Cette leçon apprend à la boucle à lire ce
flux : mode terminal brut, lectures non bloquantes, et une petite machine à
états qui retransforme les séquences d'octets en touches.

Commençons par pourquoi le terminal semble travailler contre vous. Par défaut
il est en *mode canonique* : la discipline de ligne met en tampon votre frappe
jusqu'à ce que vous appuyiez sur Entrée, renvoie chaque caractère, et gère le
retour arrière pour vous. C'est exactement juste pour un shell et exactement
faux pour un jeu — `snek` a besoin de chaque touche à l'instant où elle est
pressée, sans écho. L'API `termios` est la façon de demander cela :
`tcgetattr` copie les réglages actuels du terminal dans une
`struct termios`, vous ajustez deux drapeaux — effacer `ICANON` (pas de mise
en tampon par ligne) et `ECHO` (pas d'écho) — et `tcsetattr` installe la
structure modifiée. Deux détails comptent. `tcsetattr` prend un argument
*quand* : `TCSAFLUSH` applique le changement après avoir jeté toute entrée
tapée d'avance, `TCSANOW` applique immédiatement et la garde — le jeu utilise
`TCSANOW` à l'entrée (les octets déjà tapés sont encore de l'entrée) et
`TCSAFLUSH` à la sortie (les saletés tapées pendant le jeu ne doivent pas
déborder dans le shell). Et `tcgetattr` **échoue** quand stdin n'est pas un
terminal — un tube, dans les tests — et le programme doit le prendre avec
grâce : sauter le mode brut, continuer. Les tests par tube que vous lancerez
ci-dessous dépendent d'exactement cette branche.

Le mode brut crée une dette : **le terminal doit être restauré sur chaque
chemin de sortie**, sinon le shell reste sans écho et l'utilisateur croit que
son terminal est cassé. Le programme paie la dette de trois façons.
`RestoreTerminal` remet les réglages sauvegardés et est appelée explicitement
avant que `main` ne renvoie ; la même fonction est enregistrée avec `atexit`,
pour que tout futur chemin `exit()` paie aussi ; et Ctrl-C est attrapé — en
mode brut, SIGINT arrive toujours de la discipline de ligne — avec un
gestionnaire qui fait la seule chose légale dans un gestionnaire de signal :
poser un drapeau `volatile sig_atomic_t`. La boucle remarque le drapeau et se
déroule normalement, restaurant par le chemin ordinaire. Restaurer le terminal
*de l'intérieur du gestionnaire* est tentant et dangereux ; le drapeau diffère
le travail vers l'exécution normale.

Lire l'entrée ne doit jamais bloquer la boucle : `ProcessInput` tourne une fois
par frame et a un budget de frame à tenir. Un `read` bloquant gèlerait le jeu
jusqu'à l'arrivée d'une touche, aussi le code demande-t-il d'abord — `poll`
avec un délai de zéro rapporte si stdin a des octets en attente, et seulement
alors `read` les draine. C'est le motif « vérifier, puis lire » ; l'alternative
(`O_NONBLOCK` via `fcntl` et tolérer `EAGAIN`) fonctionne aussi, mais notez
que les drapeaux d'état de fichier vivent dans la *description de fichier
ouverte* partagée avec le shell parent — `poll` évite la question entièrement.
Un seul `read` peut renvoyer plusieurs touches à la fois (tapez vite et elles
s'empilent), et il renvoie `0` à la fin de l'entrée — une exécution par tube
qui atteint EOF veut juste dire plus de touches ; le jeu continue.

Vient ensuite le parseur. Les flèches sont des *séquences* d'échappement, et
les octets n'arrivent pas étiquetés proprement : `OnByte` est une machine à
trois états — repos, « après ESC », « après ESC [ » — qui avance un octet à
la fois et émet une direction quand la séquence se complète. Les séquences
peuvent aussi se scinder entre deux lectures (appuyez sur une touche pendant
que le jeu est occupé et regardez `ESC [` arriver dans un `read` et `A` dans
le suivant), ce qui est précisément pourquoi l'état vit à travers les appels.
L'exercice 1 vous montrera les octets eux-mêmes ; l'exercice 2 tient la seule
vraie verrue du parseur.

Deux conventions changent maintenant que `q` existe. Le budget de frames est
facultatif — `./snek` tourne jusqu'à ce que vous quittiez, `./snek 30` tourne
au plus 30 frames — et la trace par frame ne s'imprime que dans ce **mode
test** budgété, gardant l'écran d'une exécution interactive propre pour
l'affichage que la leçon 022 s'apprête à construire. Les exécutions
interactives obtiennent une ligne d'aide à la place. Pour les tests, les tubes
sont le chemin facile (`printf '\033[Aq' | ./snek 30` fournit une flèche du
haut et un `q`), et `script -qec './snek' /dev/null` donne au programme un vrai
pty quand vous voulez voir le vrai comportement d'un terminal.

La commande de construction est inchangée :

```
gcc -std=c11 -O0 -g -Wall -Wextra snek.c -o snek
```

## Étape de code

Un seul changement pour cette leçon : `snek.c` gagne le mode terminal brut avec
ses chemins de restauration, la saisie scrutée par `poll`, et le parseur de
séquences d'échappement. Son état final est étiqueté `lesson-021`.

```diff
diff --git a/sandbox/snek/snek.c b/sandbox/snek/snek.c
index 7c77efd..251f6ef 100644
--- a/sandbox/snek/snek.c
+++ b/sandbox/snek/snek.c
@@ -1,21 +1,36 @@
 // snek.c — a terminal snake game, grown lesson by lesson.
 //
-// Lesson 020: timing — clock_gettime, a fixed timestep, and a frame cap.
-#define _POSIX_C_SOURCE 200809L /* clock_gettime and nanosleep are POSIX, not ISO C */
+// Lesson 021: raw terminal input — termios, poll, and escape sequences.
+#define _POSIX_C_SOURCE 200809L /* clock_gettime, nanosleep, termios: POSIX, not ISO C */
 #include <errno.h>
+#include <poll.h>
+#include <signal.h>
 #include <stdio.h>
 #include <stdlib.h>
+#include <termios.h>
 #include <time.h>
+#include <unistd.h>
 
 static const double TICK_LEN = 1.0 / 10.0; /* fixed timestep: 10 updates per second */
 static const double FRAME_LEN = 1.0 / 30.0; /* frame cap: 30 frames per second */
 
+enum Direction { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
+
+static const char *dir_names[] = {"up", "down", "left", "right"};
+
 static int running;              /* the loop runs while this is true */
 static unsigned long frame;      /* frames since the loop started */
 static unsigned long tick;       /* game updates since the loop started */
 static double tick_accum;        /* seconds of game time not yet ticked away */
 static double frame_dt;          /* measured length of the current frame */
-static unsigned long max_frames; /* stop after this many frames */
+static unsigned long max_frames; /* stop after this many frames (0: until q) */
+static int test_mode;            /* a frame budget was given: trace every frame */
+static int dir = DIR_RIGHT;      /* where the snake is heading */
+static int esc;                  /* escape-sequence parser state */
+
+static struct termios saved_termios;
+static int termios_saved;
+static volatile sig_atomic_t interrupted;
 
 static double Now(void)
 {
@@ -32,9 +47,62 @@ static void SleepSec(double sec)
     nanosleep(&ts, NULL);
 }
 
+static void RestoreTerminal(void)
+{
+    if (termios_saved) {
+        tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_termios);
+        termios_saved = 0;
+    }
+}
+
+static void OnInterrupt(int sig)
+{
+    (void)sig;
+    interrupted = 1;
+}
+
+static void EnterRawMode(void)
+{
+    struct termios raw;
+
+    if (tcgetattr(STDIN_FILENO, &saved_termios) != 0)
+        return; /* stdin is not a terminal (piped test input) — nothing to set */
+    raw = saved_termios;
+    raw.c_lflag &= ~(ICANON | ECHO);
+    tcsetattr(STDIN_FILENO, TCSANOW, &raw); /* TCSANOW, not TCSAFLUSH: keep typed-ahead bytes */
+    termios_saved = 1;
+    atexit(RestoreTerminal);
+    signal(SIGINT, OnInterrupt);
+}
+
+static void OnByte(unsigned char c)
+{
+    if (esc == 0) {
+        if (c == 0x1b)
+            esc = 1;
+        else if (c == 'q')
+            running = 0;
+    } else if (esc == 1) {
+        esc = (c == '[') ? 2 : 0; /* a lone ESC eats the next byte */
+    } else {
+        esc = 0;
+        if (c == 'A') dir = DIR_UP;
+        else if (c == 'B') dir = DIR_DOWN;
+        else if (c == 'C') dir = DIR_RIGHT;
+        else if (c == 'D') dir = DIR_LEFT;
+    }
+}
+
 static void ProcessInput(void)
 {
-    // No keyboard yet — lesson 021 teaches the terminal.
+    struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
+    if (poll(&pfd, 1, 0) <= 0)
+        return;
+
+    unsigned char buf[64];
+    ssize_t n = read(STDIN_FILENO, buf, sizeof buf);
+    for (ssize_t i = 0; i < n; ++i)
+        OnByte(buf[i]);
 }
 
 static void Update(double dt)
@@ -45,34 +113,43 @@ static void Update(double dt)
         ++tick;
     }
     ++frame;
-    if (frame >= max_frames)
+    if (max_frames > 0 && frame >= max_frames)
         running = 0;
 }
 
 static void Render(void)
 {
-    fprintf(stderr, "frame=%lu tick=%lu dt=%.4f\n", frame, tick, frame_dt);
+    if (test_mode)
+        fprintf(stderr, "frame=%lu tick=%lu dir=%s\n", frame, tick,
+                dir_names[dir]);
 }
 
 int main(int argc, char **argv)
 {
-    if (argc != 2) {
-        fprintf(stderr, "usage: %s FRAMES\n", argv[0]);
+    if (argc > 2) {
+        fprintf(stderr, "usage: %s [FRAMES]\n", argv[0]);
         return 1;
     }
-
-    char *end;
-    errno = 0;
-    max_frames = strtoul(argv[1], &end, 10);
-    if (*end != '\0' || max_frames == 0) {
-        fprintf(stderr, "%s: FRAMES must be a positive integer, got '%s'\n",
-                argv[0], argv[1]);
-        return 1;
+    if (argc == 2) {
+        char *end;
+        errno = 0;
+        max_frames = strtoul(argv[1], &end, 10);
+        if (argv[1][0] == '-' || *end != '\0' || max_frames == 0 ||
+            errno == ERANGE) {
+            fprintf(stderr, "%s: FRAMES must be a positive integer, got '%s'\n",
+                    argv[0], argv[1]);
+            return 1;
+        }
+        test_mode = 1;
     }
 
+    EnterRawMode();
+    if (!test_mode)
+        fprintf(stderr, "snek — arrows to steer, q to quit\n");
+
     running = 1;
     double prev = Now();
-    while (running) {
+    while (running && !interrupted) {
         double frame_start = Now();
         frame_dt = frame_start - prev;
         prev = frame_start;
@@ -86,6 +163,7 @@ int main(int argc, char **argv)
             SleepSec(rem);
     }
 
+    RestoreTerminal();
     fprintf(stderr, "done after %lu frames, %lu ticks\n", frame, tick);
     return 0;
 }
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Des octets jusqu'au bout *(predict-the-output)*

Prédisez ce qu'imprime chacune de ces exécutions — les lignes de trace, la
ligne de sortie, et ce que le programme a dû *voir* pour les produire :

```
printf 'q' | ./snek 30
printf '\033[B' | ./snek 5
printf '\033' | ./snek 3
printf '\033[Aq' | ./snek 30
```

Puis lancez les quatre et accordez chaque différence. Pour voir ce que le
programme voit, ajoutez une ligne d'instrumentation dans la boucle d'octets de
`ProcessInput` — `fprintf(stderr, "byte=0x%02x\n", buf[i]);` avant `OnByte` —
et servez-vous de sa sortie pour régler les débats, y compris la question de
*sur quelle frame* les octets atterrissent.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-021/ex1.md)

### Exercice 2 — La touche qui a disparu *(fix-the-crash)*

`printf '\033q' | ./snek 30` fait tourner les trente frames : le `q` ne quitte
jamais le jeu, alors que `printf 'q' | ./snek 30` quitte immédiatement.
Trouvez où va l'octet dans le parseur d'échappement, et corrigez `OnByte` pour
qu'une touche pressée juste après la touche Échap soit traitée comme une
touche ordinaire au lieu d'être avalée — pendant que les vraies séquences de
flèches continuent de fonctionner.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-021/ex2.md)

### Exercice 3 — CSI, SS3, et votre terminal *(port-to-your-own-machine)*

Les flèches ont une seconde orthographe : en mode « touches de curseur
applicatives » — que des éditeurs comme vim activent — un terminal envoie
`ESC O A` au lieu de `ESC [ A`. Découvrez ce que *votre* installation envoie
(`cat -v` montre `^[` pour ESC ; `showkey -a` sur Linux est fait pour cela),
puis apprenez au parseur à accepter les deux orthographes pour que le jeu
fonctionne dans le terminal de vim et en dehors. Sur une machine avec une
autre tradition de terminal (console Windows, session SSH macOS), vérifiez
aussi les séquences là-bas et notez ce qui d'autre devrait changer.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-021/ex3.md)

### Exercice 4 — WASD *(extend-the-code)*

Ajoutez les touches `w`, `a`, `s`, `d` comme alias de haut, gauche, bas,
droite. Laissez la gestion des flèches intacte, et confirmez que les deux
orthographes fonctionnent — y compris dans une seule exécution,
`printf 'w\033[Csq' | ./snek 30`, qui les mélange. Combien de code coûte une
touche ordinaire comparée à une touche de flèche, et pourquoi ?

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-021/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 020 — la mesure du temps avec `clock_gettime`](lesson-020-timing.md) ·
**Suivante :** [Leçon 022 — la grille de caractères à double tampon](lesson-022-double-buffer.md) ·
**Étiquette de code :** [`lesson-021`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-021)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-021-terminal-input.md`, révision `2369864`.*

<!-- translation-source: book/lessons/part-0/lesson-021-terminal-input.md @ 2369864 -->
