// hud.h — the HUD: the play screen's readouts, drawn over the world.
//
// Lesson 094: score, health, and the game's timers — read from the
// game's own state (the same values the states act on), drawn over the
// scene, and never with the camera (lesson 054's HUD rule): the world
// scrolls, the readouts stay. The play screen owns this; the other
// states draw their own screens (game.cpp's panels).
#ifndef HUD_H
#define HUD_H

#include "entity.h"
#include "font.h"
#include "framebuffer.h"
#include "game.h"

namespace engine {

/* The play screen's readouts, at fixed screen positions: the score, the
   hero's health, the wave, and the play clock — each the game's own
   value at the moment of the draw, so the readout and the state change
   in the same frame. */
void HudDraw(const Game &game, const Entity &hero, Framebuffer &fb,
             const Font &font);

} /* namespace engine */

#endif
