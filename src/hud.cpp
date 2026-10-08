// hud.cpp — the readouts: the game's state, in text, over the world.
//
// Lesson 094: every number drawn here is read from the game's own state
// at the moment of the draw — the same values the states act on — and
// drawn at screen coordinates the camera never touches. The run reports
// what it read and where it drew it, whenever either changes.

#include "hud.h"

#include <cstdio>

#include "text.h"

namespace engine {

/* The HUD's anchor: screen pixels from the frame's top-left. The world
   is drawn at minus the camera; the readouts at this and nothing else. */
constexpr int HUD_X = 8;
constexpr int HUD_Y = 8;

void HudDraw(const Game &game, const Entity &hero, Framebuffer &fb,
             const Font &font)
{
    /* The readouts, each the game's own value as text: the score the
       game counts, the hero's health against its full, the wave being
       fought of the game's waves, and the play clock as minutes and
       seconds. */
    char score_line[32], health_line[32], wave_line[32], time_line[32];
    std::snprintf(score_line, sizeof score_line, "SCORE %06d",
                  (int)game.score);
    std::snprintf(health_line, sizeof health_line, "HEALTH %d/%d",
                  hero.health, game.hero_health_full);
    std::snprintf(wave_line, sizeof wave_line, "WAVE %d/%d", game.wave,
                  GAME_WAVES);
    int secs = (int)game.play_clock;
    std::snprintf(time_line, sizeof time_line, "TIME %d:%02d", secs / 60,
                  secs % 60);

    /* The left column at the anchor; the right column against the
       frame's edge. Neither knows where the world is — the HUD is
       screen, the world is world (lesson 054's rule). */
    DrawText(fb, font, score_line, HUD_X, HUD_Y);
    DrawText(fb, font, health_line, HUD_X, HUD_Y + FONT_CELL + 4);
    DrawText(fb, font, wave_line, FRAME_WIDTH - HUD_X - TextWidth(wave_line),
             HUD_Y);
    DrawText(fb, font, time_line, FRAME_WIDTH - HUD_X - TextWidth(time_line),
             HUD_Y + FONT_CELL + 4);

    /* The run's report — what the HUD read and where it drew it,
       whenever anything changed: the values (so a change of the game's
       state is in the same frame's account as the readout showing it)
       or the camera (so the anchor is visible staying put while the
       world scrolls under it). */
    static int was_score = -1, was_health = -1, was_wave = -1, was_secs = -1;
    static int was_cam_x = -1, was_cam_y = -1;
    int score = (int)game.score;
    if (score != was_score || hero.health != was_health ||
        game.wave != was_wave || secs != was_secs ||
        game.camera.base_x != was_cam_x || game.camera.base_y != was_cam_y) {
        was_score = score;
        was_health = hero.health;
        was_wave = game.wave;
        was_secs = secs;
        was_cam_x = game.camera.base_x;
        was_cam_y = game.camera.base_y;
        std::printf("engine: hud: score %06d, health %d/%d, time %d:%02d, wave %d/%d — at %d,%d over camera %d,%d\n",
                    score, hero.health, game.hero_health_full, secs / 60,
                    secs % 60, game.wave, GAME_WAVES, HUD_X, HUD_Y,
                    game.camera.base_x, game.camera.base_y);
    }
}

} /* namespace engine */
