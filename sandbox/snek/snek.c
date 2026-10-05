// snek.c — a terminal snake game, grown lesson by lesson.
//
// Lesson 020: timing — clock_gettime, a fixed timestep, and a frame cap.
#define _POSIX_C_SOURCE 200809L /* clock_gettime and nanosleep are POSIX, not ISO C */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const double TICK_LEN = 1.0 / 10.0; /* fixed timestep: 10 updates per second */
static const double FRAME_LEN = 1.0 / 30.0; /* frame cap: 30 frames per second */

static int running;              /* the loop runs while this is true */
static unsigned long frame;      /* frames since the loop started */
static unsigned long tick;       /* game updates since the loop started */
static double tick_accum;        /* seconds of game time not yet ticked away */
static double frame_dt;          /* measured length of the current frame */
static unsigned long max_frames; /* stop after this many frames */

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

static void ProcessInput(void)
{
    // No keyboard yet — lesson 021 teaches the terminal.
}

static void Update(double dt)
{
    tick_accum += dt;
    while (tick_accum >= TICK_LEN) {
        tick_accum -= TICK_LEN;
        ++tick;
    }
    ++frame;
    if (frame >= max_frames)
        running = 0;
}

static void Render(void)
{
    fprintf(stderr, "frame=%lu tick=%lu dt=%.4f\n", frame, tick, frame_dt);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s FRAMES\n", argv[0]);
        return 1;
    }

    char *end;
    errno = 0;
    max_frames = strtoul(argv[1], &end, 10);
    if (*end != '\0' || max_frames == 0) {
        fprintf(stderr, "%s: FRAMES must be a positive integer, got '%s'\n",
                argv[0], argv[1]);
        return 1;
    }

    running = 1;
    double prev = Now();
    while (running) {
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

    fprintf(stderr, "done after %lu frames, %lu ticks\n", frame, tick);
    return 0;
}
