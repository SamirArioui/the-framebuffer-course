# Leçon 020 — la mesure du temps avec `clock_gettime`

{{#include ../../stability-horizon.md}}

## Prose

La boucle de la leçon 019 n'avait aucune notion de vitesse : les frames
passaient aussi vite que le CPU pouvait les imprimer, et rien dans le programme
ne savait combien de temps une durait. Un jeu est différent — le serpent doit
traverser le terrain dans le même nombre de secondes sur une machine rapide et
sur une lente. Cette leçon donne une horloge à la boucle : comment C mesure le
temps, ce qu'est le *delta time* et pourquoi la logique de jeu s'y intègre, et
la discipline du pas fixe qui garde la vitesse d'un jeu stable.

Mesurer le temps commence avec `clock_gettime`, qui remplit une
`struct timespec` — deux entiers, `tv_sec` et `tv_nsec`, secondes plus
nanosecondes. Deux détails de cet appel sont du cours à part entière. D'abord,
un pli de la chaîne d'outils : `clock_gettime` est POSIX, pas ISO C, et
`-std=c11` cache délibérément tout ce que la norme ne décrit pas. La correction
est une *macro de test de fonctionnalité* au tout début du fichier, avant
tout `#include` :

```c
#define _POSIX_C_SOURCE 200809L
```

Cela dit aux en-têtes de la bibliothèque C « exposez aussi les déclarations
POSIX.1-2008 ». Sans elle, la construction est un mur d'avertissements de
déclaration implicite et un retour implicite `int` qui corrompt silencieusement
la mesure. (L'alternative est `-std=gnu11`, qui ouvre aussi tout le GNU ;
épingler la norme et déclarer ce dont nous avons besoin garde le langage
honnête.) Ensuite, `clock_gettime` prend un *identifiant d'horloge*, et le
choix compte : `CLOCK_REALTIME` est le temps civil — des secondes depuis
l'époque, que le NTP, un administrateur, ou une réparation d'horloge après
double démarrage peuvent faire sauter en avant **ou en arrière** à tout
moment. `CLOCK_MONOTONIC` est le temps depuis un point fixe arbitraire et est
garanti de ne bouger qu'en avant à un rythme régulier. La mesure du temps d'un
jeu mesure des durées, aussi utilise-t-elle l'horloge monotone — une horloge
qui peut sauter en arrière peut vous remettre une frame négative.

La durée d'une frame est le *delta time*, `dt` : l'écart d'horloge murale
entre le début d'une frame et le suivant. La logique de jeu s'intègre sur dt —
la position avance de `speed * dt`, pas d'une quantité fixe par frame — pour
que le monde évolue au même rythme quel que soit le nombre de frames par
seconde que la machine produit. Avancez d'une quantité fixe par frame et le
serpent bouge deux fois plus vite sur un écran 120 Hz que sur un 60 Hz ;
avancez de `speed * dt` et les deux prennent le même temps de traversée.
Intégrez le dt brut par frame, en revanche, et un blocage de deux secondes —
un point d'arrêt dans `gdb`, disons — donne au serpent un saut de deux
secondes droit à travers un mur.

La correction pour cela est le **pas fixe** : la logique de jeu ne voit pas du
tout le dt réel. Elle tourne en quanta d'un pas fixe — ici `TICK_LEN`, 100 ms,
donnant dix *mises à jour par seconde* — et un petit accumulateur convertit le
temps réel écoulé en ticks. Chaque frame ajoute son dt à `tick_accum` ; chaque
`TICK_LEN` complet qui s'y trouve est drainé comme un tick. Le temps réel est
désormais mis en quarantaine dans l'accumulateur, où un blocage signifie
simplement que plusieurs ticks se déclenchent coup sur coup (rattrapage,
borné), et la vitesse de la simulation est une constante du programme :
`TICK_LEN` est la loi du jeu, pas un comportement du matériel. La trace que
vous imprimez par frame rapporte les deux compteurs — `frame` pour les tours
de boucle, `tick` pour les mises à jour du jeu — plus le dt mesuré, et les
deux comptes divergent exprès.

Dernière pièce : le **plafond de frames**. Rien dans la boucle ci-dessus ne la
ralentit ; une boucle occupée tournerait à des milliers de frames par seconde
et brûlerait un cœur à dessiner la même image en boucle. `FRAME_LEN` fixe une
durée de frame minimale (ici 1/30 s) et la boucle dort le reste du budget de
chaque frame avec `nanosleep` — l'argument est une `struct timespec`, et
dormir le *reste* (jamais une quantité fixe) fait que le rythme reste juste
même si les frames elles-mêmes font du vrai travail. Le plafond est pourquoi
une exécution bornée est aussi un minuteur : avec 30 frames par seconde et 30
frames à faire, `./snek 30` est fini en environ une seconde, et
`time ./snek 30` le prouve. Prédisez les détails avant de l'exécuter —
l'exercice 1 attend.

La commande de construction est inchangée depuis la leçon 019 :

```
gcc -std=c11 -O0 -g -Wall -Wextra snek.c -o snek
```

## Étape de code

Un seul changement pour cette leçon : `snek.c` gagne l'horloge monotone,
l'accumulateur de pas fixe, et le plafond de frames. Son état final est
étiqueté `lesson-020`.

```diff
diff --git a/sandbox/snek/snek.c b/sandbox/snek/snek.c
index a5bb4a5..7c77efd 100644
--- a/sandbox/snek/snek.c
+++ b/sandbox/snek/snek.c
@@ -1,21 +1,49 @@
 // snek.c — a terminal snake game, grown lesson by lesson.
 //
-// Lesson 019: the game loop — ProcessInput, Update, Render, frame by frame.
+// Lesson 020: timing — clock_gettime, a fixed timestep, and a frame cap.
+#define _POSIX_C_SOURCE 200809L /* clock_gettime and nanosleep are POSIX, not ISO C */
 #include <errno.h>
 #include <stdio.h>
 #include <stdlib.h>
+#include <time.h>
+
+static const double TICK_LEN = 1.0 / 10.0; /* fixed timestep: 10 updates per second */
+static const double FRAME_LEN = 1.0 / 30.0; /* frame cap: 30 frames per second */
 
 static int running;              /* the loop runs while this is true */
 static unsigned long frame;      /* frames since the loop started */
+static unsigned long tick;       /* game updates since the loop started */
+static double tick_accum;        /* seconds of game time not yet ticked away */
+static double frame_dt;          /* measured length of the current frame */
 static unsigned long max_frames; /* stop after this many frames */
 
+static double Now(void)
+{
+    struct timespec ts;
+    clock_gettime(CLOCK_MONOTONIC, &ts);
+    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
+}
+
+static void SleepSec(double sec)
+{
+    struct timespec ts;
+    ts.tv_sec = (time_t)sec;
+    ts.tv_nsec = (long)((sec - (double)ts.tv_sec) * 1e9);
+    nanosleep(&ts, NULL);
+}
+
 static void ProcessInput(void)
 {
     // No keyboard yet — lesson 021 teaches the terminal.
 }
 
-static void Update(void)
+static void Update(double dt)
 {
+    tick_accum += dt;
+    while (tick_accum >= TICK_LEN) {
+        tick_accum -= TICK_LEN;
+        ++tick;
+    }
     ++frame;
     if (frame >= max_frames)
         running = 0;
@@ -23,7 +51,7 @@ static void Update(void)
 
 static void Render(void)
 {
-    fprintf(stderr, "frame=%lu\n", frame);
+    fprintf(stderr, "frame=%lu tick=%lu dt=%.4f\n", frame, tick, frame_dt);
 }
 
 int main(int argc, char **argv)
@@ -43,12 +71,21 @@ int main(int argc, char **argv)
     }
 
     running = 1;
+    double prev = Now();
     while (running) {
+        double frame_start = Now();
+        frame_dt = frame_start - prev;
+        prev = frame_start;
+
         ProcessInput();
-        Update();
+        Update(frame_dt);
         Render();
+
+        double rem = FRAME_LEN - (Now() - frame_start);
+        if (rem > 0)
+            SleepSec(rem);
     }
 
-    fprintf(stderr, "done after %lu frames\n", frame);
+    fprintf(stderr, "done after %lu frames, %lu ticks\n", frame, tick);
     return 0;
 }
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Prédire les ticks *(predict-the-output)*

Avant d'exécuter quoi que ce soit, écrivez deux nombres pour `./snek 30` : la
durée d'horloge murale que vous attendez (vérifiez-la avec `time ./snek 30`),
et la valeur finale de `tick` dans la ligne `done after …`. Puis exécutez-le
et rendez compte de chaque frame d'écart entre votre prédiction et la machine.
Pour voir le mécanisme frame par frame, ajoutez une ligne d'instrumentation
dans la boucle de ticks — `fprintf(stderr, "tick=%lu accum=%.4f\n", tick, tick_accum);`
après `++tick;` — et servez-vous de sa sortie pour expliquer exactement quand
les ticks se déclenchent et à quoi ressemble la contribution de la première
frame.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-020/ex1.md)

### Exercice 2 — Ce que coûte le plafond de frames *(measure-the-performance)*

Mesurez ce que fait réellement `FRAME_LEN`. Chronométrez `./snek 100` avec le
plafond en place ; puis désactivez le plafond — mettre le budget de frames à
zéro suffit — et chronométrez à nouveau. Lancez aussi `./snek 100000` sans
plafond et regardez à la fois le temps écoulé et le compte de `tick`. Combien
de temps mural le plafond a-t-il ajouté, pourquoi le `dt` change-t-il si
radicalement, et pourquoi le compte de ticks ne suit-il *pas* le compte de
frames quand le plafond est éteint ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-020/ex2.md)

### Exercice 3 — Deux horloges *(explain-in-prose)*

Expliquez en prose pourquoi le jeu mesure son dt avec `CLOCK_MONOTONIC` et pas
`CLOCK_REALTIME` : ce que chaque horloge garantit, ce qu'une correction NTP
fait à chacune, et ce qu'un dt négatif ferait à l'accumulateur et au jeu
au-delà. Pour comparer les horloges côte à côte avant d'écrire, étendez
`Render` avec deux appels à `clock_gettime` — un par horloge — et imprimez les
deux lectures à chaque frame ; faites tourner quelques secondes de frames et
décrivez ce que les deux nombres comptent réellement.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-020/ex3.md)

### Exercice 4 — Une chose qui bouge *(extend-the-code)*

Donnez au jeu sa première pièce de mouvement : une variable `pos` qui s'intègre
sur le pas fixe à une cellule par seconde (`pos += 1.0 * TICK_LEN;` dans la
boucle de ticks), imprimée dans la trace. Exécutez `./snek 30` et confirmez que
la position est une fonction des ticks seuls ; puis lancez une construction
sans plafond de l'exercice 2 pendant quelques millions de frames et confirmez
la même chose. C'est exactement ainsi que le serpent bougera dans la leçon 022.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-020/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 019 — la boucle de jeu](lesson-019-game-loop.md) ·
**Suivante :** [Leçon 021 — la saisie brute au terminal avec les codes d'échappement](lesson-021-terminal-input.md) ·
**Étiquette de code :** [`lesson-020`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-020)

*Page traduite de la version anglaise `book/lessons/part-0/lesson-020-timing.md`,
révision `203c219`.*

<!-- translation-source: book/lessons/part-0/lesson-020-timing.md @ 203c219 -->
