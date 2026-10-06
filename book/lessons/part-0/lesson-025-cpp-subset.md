# Lesson 025 — the C++ subset: classes and vtables

{{#include ../../stability-horizon.md}}

## Prose

This is Part 0's last lesson, and it hands the sandbox over to the language
the engine is born in. From lesson 019 to 024 you wrote `snek` in C; the
engine at Part 1 is C++ — but only a subset of it, chosen by one policy: **a
language feature is admitted only if we can explain what it compiles down
to**. Exactly five features pass: *references*, *overloading*, *namespaces*,
*constexpr*, and *classes with vtables*. Templates are never admitted into
game code. Exceptions, the STL, `new`/`delete` churn — outside until the
policy says otherwise. Today's lesson admits all five at once by converting
`snek` in place, and each feature earns its place by compiling down to
something you have already built by hand.

The game does not change. Same title screen, same deterministic death at tick
9 with the scripted input, same `q` that quits. What changes is the language
of the file it lives in: `snek.c` becomes `snek.cpp`, and the build command
becomes:

```
g++ -std=c++17 -O0 -g -Wall -Wextra *.cpp -o snek
```

Read it against lesson 001's command flag by flag. `g++` runs the same
pipeline — preprocessor, compiler, assembler, linker — with a C++ compiler in
the middle; it is `gcc`'s sibling, not a different kind of tool (`clang++` is
the same idea from the other vendor). `-std=c++17` pins the language version
the way `-std=c11` did: the compiler agrees with this book about what the
code means. `-O0 -g -Wall -Wextra` mean exactly what they meant for `snek.c`
— no optimization, debug information, warnings as curriculum — and the build
stays at zero warnings. `*.cpp` links every C++ source in the directory into
one program.

**Namespaces.** Everything the game is made of — every function, every class,
the game state — now lives in `namespace snek`. What does that compile down
to? Qualified names, and nothing else. The linker sees the namespace as part
of the name:

```
$ nm snek | grep RunEiPPc
0000000000002386 T _ZN4snek3RunEiPPc
```

Read the mangled name as a length-prefixed path: `4snek` is the namespace,
`3Run` is the function. At runtime a qualified name costs exactly what an
unqualified one costs — nothing; the qualification happens at compile and
link time. One function stays outside: `main` must be the *global*
namespace's `main`, because the C runtime looks up `::main` and nothing else.
So the body of `main` moved in as `snek::Run` and global `main` forwards one
call to it.

**`constexpr`.** `TICK_LEN`, `FRAME_LEN`, `GRID_ROWS`, `GRID_COLS`, and
`SNAKE_MAX` are now `constexpr` — values the compiler must be able to fold
away at compile time. `SNAKE_MAX = GRID_ROWS * GRID_COLS` is multiplied by
the compiler, not the CPU; the arrays are sized before the program exists;
and where the old `enum` constants lived as C's compile-time ints, `constexpr`
covers the doubles too. The disassembly of `Grid::Clear`'s loops shows the
folding — the bounds are literals in the instructions, not values fetched
from anywhere:

```
    1c13:	addl   $0x1,-0x4(%rbp)
    1c17:	cmpl   $0x27,-0x4(%rbp)
    1c1b:	jle    1bec <_ZN4snek4Grid5ClearEv+0x1e>
    1c1d:	addl   $0x1,-0x8(%rbp)
    1c21:	cmpl   $0x13,-0x8(%rbp)
```

`0x27` is `GRID_COLS - 1`, `0x13` is `GRID_ROWS - 1`: the compiler did the
arithmetic and the machine just compares.

**Classes, before any vtables.** Lesson 024's `struct Command` and its
scanner `RunCommand` are now one class, `CommandTable`: the rows are nested
`Command` data exactly as they were, and the scanner is a member function.
What does a class compile down to? Fields are the same struct layout; a
member function is a plain function that receives the object's address as an
invisible extra first parameter — the `this` pointer. Watch `Grid::Clear`
open by saving it:

```
0000000000001bce <_ZN4snek4Grid5ClearEv>:
    1bce:	endbr64
    1bd2:	push   %rbp
    1bd3:	mov    %rsp,%rbp
    1bd6:	mov    %rdi,-0x18(%rbp)
```

`%rdi` is the first argument register: `table.Run(key)` is `Run(&table, key)`
in everything but spelling. The constructor is a plain function that fills in
the fields, and the private members carry a trailing underscore — the
convention this course adopts for them. None of this costs anything at
runtime; it costs you one pointer-shaped parameter.

**Overloading.** `Grid` has two `Put` functions: one writes a single `char`,
one writes a whole string, and the string overload loops its characters back
into the character one. Same name, same job, two types — the compiler picks
the function by the argument type at the call site. What does *that* compile
down to? Two separate functions with two separate linker names: **name
mangling** again, this time encoding parameter types instead of namespaces.

```
$ nm snek | grep Put
0000000000001c8a T _ZN4snek4Grid3PutEiiPKc
0000000000001c2c T _ZN4snek4Grid3PutEiic
$ nm snek | grep Put | c++filt
0000000000001c8a T snek::Grid::Put(int, int, char const*)
0000000000001c2c T snek::Grid::Put(int, int, char)
```

`c++filt` demangles; the suffixes `Eiic` and `EiiPKc` are the two parameter
lists (`i i c` = int, int, char; `PKc` = pointer to const char). C could not
do this — one name, one function — and the linker needs the names apart
because both overloads live in the program at once.

**References.** Every `Draw` takes `Grid &grid`. A reference is a pointer the
compiler dereferences for you: the signature reads like a value parameter,
the machine sees a `Grid *`, and every `grid.Put(...)` inside inserts the
dereference. It cannot be null and cannot be reseated, which is what makes it
the right type for "draw into *this* grid" — no copying the two 800-byte
buffers, no address-of noise at the call sites. Exercise 4 shows what a
reference quietly prevents.

**Classes with vtables — the draw path.** The interface is one abstract base:
`Drawable`, with a pure virtual `Draw`. Two concrete classes implement it —
`GridView` draws the border, the food and the snake; `StatusView` draws the
status line and the prompts — and `views` is a small table of `Drawable *`
rows that `Render` walks:

```c++
    for (size_t i = 0; i < sizeof views / sizeof views[0]; ++i)
        views[i]->Draw(grid);
```

The loop does not know what it is drawing. Virtual dispatch compiles down to
two pieces of data: one **vptr** in every object (its first word, pointing at
its class's table) and one **vtable** per class — a table of function
pointers. The call loads the object's first word, loads the slot it points
at, and calls. Compare that with lesson 024, feature by feature: the rows
pairing keys with functions are the vtable; the scanner finding the right row
is the fixed index; `commands[i].run()` is the call. **The vtable is lesson
024's command table, automated** — the compiler writes the table once per
class, plants the pointer in every object, and you never see the machinery
except in the binary:

```
$ nm snek | grep _ZTV | grep snek | c++filt
0000000000004cc0 V vtable for snek::StatusView
0000000000004cd8 V vtable for snek::GridView
```

Real data, one per class — exercise 3 follows a vptr all the way to a
function pointer. One rule of the table: draw order is table order, later
rows painting over earlier ones, which is how the status line keeps its
z-order under the snake.

What is *not* here matters as much: no templates, no exceptions, no STL
containers, no `new`/`delete`. The game state is still plain scalars and
arrays, and the classes wrap code and dispatch rather than taking ownership
of everything. That is the subset Part 1's engine starts from — every feature
above you can now read as assembly. The code step is the whole conversion in
one diff.

## Code step

One change for this lesson: the whole conversion — `snek.c` renamed to
`snek.cpp`, the command table wrapped in a class, the draw path rebuilt as a
drawable interface with a vtable. Its end state is tagged `lesson-025`.

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

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — A third view *(extend-the-code)*

The views table has two rows and `Render` still never mentions what they
draw. Add a third concrete `Drawable` — a `TallyView` that draws the frame
and tick counters (`f=NNN t=NNN`) along the bottom border row — register it
in `views`, and show it working in a test-mode run (`./snek 30`). How many
existing functions did adding it have to touch?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-025/ex1.md)

### Exercise 2 — The hidden word *(predict-the-output)*

Both view classes carry no data at all. Before touching anything, predict
what `sizeof(GridView)`, `sizeof(StatusView)`, and `sizeof` of an empty plain
twin — a `struct PlainView` with a non-virtual `Draw` and no members — print
on your machine. Then add the one instrumenting `fprintf` that prints the
three sizes before the loop starts, run `./snek 5`, and reconcile the
numbers. What is the extra word, and why does every object pay for it?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-025/ex2.md)

### Exercise 3 — The vtable under the glass *(explain-in-prose)*

Explain in prose what `views[i]->Draw(grid)` compiles to — in the same
machine terms lesson 024 used for `commands[i].run()`. Before writing, make
the mechanism visible: print the object's first word as a pointer, print the
first function pointer stored where that word points, run a frame, and
compare both addresses with `nm snek` (through `c++filt`) for
`GridView::Draw`. What do the numbers prove, and which part of lesson 024's
dispatch did the compiler just write for you?

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-025/ex3.md)

### Exercise 4 — The copy that cannot exist *(fix-the-crash)*

A teammate wants `Render` to keep the status view cached in a local —
`Drawable saved = status;` — and the build dies with `error: cannot allocate
an object of abstract type ‘snek::Drawable’`. Make the cache work: after your
fix, the status text must still come from `StatusView::Draw` even though it
is drawn through the cached name. Explain why the compiler refused the copy,
and what the program would have done if it had been allowed.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-025/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 024 — the function-pointer command table](lesson-024-command-table.md) ·
**Next:** [Lesson 026 — the codebase is born](../part-1/lesson-026-birth.md) ·
**Code tag:** [`lesson-025`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-025)
