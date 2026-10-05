# Lesson 022 — the double-buffered character grid

{{#include ../../stability-horizon.md}}

## Prose

The program now knows what the player pressed; it still cannot show anything.
This lesson builds the display — and the way it is built is the point. A game
that draws straight to the screen spends every frame erasing and repainting
the whole world, and the player sees every intermediate state as flicker. The
fix is *double buffering*: compose the frame somewhere invisible, then move
only what changed onto the real display. Every engine does this; the
framebuffer of this course's title is exactly the "somewhere invisible".

`snek`'s invisible surface is a plain two-dimensional `char` array — the *back
buffer* — sized `GRID_ROWS` × `GRID_COLS`, one character per cell. A
terminal grid is the honest, lowest-tech framebuffer there is: pixels are
bytes, and drawing is array indexing. `GridClear` wipes the back buffer to
spaces and `GridPut` stores one character at a cell (bounds-checked, so a
piece of game logic one cell off the edge degrades to nothing instead of
corrupting memory). `Render` composes the scene there: a banner on the top
row, the marker — today's one-cell proto-snake, `@` — at its position. The
marker moves one cell per tick in the current direction and wraps at the
edges; that movement, from lesson 020's fixed timestep, is the game's first
animation.

The second array is the *front buffer*: it is not drawn to the terminal at
flush time in some clever way — it is simply a **claim about what the
terminal currently shows**. `GridFlush` walks both arrays cell by cell and
writes only the cells where the claim disagrees with the scene. Each such
cell is one cursor-position escape — `ESC [ row ; col H` moves the cursor to
a cell — followed by the character itself. When the walk finds nothing, the
frame costs zero bytes, and the terminal is left showing exactly what it
already showed: no flicker is possible when nothing is erased.

The claim needs one exception: before the first flush, `front` describes a
screen the program has never drawn to, so the flag `front_valid` is zero and
*every* cell counts as changed. That first flush clears the screen with
`ESC [ 2 J` and paints the whole grid; every flush after that is diff-only.
If anything else ever scribbles on the terminal — an error message, another
program — the claim is stale and the display corrupt until the buffers are
invalidated again. Holding that invariant honestly is most of the discipline
of double buffering.

Bytes matter here too. The diff flush is not just anti-flicker; it is
compression. A full redraw of a 20×40 grid is hundreds of cursor escapes per
frame; the diff is usually zero or two cells. Exercise 3 measures the ratio,
and exercise 4 squeezes the encoding further. One plumbing detail keeps the
bytes flowing: C's `stdout` is *buffered* — when output goes to a pipe or
file it accumulates in the C library until the buffer fills — so `GridFlush`
ends with `fflush(stdout)`. Without it the display arrives in chunks whenever
the buffer feels like it, and a redirected run produces a file in bursts.

Nothing about the game loop changed: `ProcessInput` reads keys, `Update`
advances ticks and the marker, `Render` now composes and flushes the grid
instead of printing a line. The trace still goes to stderr in test mode and
now reports the marker's cell as well, so `./snek 60 2>/dev/null` shows the
display's bytes alone while `./snek 60 2>&1 >/dev/null` shows the state
alone. The build command is unchanged:

```
gcc -std=c11 -O0 -g -Wall -Wextra snek.c -o snek
```

## Code step

One change for this lesson: `snek.c` grows the back and front buffers, the
grid draw/flush functions, and the moving marker. Its end state is tagged
`lesson-022`.

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

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — Counting the flush *(predict-the-output)*

Predict how many cells `GridFlush` writes on each of these flushes: the very
first flush of a run; a flush on a frame that falls between ticks; a flush on
a frame where the marker moved one cell. Then add one instrumenting counter
to `GridFlush` — `int cells` bumped per written cell, reported with
`fprintf(stderr, "flush cells=%d\n", cells);` at the end — run `./snek 10`,
and check every prediction against the real counts.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-022/ex1.md)

### Exercise 2 — One off, twice *(fix-the-crash)*

Run the game on a real terminal and the whole field is drawn one cell below
and to the right of where the banner and the marker should be — while the
byte stream contains cursor addresses like `ESC [ 0 ; 0 H` that no terminal
counts as legal. Find the addressing bug in `GridFlush`, and fix it at the
one place where the program's coordinates meet the terminal's — without
changing a single coordinate inside the program.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-022/ex2.md)

### Exercise 3 — What the diff saves *(measure-the-performance)*

Measure the double buffer's payoff. Run `./snek 60 2>/dev/null | wc -c` for
the diff's byte count. Then give `GridFlush` a switch — one `getenv` check
for `SNEK_FULL` that makes it treat every cell as changed — and measure the
same run with the switch on. How many times more bytes does the full redraw
cost at 60 frames, and how does that ratio move at 120? Where does the extra
cost come from, and what does the player see of it?

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-022/ex3.md)

### Exercise 4 — Runs, not cells *(extend-the-code)*

The flush pays one cursor-address escape per changed cell even when changed
cells sit next to each other. Extend `GridFlush` to send each *run* of
consecutive changed cells in one row as a single escape followed by the
characters — the terminal advances the cursor by itself as characters arrive.
Measure `./snek 60 2>/dev/null | wc -c` before and after. How much did the
first flush shrink, and why is that where the wins are?

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-022/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 021 — raw terminal input with escape codes](lesson-021-terminal-input.md) ·
**Next:** [Lesson 023 — the state machine: title, play, death](lesson-023-state-machine.md) ·
**Code tag:** [`lesson-022`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-022)
