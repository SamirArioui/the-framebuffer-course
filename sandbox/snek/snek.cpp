// snek.cpp — a terminal snake game, grown lesson by lesson.
//
// Lesson 025: the C++ subset — classes and vtables.
#define _POSIX_C_SOURCE 200809L /* clock_gettime, nanosleep, termios: POSIX, not ISO C */
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/* The whole game lives in namespace snek: qualified names, zero runtime cost.
   Global main below is the one function the runtime insists on finding itself. */
namespace snek {

constexpr double TICK_LEN = 1.0 / 10.0; /* fixed timestep: 10 updates per second */
constexpr double FRAME_LEN = 1.0 / 30.0; /* frame cap: 30 frames per second */

enum Direction { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
enum GameState { TITLE, PLAY, DEAD };
enum KeyCode {
    KEY_NONE = 0,
    KEY_UP = 256, /* named keys get codes no single byte can collide with */
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
};
constexpr int GRID_ROWS = 20;
constexpr int GRID_COLS = 40;
constexpr int SNAKE_MAX = GRID_ROWS * GRID_COLS; /* folded at compile time */

static const char *dir_names[] = {"up", "down", "left", "right"};
static const char *state_names[] = {"title", "play", "dead"};

static int running;              /* the loop runs while this is true */
static unsigned long frame;      /* frames since the loop started */
static unsigned long tick;       /* game updates since the loop started */
static double tick_accum;        /* seconds of game time not yet ticked away */
static double frame_dt;          /* measured length of the current frame */
static unsigned long max_frames; /* stop after this many frames (0: until q) */
static int test_mode;            /* a frame budget was given: trace every frame */
static int dir = DIR_RIGHT;      /* where the snake is heading */
static int esc;                  /* escape-sequence parser state */
static int state = TITLE;        /* title, play, or dead */

static int snake_row[SNAKE_MAX], snake_col[SNAKE_MAX]; /* segment 0 is the head */
static int snake_len;
static int food_row, food_col;
static int score;
static unsigned rng_state = 12345; /* fixed seed: every run is reproducible */

static struct termios saved_termios;
static int termios_saved;
static volatile sig_atomic_t interrupted;

static double Now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static void SleepSec(double sec)
{
    struct timespec ts;
    ts.tv_sec = (time_t)sec;
    ts.tv_nsec = (long)((sec - (double)ts.tv_sec) * 1e9);
    nanosleep(&ts, NULL);
}

static void RestoreTerminal(void)
{
    if (termios_saved) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_termios);
        termios_saved = 0;
    }
}

static void OnInterrupt(int sig)
{
    (void)sig;
    interrupted = 1;
}

static void EnterRawMode(void)
{
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &saved_termios) != 0)
        return; /* stdin is not a terminal (piped test input) — nothing to set */
    raw = saved_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_iflag &= ~ICRNL; /* Enter is CR: match the byte, not the translation */
    tcsetattr(STDIN_FILENO, TCSANOW, &raw); /* TCSANOW, not TCSAFLUSH: keep typed-ahead bytes */
    termios_saved = 1;
    atexit(RestoreTerminal);
    signal(SIGINT, OnInterrupt);
}

static unsigned RngNext(void)
{
    rng_state = rng_state * 1103515245u + 12345u;
    return (rng_state >> 16) & 0x7fff;
}

static int SnakeAt(int row, int col)
{
    for (int i = 0; i < snake_len; ++i)
        if (snake_row[i] == row && snake_col[i] == col)
            return 1;
    return 0;
}

static void PlaceFood(void)
{
    int start = (int)(RngNext() % (GRID_ROWS * GRID_COLS));
    for (int i = 0; i < GRID_ROWS * GRID_COLS; ++i) {
        int cell = (start + i) % (GRID_ROWS * GRID_COLS);
        int row = cell / GRID_COLS, col = cell % GRID_COLS;
        if (row < 2 || row > GRID_ROWS - 2 || col < 1 || col > GRID_COLS - 2)
            continue; /* the border and the status row */
        if (SnakeAt(row, col))
            continue;
        food_row = row;
        food_col = col;
        return;
    }
}

static void StartGame(void)
{
    snake_len = 3;
    for (int i = 0; i < snake_len; ++i) {
        snake_row[i] = GRID_ROWS / 2;
        snake_col[i] = GRID_COLS / 2 - i;
    }
    score = 0;
    PlaceFood();
    state = PLAY;
}

