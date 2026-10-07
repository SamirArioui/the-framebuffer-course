// game.h — the game-state machine: the game standing on finished services.
//
// Lesson 082: the game is not one loop with flags — it runs in exactly
// one of five states, and each state owns its screen and its input. The
// states are named (D7): title, play, pause, death, victory. Transitions
// happen on named conditions — play begins from the title, pause
// suspends play and resumes it, death follows the hero's defeat, victory
// follows the game's completion — and never by accident.
//
// The state also owns the game-time scale (D7): play runs at full speed,
// every other state at zero. So the simulation stands still outside play
// while the presentation keeps drawing that state's screen — the frame
// record stays honest (lesson 079), and pause is the same hook lesson
// 078 named, extended to the other screens.
//
// This is the game layer's first file pair, beside the services (D2):
// the services (table, store, mover, game-time, camera) never grow game
// behavior, and the game never grows inside them. The play state's
// gameplay stands on those services and invents nothing.
#ifndef GAME_H
#define GAME_H

#include "entity.h"
#include "font.h"
#include "framebuffer.h"
#include "gametime.h"
#include "platform.h"

namespace engine {

/* The five states. The game runs in exactly one at a time, and only that
   state's input acts. */
enum GameState {
    GAME_TITLE = 0,
    GAME_PLAY,
    GAME_PAUSE,
    GAME_DEATH,
    GAME_VICTORY,
};

/* The game's wave count — the named condition for victory reads it: the
   game is complete when its waves are done. Lesson 091 builds the waves
   that spend it; until then it is the game's plan, named here. */
constexpr int GAME_WAVES = 3;

/* The game's own state: which state it is in, and the facts the named
   transitions read. Nothing here is a service's — it is the game's. */
struct Game {
    GameState state;
    int waves_remaining;  /* the named condition for victory */
    int hero_health_full; /* the health a fresh game starts the hero at */
    double play_clock;    /* wall seconds spent in play this game */
};

/* The game begins on the title screen. The hero's starting health is the
   row's fact, handed over so a fresh game can restore it. */
void GameInit(Game &game, int hero_health_full);

/* A state's name, for the run's reports. */
const char *GameStateName(GameState state);

/* The current state's input and the named transitions, once per frame.
   The window is polled for held movement (play) and edge-triggered menu
   keys (the latch of lesson 033 keeps one press from acting twice). The
   hero's movement request is written here — play's arrows — and left at
   rest in every other state. The named conditions (defeat, completion)
   are read from the game's own state each frame. */
void GameInput(Game &game, platform::Window *window, Entity &hero,
               double wall_dt);

/* The game-time scale the current state sets: play at full speed, every
   other state at zero. This one number is what makes the simulation
   stand still outside play. */
double GameScale(const Game &game);

/* The current state's screen, for the four states whose screen is a
   panel over a still world — title, pause, death, victory. Play's screen
   is the world the frame draws; the loop draws it and calls this for the
   rest. */
void GameDrawPanel(const Game &game, Framebuffer &fb, const Font &font);

} /* namespace engine */

#endif
