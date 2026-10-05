// snek.c — a terminal snake game, grown lesson by lesson.
//
// Lesson 021: raw terminal input — termios, poll, and escape sequences.
#define _POSIX_C_SOURCE 200809L /* clock_gettime, nanosleep, termios: POSIX, not ISO C */
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static const double TICK_LEN = 1.0 / 10.0; /* fixed timestep: 10 updates per second */
static const double FRAME_LEN = 1.0 / 30.0; /* frame cap: 30 frames per second */

enum Direction { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };

static const char *dir_names[] = {"up", "down", "left", "right"};

static int running;              /* the loop runs while this is true */
static unsigned long frame;      /* frames since the loop started */
static unsigned long tick;       /* game updates since the loop started */
static double tick_accum;        /* seconds of game time not yet ticked away */
static double frame_dt;          /* measured length of the current frame */
static unsigned long max_frames; /* stop after this many frames (0: until q) */
static int test_mode;            /* a frame budget was given: trace every frame */
static int dir = DIR_RIGHT;      /* where the snake is heading */
static int esc;                  /* escape-sequence parser state */

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
    tcsetattr(STDIN_FILENO, TCSANOW, &raw); /* TCSANOW, not TCSAFLUSH: keep typed-ahead bytes */
    termios_saved = 1;
    atexit(RestoreTerminal);
    signal(SIGINT, OnInterrupt);
}

static void OnByte(unsigned char c)
{
    if (esc == 0) {
        if (c == 0x1b)
            esc = 1;
        else if (c == 'q')
            running = 0;
    } else if (esc == 1) {
        esc = (c == '[') ? 2 : 0; /* a lone ESC eats the next byte */
    } else {
        esc = 0;
        if (c == 'A') dir = DIR_UP;
        else if (c == 'B') dir = DIR_DOWN;
        else if (c == 'C') dir = DIR_RIGHT;
        else if (c == 'D') dir = DIR_LEFT;
    }
}

static void ProcessInput(void)
{
    struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
    if (poll(&pfd, 1, 0) <= 0)
        return;

    unsigned char buf[64];
    ssize_t n = read(STDIN_FILENO, buf, sizeof buf);
    for (ssize_t i = 0; i < n; ++i)
        OnByte(buf[i]);
}

static void Update(double dt)
{
    tick_accum += dt;
    while (tick_accum >= TICK_LEN) {
        tick_accum -= TICK_LEN;
        ++tick;
    }
    ++frame;
    if (max_frames > 0 && frame >= max_frames)
        running = 0;
}

static void Render(void)
{
    if (test_mode)
        fprintf(stderr, "frame=%lu tick=%lu dir=%s\n", frame, tick,
                dir_names[dir]);
}

int main(int argc, char **argv)
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
        fprintf(stderr, "snek — arrows to steer, q to quit\n");

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