static void AdvanceSnake(void)
{
    int new_row = snake_row[0], new_col = snake_col[0];
    if (dir == DIR_UP) --new_row;
    else if (dir == DIR_DOWN) ++new_row;
    else if (dir == DIR_LEFT) --new_col;
    else if (dir == DIR_RIGHT) ++new_col;

    if (new_row < 2 || new_row > GRID_ROWS - 2 ||
        new_col < 1 || new_col > GRID_COLS - 2) {
        state = DEAD; /* the wall */
        return;
    }
    for (int i = 0; i < snake_len - 1; ++i) {
        if (snake_row[i] == new_row && snake_col[i] == new_col) {
            state = DEAD; /* itself */
            return;
        }
    }

    int grow = (new_row == food_row && new_col == food_col);
    if (grow) {
        ++score;
        if (snake_len < SNAKE_MAX)
            ++snake_len;
    }
    for (int i = snake_len - 1; i > 0; --i) {
        snake_row[i] = snake_row[i - 1];
        snake_col[i] = snake_col[i - 1];
    }
    snake_row[0] = new_row;
    snake_col[0] = new_col;
    if (grow)
        PlaceFood();
}

static void CmdQuit(void)
{
    running = 0;
}

static void CmdStart(void)
{
    if (state != PLAY)
        StartGame();
}

static void CmdTurn(int new_dir)
{
    if (state != PLAY)
        return;
    /* a longer snake cannot reverse into its own neck */
    if (snake_len > 1 &&
        ((new_dir == DIR_UP && dir == DIR_DOWN) ||
         (new_dir == DIR_DOWN && dir == DIR_UP) ||
         (new_dir == DIR_LEFT && dir == DIR_RIGHT) ||
         (new_dir == DIR_RIGHT && dir == DIR_LEFT)))
        return;
    dir = new_dir;
}

static void CmdUp(void) { CmdTurn(DIR_UP); }
static void CmdDown(void) { CmdTurn(DIR_DOWN); }
static void CmdLeft(void) { CmdTurn(DIR_LEFT); }
static void CmdRight(void) { CmdTurn(DIR_RIGHT); }

/* The command table of lesson 024, as a class: the rows are the data it was,
   the scanner is now a member function over an invisible this pointer. */
class CommandTable {
public:
    struct Command {
        int key;
        void (*run)(void);
    };

    CommandTable(const Command *commands, size_t count)
        : commands_(commands), count_(count) {}

    void Run(int key) const
    {
        for (size_t i = 0; i < count_; ++i) {
            if (commands_[i].key == key) {
                commands_[i].run();
                return;
            }
        }
    }

private:
    const Command *commands_;
    size_t count_;
};

static const CommandTable::Command commands[] = {
    { 'q',        CmdQuit },
    { ' ',        CmdStart },
    { '\n',       CmdStart },
    { KEY_UP,     CmdUp },
    { KEY_DOWN,   CmdDown },
    { KEY_LEFT,   CmdLeft },
    { KEY_RIGHT,  CmdRight },
};

static const CommandTable table(commands, sizeof commands / sizeof commands[0]);

static int ParseByte(unsigned char c)
{
    if (esc == 0) {
        if (c == 0x1b) {
            esc = 1;
            return KEY_NONE;
        }
        return c; /* a plain key: its byte is its code */
    }
    if (esc == 1) {
        if (c == '[') {
            esc = 2;
            return KEY_NONE;
        }
        esc = 0; /* a lone ESC eats the next byte */
        return KEY_NONE;
    }
    esc = 0;
    if (c == 'A') return KEY_UP;
    if (c == 'B') return KEY_DOWN;
    if (c == 'C') return KEY_RIGHT;
    if (c == 'D') return KEY_LEFT;
    return KEY_NONE;
}

static void ProcessInput(void)
{
    struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
    if (poll(&pfd, 1, 0) <= 0)
        return;

    unsigned char buf[64];
    ssize_t n = read(STDIN_FILENO, buf, sizeof buf);
    for (ssize_t i = 0; i < n; ++i) {
        int key = ParseByte(buf[i]);
        if (key != KEY_NONE)
            table.Run(key);
    }
}

/* The off-screen buffer, as a class. The two Put overloads do one job for two
   types; the compiler keeps them apart by mangling their names. */
class Grid {
public:
    void Clear(void);
    void Put(int row, int col, char ch);
    void Put(int row, int col, const char *s);
    void Flush(void);

private:
    char back_[GRID_ROWS][GRID_COLS];  /* drawn into, off-screen */
    char front_[GRID_ROWS][GRID_COLS]; /* what the terminal shows */
    int front_valid_;                  /* 0: the terminal needs a full redraw */
};

void Grid::Clear(void)
{
    for (int row = 0; row < GRID_ROWS; ++row)
        for (int col = 0; col < GRID_COLS; ++col)
            back_[row][col] = ' ';
}

void Grid::Put(int row, int col, char ch)
{
    if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS)
        back_[row][col] = ch;
}

