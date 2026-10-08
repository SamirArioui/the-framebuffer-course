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

#include "camera.h"
#include "entity.h"
#include "font.h"
#include "framebuffer.h"
#include "gametime.h"
#include "platform.h"
#include "tiles.h"

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
   game is complete when its waves are done. Lesson 091: the waves are
   fought now — each one spawns the kinds its table rows call for, and
   the next begins when the last enemy of the current one is retired. */
constexpr int GAME_WAVES = 3;

/* The game's own state: which state it is in, and the facts the named
   transitions read. Nothing here is a service's — it is the game's. */
struct Game {
    GameState state;
    int waves_remaining;  /* the named condition for victory */
    int hero_health_full; /* the health a fresh game starts the hero at */
    double play_clock;    /* wall seconds spent in play this game */
    int wave;             /* lesson 091: the wave being fought (0 = the
                            fight has not started) */
    Camera camera;        /* lesson 083: the game's world-view — one
                            camera over the single scrolling map. Its
                            base follows the hero; its additive offset
                            rests at zero (the juice hook, lesson 092). */
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
   is the world the game draws below; the loop draws it and calls this
   for the rest. */
void GameDrawPanel(const Game &game, Framebuffer &fb, const Font &font);

/* Lesson 083: the game's world-view. The camera's base follows the hero
   — the world scrolls under the movement — clamped to the map's bounds,
   and its additive offset rests at exactly zero. The game owns the
   camera now; the loop no longer keeps one. */
void GameFollow(Game &game, const Entity &hero, const TileMap &map);

/* The game's world, drawn through the game's camera: the single
   scrolling map, and every live entity at its position. Split so the
   frame record can time the map and the sprites as the two named
   sub-phases it already has. */
void GameDrawMap(const Game &game, Framebuffer &fb, const TileMap &map,
                 const TileSheet &sheet);
void GameDrawSprites(const Game &game, Framebuffer &fb,
                     const EntityStore &store);

/* Lesson 084: the walk — the game resolves every live entity's movement
   against the tilemap. Each entity's movement request becomes motion
   through the mover (MoveEntity), one axis at a time, so it stops at a
   solid tile and slides along a wall. Lesson 087: a projectile's
   behavior flies it (CombatFly — its own sub-stepped flight, retiring at
   walls, at its range, at what it hits) instead of the request. Lesson
   089-090: the enemy behaviors and the boss's pattern write the request
   the way the player's input writes the hero's, and an armed entity
   attacks at its row's rate. The hero is handed along for the combat's
   rules to know the game's actor by. Returns the visit count. */
int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
             const EntityTable &shots, double dt);

/* Lesson 091: the waves, once per frame of play. A fresh fight clears
   the last one from the store; a wave spawns its composition from the
   table's definitions (every kind whose row's wave has come, its row's
   count of them); and the next wave begins when the last enemy of the
   current one is retired. When the last wave is clear the waves are
   complete — the named condition for victory, and no stand-in. */
void GameWaves(Game &game, EntityStore &store, const EntityTable &foes);

} /* namespace engine */

#endif
