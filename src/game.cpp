// game.cpp — the game-state machine: the states, their input, their
// transitions, and the scale the state sets.
//
// Lesson 082: the whole of the machine lives here so the loop stays the
// loop. `Run` reads news, calls GameInput (the state's input and the
// named transitions), sets the scale from GameScale, advances the world
// by game time, and draws the current state's screen. Nothing in the
// machine reaches behind the platform seam, and nothing allocates.

#include "game.h"

#include <cstdio>

#include "blit.h"
#include "text.h"
#include "tilemap.h"
#include "tiles.h"

namespace engine {

/* The demonstration stand-in, named so it cannot be mistaken for the
   game. Lesson 082 has the state machine but not yet the gameplay that
   drives two of its named conditions: combat reduces the hero's health
   (lesson 087) and the waves spend the game's completion (lesson 091).
   Until those land, keys stand in for them: SPACE is a hit on the hero,
   and ENTER in play says the game is complete — the same kind of keyed
   demonstration, and like lesson 078's script before the juice toolkit
   drove it. Both are removed when the real triggers arrive; the
   transitions they fire are the game's own (defeat on zero health,
   completion on no waves). Keyed, they never fire on their own during a
   gameplay test. */

/* A transition, named once here and printed the moment it happens, so a
   run shows the machine moving between states and why. */
static void Transition(Game &game, GameState to, const char *why)
{
    std::printf("engine: state %s -> %s (%s)\n", GameStateName(game.state),
                GameStateName(to), why);
    game.state = to;
}

const char *GameStateName(GameState state)
{
    switch (state) {
    case GAME_TITLE:
        return "title";
    case GAME_PLAY:
        return "play";
    case GAME_PAUSE:
        return "pause";
    case GAME_DEATH:
        return "death";
    default:
        return "victory";
    }
}

void GameInit(Game &game, int hero_health_full)
{
    game.state = GAME_TITLE;
    game.waves_remaining = GAME_WAVES;
    game.hero_health_full = hero_health_full;
    game.play_clock = 0.0;
    game.camera = { 0, 0, 0, 0 };
    std::printf("engine: game: %d state%s, starting on %s\n", 5, "s",
                GameStateName(game.state));
}

void GameInput(Game &game, platform::Window *window, Entity &hero,
               double wall_dt)
{
    switch (game.state) {
    case GAME_TITLE:
        /* The title screen accepts one thing: the start key. */
        if (platform::KeyPressed(window, platform::KEY_ENTER)) {
            /* A fresh game restores the hero's health, the game's waves,
               and the hero's rest — the row's facts, not remembered
               state. */
            hero.health = game.hero_health_full;
            game.waves_remaining = GAME_WAVES;
            game.play_clock = 0.0;
            hero.move_x = 0.0;
            hero.move_y = 0.0;
            Transition(game, GAME_PLAY, "the player started");
        }
        break;

    case GAME_PLAY: {
        /* Play's movement is the hero's own (HeroMove, lesson 085) — the
           held direction read and eased into motion there. What is left
           here is the play state's other input and the named conditions. */

        /* The stand-in for combat: SPACE is a hit on the hero (lesson 087
           makes real hits land). The named condition below reads the
           health this lowers. */
        if (platform::KeyPressed(window, platform::KEY_SPACE) &&
            hero.health > 0) {
            hero.health -= 1;
            std::printf("engine: hero takes a hit — health %d (t=%.3f)\n",
                        hero.health, game.play_clock);
        }

        game.play_clock += wall_dt;

        /* The named transitions, read from the game's own state. Defeat
           first: a hero at zero health is dead, whatever else holds. */
        if (hero.health <= 0) {
            Transition(game, GAME_DEATH, "the hero's health reached zero");
            break;
        }

        /* The stand-in for the waves: ENTER in play says the game is
           complete (lesson 091 spends the waves for real). Completion is
           the game's waves reaching zero. A keyed stand-in, like SPACE
           is a hit — it never fires on its own during a gameplay test. */
        if (platform::KeyPressed(window, platform::KEY_ENTER)) {
            game.waves_remaining = 0;
            std::printf("engine: the waves are complete (t=%.3f)\n",
                        game.play_clock);
        }
        if (game.waves_remaining == 0) {
            Transition(game, GAME_VICTORY, "the game's waves are complete");
            break;
        }

        /* Pause suspends play. The latch keeps one press from acting
           twice (lesson 033). */
        if (platform::KeyPressed(window, platform::KEY_ESCAPE))
            Transition(game, GAME_PAUSE, "the player paused");
        break;
    }

    case GAME_PAUSE:
        /* The pause screen accepts one thing: resume. Play continues from
           where it stood — the world was frozen, not reset. */
        if (platform::KeyPressed(window, platform::KEY_ESCAPE))
            Transition(game, GAME_PLAY, "the player resumed");
        break;

    case GAME_DEATH:
    case GAME_VICTORY:
        /* The end screens accept one thing: back to the title. */
        if (platform::KeyPressed(window, platform::KEY_ENTER))
            Transition(game, GAME_TITLE, "the player returned to the title");
        break;
    }
}

double GameScale(const Game &game)
{
    /* The one number the state sets: play advances the world, every other
       state holds it still. This is D7, and it is why the frame record's
       step is 0 outside play without the world knowing anything special. */
    return game.state == GAME_PLAY ? GAMETIME_FULL : 0.0;
}

/* One panel screen: a title line and a hint, centred. The screen is the
   state's — the world is not drawn behind it. The backdrop is cleared by
   the frame's render phase before this runs, so the panel's own time is
   the text it draws and nothing else. */
static void Panel(Framebuffer &fb, const Font &font, const char *title,
                  const char *hint)
{
    int title_x = (FRAME_WIDTH - TextWidth(title)) / 2;
    int hint_x = (FRAME_WIDTH - TextWidth(hint)) / 2;
    DrawText(fb, font, title, title_x, FRAME_HEIGHT / 2 - FONT_CELL);
    DrawText(fb, font, hint, hint_x, FRAME_HEIGHT / 2 + FONT_CELL);
}

void GameDrawPanel(const Game &game, Framebuffer &fb, const Font &font)
{
    switch (game.state) {
    case GAME_TITLE:
        Panel(fb, font, "THE FRAMEBUFFER GAME", "ENTER: PLAY");
        break;
    case GAME_PAUSE:
        Panel(fb, font, "PAUSED", "ESCAPE: RESUME");
        break;
    case GAME_DEATH:
        Panel(fb, font, "GAME OVER", "ENTER: TITLE");
        break;
    default:
        Panel(fb, font, "VICTORY", "ENTER: TITLE");
        break;
    }
}

void GameFollow(Game &game, const Entity &hero, const TileMap &map)
{
    /* Lesson 083: the camera's base follows the hero — the world scrolls
       under the movement — clamped to the map's bounds so the view never
       shows past the world's edge. The base is the view's origin over the
       map; the additive offset rests at exactly zero, the hook the juice
       toolkit will drive (lesson 092). */
    int base_x = (int)hero.x + hero.sprite->width / 2 - FRAME_WIDTH / 2;
    int base_y = (int)hero.y + hero.sprite->height / 2 - FRAME_HEIGHT / 2;
    if (base_x < 0)
        base_x = 0;
    if (base_y < 0)
        base_y = 0;
    if (base_x > map.width * TILE_SIZE - FRAME_WIDTH)
        base_x = map.width * TILE_SIZE - FRAME_WIDTH;
    if (base_y > map.height * TILE_SIZE - FRAME_HEIGHT)
        base_y = map.height * TILE_SIZE - FRAME_HEIGHT;
    if (base_x != game.camera.base_x || base_y != game.camera.base_y) {
        game.camera.base_x = base_x;
        game.camera.base_y = base_y;
        std::printf("engine: camera base %d,%d\n", base_x, base_y);
    }
    game.camera.add_x = 0;
    game.camera.add_y = 0;
}

void GameDrawMap(const Game &game, Framebuffer &fb, const TileMap &map,
                 const TileSheet &sheet)
{
    /* The single scrolling map, drawn through the game's camera — the
       world is the game's, and this is the game drawing it. */
    DrawTileMap(fb, map, sheet, -CameraX(game.camera), -CameraY(game.camera));
}

void GameDrawSprites(const Game &game, Framebuffer &fb,
                     const EntityStore &store)
{
    /* Every live entity, its art at its position, through the camera's
       summed offset — the draw walk, once, for the game's whole world. */
    for (int i = 0; i < ENTITY_CAP; ++i) {
        if (!store.slots[i].live)
            continue;
        const Entity &e = store.slots[i];
        BlitSprite(fb, *e.sprite, (int)e.x - CameraX(game.camera),
                   (int)e.y - CameraY(game.camera));
    }
}

int GameWalk(EntityStore &store, const TileMap &map, double dt)
{
    /* Lesson 084: the walk — every live entity, once per frame, in slot
       order, its movement resolved against the tilemap. The per-entity
       work is expressed once here, not per type: the mover (MoveEntity)
       turns the request into motion one axis at a time, so an entity
       that meets a solid tile stops on that axis and slides along the
       wall on the other — and the facing follows where it is going.
       The hero and every other entity resolve the same way. */
    int visited = 0;
    for (int i = 0; i < ENTITY_CAP; ++i) {
        if (!store.slots[i].live)
            continue;
        visited += 1;
        Entity &e = store.slots[i];
        MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
        if (e.move_x > 0.0)
            e.facing = 0;
        else if (e.move_y > 0.0)
            e.facing = 1;
        else if (e.move_x < 0.0)
            e.facing = 2;
        else if (e.move_y < 0.0)
            e.facing = 3;
    }
    return visited;
}

} /* namespace engine */
