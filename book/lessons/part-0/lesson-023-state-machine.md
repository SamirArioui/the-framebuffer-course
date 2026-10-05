# Lesson 023 — the state machine: title, play, death

{{#include ../../stability-horizon.md}}

## Prose

A game is not one flow of control — it is several, and the program is always
in exactly one of them. `snek` shows a title screen; then it plays; then the
snake dies and a game-over screen waits for a restart. These are **states**,
and today's code step makes them explicit: one `enum GameState { TITLE, PLAY,
DEAD }`, one `state` variable, and every part of the program asking "which
state are we in?" before doing anything state-specific. That is a state
machine, and it is how every game — and every parser, protocol, and UI —
keeps modes from collapsing into nested conditionals.

Why not flags? `int playing, dead;` *works* until it doesn't: after five
states there are combinations no flag test excludes (`playing && dead`),
every input handler must reason about all of them, and the answer to "what
does space do?" lives in scattered `if`s. With one `state` variable the legal
combinations are exactly the declared ones, each transition is a single
assignment you can find with a search, and `Render` becomes one branch per
state drawing one complete screen each. The machine is also *testable*: the
trace prints `state=…` every frame, so a scripted run shows the transitions
as data.

The state decides everything. `Update` advances the snake per tick only in
`PLAY` — in `TITLE` and `DEAD` the game world holds still while the loop and
the clock carry on. `OnByte` routes keys by state: `q` always quits, space
starts a game from `TITLE` or restarts from `DEAD` (anything but `PLAY`), and
arrows steer only while playing. Note one rule baked into the steering: a
snake longer than one segment may not reverse into its own neck — the input
is checked against the current direction and *ignored* if it is a 180° turn.
That is game rules expressed as input validation; letting the reversal
through would kill the player on the next tick.

Under the rules sits the game data, all of it plain arrays — the C idiom for
a game object. The snake is two `int` arrays of row/column pairs plus a
length, segment 0 being the head; movement is a shift: check the *proposed*
head cell first (a wall ends the game, a body cell ends the game), then
rewrite the array — every segment takes its predecessor's cell, the head
takes the new one, and the old tail is simply overwritten. Eating food grows
the snake by one (the shift copies one extra segment, so the tail survives),
bumps `score`, and places new food. `PlaceFood` scans from a pseudo-random
start for the first free interior cell, using a tiny linear congruential
generator seeded with a *fixed* number — deliberately, so every run of
`./snek 150` places identical food and test traces are reproducible. A
shipped game would seed from the clock; a tested game seeds from a constant.

`StartGame` builds a fresh three-segment snake at the middle of the field,
ready to move right — a new life is a full reset of the world the machine
manages. (How full, exactly, is exercise 2's business.) The death screen
keeps the final board visible under its message — death is a state with a
picture, not an exit.

Testing the whole machine means feeding input over *time*: the shell is
happy to be a scripted player, because a pipeline producer can decide when
each key arrives — `( printf ' \033[A'; sleep 3.5; printf ' ' ) | ./snek 150`
starts the game, steers up, and presses space again only after the snake has
died. The trace then narrates the machine's life: state, score, direction,
and the head's cell each frame. Exercise 1 predicts one such story before
you run it.

The build command is unchanged:

```
gcc -std=c11 -O0 -g -Wall -Wextra snek.c -o snek
```

## Code step

One change for this lesson: `snek.c` grows the game — states, snake, food,
score, and the rules that move them. Its end state is tagged `lesson-023`.

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

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — Counting to the wall *(predict-the-output)*

Predict the trace of `printf ' \033[A' | ./snek 60` from the geometry alone:
the space starts a game, the up-arrow turns the snake, and the wall check in
`AdvanceSnake` decides the ending. At which **tick** does the snake die, and
why that one? Which cell does the head freeze on? Write the prediction down
before running anything. Then run it and account for every tick of
difference. To watch each step as it is attempted, add one instrumenting line
at the top of `AdvanceSnake` — `fprintf(stderr, "head %d,%d\n", new_row,
new_col);` — and use it to settle any argument about move counts.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-023/ex1.md)

### Exercise 2 — The restart that wasn't *(fix-the-crash)*

Play one life, die, and press space again:

```
( printf ' \033[A'; sleep 3.5; printf ' ' ) | ./snek 150
```

The restart brings the snake back — but it dies almost exactly the way it
died the first time, as if the second life were still living the first one's
ending. Find the game field that survives the "fresh start" in `StartGame`,
fix the reset, and rerun the same script to show the difference in the
trace. (The producer needs its `sleep`: the second space must arrive while
the game is on the death screen.)

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-023/ex2.md)

### Exercise 3 — Pause *(extend-the-code)*

Add a `PAUSE` state toggled with `p`: while paused the snake freezes where
it is and the grid says so; `p` again resumes play. Pause must not leak into
the other states — `p` on the title or death screen does nothing. Show the
freeze working with a timed script like
`( printf ' p'; sleep 1.5; printf 'p' ) | ./snek 75`, and explain what
happened to the tick counter while the snake sat still.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-023/ex3.md)

### Exercise 4 — Why states, not flags *(explain-in-prose)*

Explain in prose why this game keeps one `state` variable instead of
independent `playing`/`dead`/`paused` flags: what combinations flags make
expressible, what a transition costs in each design, and how the next feature
would land in each. Before writing, make the transitions visible: wrap every
state change in a `SetState` helper that logs
`state <old> -> <new>` on stderr, run a play-die-restart script, and describe
what the log shows about the machine's shape.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-023/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 022 — the double-buffered character grid](lesson-022-double-buffer.md) ·
**Next:** [Lesson 024 — the function-pointer command table](lesson-024-command-table.md) ·
**Code tag:** [`lesson-023`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-023)
