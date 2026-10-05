// snek.c — a terminal snake game, grown lesson by lesson.
//
// Lesson 019: the game loop — ProcessInput, Update, Render, frame by frame.
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static int running;              /* the loop runs while this is true */
static unsigned long frame;      /* frames since the loop started */
static unsigned long max_frames; /* stop after this many frames */

static void ProcessInput(void)
{
    // No keyboard yet — lesson 021 teaches the terminal.
}

static void Update(void)
{
    ++frame;
    if (frame >= max_frames)
        running = 0;
}

static void Render(void)
{
    fprintf(stderr, "frame=%lu\n", frame);
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
    while (running) {
        ProcessInput();
        Update();
        Render();
    }

    fprintf(stderr, "done after %lu frames\n", frame);
    return 0;
}