void Grid::Put(int row, int col, const char *s)
{
    for (int i = 0; s[i]; ++i)
        Put(row, col + i, s[i]);
}

void Grid::Flush(void)
{
    if (!front_valid_)
        fprintf(stdout, "\033[2J\033[H"); /* clear the screen before the first draw */

    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
            if (front_valid_ && back_[row][col] == front_[row][col])
                continue;
            fprintf(stdout, "\033[%d;%dH%c", row, col, back_[row][col]);
            front_[row][col] = back_[row][col];
        }
    }
    front_valid_ = 1;
    fflush(stdout);
}

static Grid grid;

/* The draw path, as an interface: a drawable knows only that it must draw
   itself into a Grid. A reference parameter — a pointer the compiler
   dereferences for you. */
class Drawable {
public:
    virtual void Draw(Grid &grid) const = 0;
};

class GridView : public Drawable {
public:
    void Draw(Grid &grid) const;
};

class StatusView : public Drawable {
public:
    void Draw(Grid &grid) const;
};

void GridView::Draw(Grid &grid) const
{
    for (int col = 0; col < GRID_COLS; ++col) {
        grid.Put(1, col, '-');
        grid.Put(GRID_ROWS - 1, col, '-');
    }
    for (int row = 1; row < GRID_ROWS; ++row) {
        grid.Put(row, 0, '|');
        grid.Put(row, GRID_COLS - 1, '|');
    }
    grid.Put(1, 0, '+');
    grid.Put(1, GRID_COLS - 1, '+');
    grid.Put(GRID_ROWS - 1, 0, '+');
    grid.Put(GRID_ROWS - 1, GRID_COLS - 1, '+');

    if (state == TITLE)
        return; /* on the title screen the playfield is just the border */
    grid.Put(food_row, food_col, '*');
    for (int i = snake_len - 1; i >= 0; --i)
        grid.Put(snake_row[i], snake_col[i], i == 0 ? '@' : 'o');
}

void StatusView::Draw(Grid &grid) const
{
    char msg[GRID_COLS + 1];

    if (state == TITLE) {
        grid.Put(0, 0, "SNEK");
        grid.Put(9, 10, "press space to play");
        grid.Put(11, 15, "q to quit");
    } else if (state == DEAD) {
        snprintf(msg, sizeof msg, "game over! score: %d", score);
        grid.Put(0, 0, msg);
        grid.Put(9, 9, "press space to play again");
        grid.Put(11, 15, "q to quit");
    } else {
        snprintf(msg, sizeof msg, "score: %d", score);
        grid.Put(0, 0, msg);
    }
}

static void Update(double dt)
{
    tick_accum += dt;
    while (tick_accum >= TICK_LEN) {
        tick_accum -= TICK_LEN;
        ++tick;
        if (state == PLAY)
            AdvanceSnake();
    }
    ++frame;
    if (max_frames > 0 && frame >= max_frames)
        running = 0;
}

/* Draw order is table order: later rows paint over earlier ones. */
static GridView playfield;
static StatusView status;
static Drawable *const views[] = { &status, &playfield };

static void Render(void)
{
    grid.Clear();
    for (size_t i = 0; i < sizeof views / sizeof views[0]; ++i)
        views[i]->Draw(grid);
    grid.Flush();

    if (test_mode)
        fprintf(stderr, "frame=%lu tick=%lu state=%s score=%d dir=%s at=%d,%d\n",
                frame, tick, state_names[state], score, dir_names[dir],
                snake_row[0], snake_col[0]);
}

int Run(int argc, char **argv)
{
    if (argc > 2) {
        fprintf(stderr, "usage: %s [FRAMES]\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        char *end;
        errno = 0;
        max_frames = strtoul(argv[1], &end, 10);
        if (argv[1][0] == '-' || *end != '\0' || max_frames == 0 ||
            errno == ERANGE) {
            fprintf(stderr, "%s: FRAMES must be a positive integer, got '%s'\n",
                    argv[0], argv[1]);
            return 1;
        }
        test_mode = 1;
    }

    EnterRawMode();
    if (!test_mode)
        fprintf(stderr, "snek - arrows to steer, q to quit\n");

    running = 1;
    double prev = Now();
    while (running && !interrupted) {
        double frame_start = Now();
        frame_dt = frame_start - prev;
        prev = frame_start;

        ProcessInput();
        Update(frame_dt);
        Render();

        double rem = FRAME_LEN - (Now() - frame_start);
        if (rem > 0)
            SleepSec(rem);
    }

    RestoreTerminal();
    fprintf(stderr, "done after %lu frames, %lu ticks\n", frame, tick);
    return 0;
}

} /* namespace snek */

int main(int argc, char **argv)
{
    return snek::Run(argc, argv);
}
