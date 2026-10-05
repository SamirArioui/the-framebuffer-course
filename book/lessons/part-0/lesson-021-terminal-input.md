# Lesson 021 — raw terminal input with escape codes

{{#include ../../stability-horizon.md}}

## Prose

A terminal looks like a device with keys on it. It is not. It is a **byte
stream** that happens to be attached to a keyboard: when you press `a`, one
byte (`0x61`) arrives; when you press the up-arrow, *three* bytes arrive —
ESC, `[`, `A`. There is no "key pressed" event anywhere in the pipeline, and
the escape codes that draw the screen are made of the same stuff as the codes
that describe keys. This lesson teaches the loop to read that stream: raw
terminal mode, non-blocking reads, and a small state machine that turns
byte sequences back into keys.

Start with why the terminal seems to work against you. By default it is in
*canonical mode*: the line discipline buffers your typing until you press
Enter, echoes every character back, and handles backspace for you. That is
exactly right for a shell and exactly wrong for a game — `snek` needs each
key the instant it is pressed, without echo. The `termios` API is how you ask
for that: `tcgetattr` copies the terminal's current settings into a `struct
termios`, you adjust two flags — clear `ICANON` (no line buffering) and
`ECHO` (no echo) — and `tcsetattr` installs the modified struct. Two details
matter. `tcsetattr` takes a *when* argument: `TCSAFLUSH` applies the change
after discarding any typed-ahead input, `TCSANOW` applies immediately and
keeps it — the game uses `TCSANOW` on entry (bytes already typed are still
input) and `TCSAFLUSH` on the way out (junk typed during play should not
spill into the shell). And `tcgetattr` **fails** when stdin is not a terminal
— a pipe, in tests — and the program must take that gracefully: skip raw
mode, keep running. The piped tests you will run below depend on exactly
that branch.

Raw mode creates a debt: **the terminal must be restored on every exit
path**, or the shell is left without echo and the user thinks their terminal
is broken. The program pays the debt three ways. `RestoreTerminal` puts the
saved settings back and is called explicitly before `main` returns; the same
function is registered with `atexit`, so any future `exit()` path pays too;
and Ctrl-C is caught — in raw mode SIGINT still arrives from the line
discipline — with a handler that does the only thing that is legal inside a
signal handler: set a `volatile sig_atomic_t` flag. The loop notices the
flag and unwinds normally, restoring through the ordinary path. Restoring the
terminal *from inside the handler* is tempting and unsafe; the flag defers
the work to normal execution.

Reading input must never stall the loop: `ProcessInput` runs once per frame
and has a frame budget to keep. A blocking `read` would freeze the game until
a key arrives, so the code asks first — `poll` with a zero timeout reports
whether stdin has bytes waiting, and only then does `read` drain them. This
is the "check, then read" pattern; the alternative (`O_NONBLOCK` via `fcntl`
and tolerating `EAGAIN`) works too, but note that file status flags live in
the *open file description* shared with the parent shell — `poll` avoids the
question entirely. One `read` can return several keys at once (type fast and
they queue up), and it returns `0` at end of input — a piped run reaching EOF
just means no more keys; the game runs on.

Then the parser. Arrow keys are escape *sequences*, and bytes do not arrive
neatly labeled: `OnByte` is a three-state machine — idle, "after ESC",
"after ESC [" — that moves one byte at a time and emits a direction when the
sequence completes. Sequences can also split across reads (press a key while
the game is busy and watch `ESC [` arrive in one `read` and `A` in the
next), which is precisely why the state lives across calls. Exercise 1 will
show you the bytes themselves; exercise 2 has the parser's one real wart
waiting.

Two conventions change now that `q` exists. The frame budget is optional —
`./snek` runs until you quit, `./snek 30` runs at most 30 frames — and the
per-frame trace prints only in that budgeted **test mode**, keeping an
interactive run's screen clear for the display lesson 022 is about to build.
Interactive runs get one help line instead. For testing, pipes are the easy
path (`printf '\033[Aq' | ./snek 30` feeds an up-arrow and a `q`), and
`script -qec './snek' /dev/null` gives the program a real pty when you want
to see true terminal behavior.

The build command is unchanged:

```
gcc -std=c11 -O0 -g -Wall -Wextra snek.c -o snek
```

## Code step

One change for this lesson: `snek.c` grows raw terminal mode with its
restore paths, polled input, and the escape-sequence parser. Its end state is
tagged `lesson-021`.

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

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — Bytes all the way down *(predict-the-output)*

Predict what each of these runs prints — the trace lines, the exit line, and
what the program must have *seen* to produce them:

```
printf 'q' | ./snek 30
printf '\033[B' | ./snek 5
printf '\033' | ./snek 3
printf '\033[Aq' | ./snek 30
```

Then run all four and reconcile every difference. To see what the program
sees, add one instrumenting line in `ProcessInput`'s byte loop —
`fprintf(stderr, "byte=0x%02x\n", buf[i]);` before `OnByte` — and use its
output to settle the arguments, including the question of *which frame* the
bytes land on.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-021/ex1.md)

### Exercise 2 — The key that vanished *(fix-the-crash)*

`printf '\033q' | ./snek 30` runs all thirty frames: the `q` never quits the
game, even though `printf 'q' | ./snek 30` quits immediately. Find where the
byte goes in the escape parser, and fix `OnByte` so a key pressed right after
the Escape key is processed as a plain key instead of being swallowed — while
genuine arrow sequences keep working.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-021/ex2.md)

### Exercise 3 — CSI, SS3, and your terminal *(port-to-your-own-machine)*

Arrow keys have a second spelling: in "application cursor keys" mode — which
editors like vim switch on — a terminal sends `ESC O A` instead of
`ESC [ A`. Find out what *your* setup sends (`cat -v` shows `^[` for ESC;
`showkey -a` on Linux is made for this), then teach the parser to accept both
spellings so the game works in vim's terminal and outside it. On a machine
with a different terminal tradition (Windows console, a macOS SSH session),
check the sequences there too and note what else would have to change.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-021/ex3.md)

### Exercise 4 — WASD *(extend-the-code)*

Add the word keys `w`, `a`, `s`, `d` as aliases for up, left, down, right.
Keep the arrow handling untouched, and confirm both spellings work — including
in one run, `printf 'w\033[Csq' | ./snek 30`, which mixes them. How much code
does a plain key cost compared to an arrow key, and why?

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-021/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 020 — timing with `clock_gettime`](lesson-020-timing.md) ·
**Next:** [Lesson 022 — the double-buffered character grid](lesson-022-double-buffer.md) ·
**Code tag:** [`lesson-021`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-021)
