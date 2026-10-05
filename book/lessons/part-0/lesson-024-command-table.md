# Lesson 024 — the function-pointer command table

{{#include ../../stability-horizon.md}}

## Prose

The input code has a shape problem. Every key the game knows is an `else if`
somewhere inside `OnByte`, mixed together with parsing details and game
policy: adding a key means finding the right branch, and the answer to "what
does this game bind?" is spread across a function body. This lesson replaces
the branches with a **command table** — rows of data pairing keys with the
functions that act on them — and teaches the one C feature that makes such a
table possible: the *function pointer*. It is the C route to interfaces, and
the machine-level preview of the C++ virtual method that lesson 025 builds.

First, the type. In `void (*run)(void)`, read the name inside out: `run` is
a pointer (`*run`) to something that takes `(void)` and returns `void` —
the address of a function with that signature. `commands[i].run()` calls
through that address: the CPU jumps to whatever function the row holds. A
function pointer is an ordinary value — it can sit in a struct, in an array,
be passed around and compared — and the compiler only checks the *signature*,
not which function it will turn out to be. This is the seam the whole lesson
sits on: behavior stored as data.

The table itself is one struct and one array:

```c
struct Command {
    int key;
    void (*run)(void);
};
```

Each row binds a key code to a command function. `RunCommand` scans the rows
and runs the first whose key matches — the dispatch is now data lookup, and
the functions it calls (`CmdQuit`, `CmdStart`, `CmdUp`, …) are plain game
actions with no knowledge of keyboards. The four direction commands are
one-liners forwarding to a shared `CmdTurn`, because the function-pointer
signature is fixed at `void (void)` and a command that wants an argument
wraps it — the adapter idiom, and worth recognizing now, because C++'s
`std::function` and friends exist for exactly this friction.

For the table to be scannable, every key needs a code, and the codes must
not collide. Plain keys keep their bytes (`'q'` is 113, `' '` is 32);
*named* keys — the arrows — get `enum KeyCode` values starting at 256, past
every possible byte. The parser's job narrows accordingly: `ParseByte` now
*emits key codes* instead of performing actions — a finished arrow sequence
becomes `KEY_UP`, a plain byte becomes itself, `KEY_NONE` means "not a key
yet" — and `ProcessInput` feeds each emitted code to `RunCommand`. Parsing
and policy are finally separate: the state machine's guards (space only
outside `PLAY`, no reversing into the neck) moved into the command functions
where game rules belong.

One raw-mode detail tightens along the way: `EnterRawMode` now also clears
`ICRNL`, the line-discipline flag that rewrites CR into NL on input. With a
command table, the program matches the bytes the keyboard *sends*, and
silent translations are exactly the kind of surprise a table makes visible —
exercise 2 has the surprise waiting. The general shape deserves its own
sentence: when keys are rows of data, every key's true byte value becomes
part of your program's contract with the terminal.

Why is this the C route to interfaces? The scanner doesn't know which
function a row holds — pair data with behavior, swap the behavior without
touching the dispatch. That is an interface in everything but language
support, and when lesson 025 wraps this table in a C++ class with a virtual
method, the compiled shape barely changes: a table of function addresses,
one indirect call. What you write today in C is what the compiler will
generate for you tomorrow.

Adding a key is now a row — try exercise 1 and feel the difference. The scan
has one contract worth knowing cold (exercise 3), and the pointers can be
*inspected* (exercise 4). The build command is unchanged:

```
gcc -std=c11 -O0 -g -Wall -Wextra snek.c -o snek
```

## Code step

One change for this lesson: `snek.c` replaces the input branches with the
command table — key codes, commands, and a scanning dispatcher. Its end
state is tagged `lesson-024`.

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

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — WASD is four rows *(extend-the-code)*

Bind the word keys `w`, `a`, `s`, `d` to up, left, down, right using nothing
but the command table — no parser changes, no new functions. Show the
bindings working with a timed run that presses each key in turn (a producer
like `( printf ' w'; sleep 0.4; printf 'a'; sleep 0.4; … ) | ./snek 60`
narrates the turns in the trace), and note how the diff's size compares to
what four more branches would have cost.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-024/ex1.md)

### Exercise 2 — Enter is a carriage return *(fix-the-crash)*

On a real terminal, pressing Enter on the title screen does nothing — even
though your pipe tests with `'\n'` pass. Find out what byte the Enter key
actually sends on your terminal and why the raw mode of this lesson delivers
it untranslated, then fix the command table so both spellings of "confirm"
work. Demonstrate the fix through a pty with the key arriving after raw mode
is up — `script -qec` with a delayed input producer — and check the trace
shows the game starting.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-024/ex2.md)

### Exercise 3 — Two rows, one key *(predict-the-output)*

Add a *second* row for `'q'` at the bottom of the table, bound to a command
that prints `second q row ran` on stderr before quitting. Predict what
`printf 'q' | ./snek 30` prints, and what the prediction implies about the
contract between `RunCommand` and the table — including what would change if
the new row were placed *above* the old one. Then run it and check. What
does this contract cost you that a `switch` would not?

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-024/ex3.md)

### Exercise 4 — Where the functions live *(explain-in-prose)*

Explain in prose what `commands[i].run()` actually does at the machine
level: what a function pointer holds, what the call through it compiles to,
and why the dispatcher can call a function it cannot name. Before writing,
make the pointers visible: print `commands[i].run` with `%p` at the dispatch
site, run a few keys, and compare the printed addresses with what
`nm snek` reports for `CmdQuit`, `CmdStart`, and `CmdUp`. What do the
numbers prove, and what does it have to do with C++ virtual methods?

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-024/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 023 — the state machine: title, play, death](lesson-023-state-machine.md) ·
**Next:** [Lesson 025 — the C++ subset: classes and vtables](lesson-025-cpp-subset.md) ·
**Code tag:** [`lesson-024`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-024)
