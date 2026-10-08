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
#include "ai.h"
#include "combat.h"
#include "text.h"
#include "tilemap.h"
#include "tiles.h"

namespace engine {

/* Lesson 087 made the hit real: a projectile reduces its target's
   health by its row's damage, and a zero-health entity is retired.
   Lesson 090 made the enemy fire real. Lesson 091 made the waves real.
   No stand-ins remain: every named condition of this machine is the
   game's own work now. */

/* A transition, named once here and printed the moment it happens, so a
   run shows the machine moving between states and why. */
static void Transition(Game &game, GameState to, const char *why)
{
    std::printf("engine: state %s -> %s (%s)\n", GameStateName(game.state),
                GameStateName(to), why);
    game.state = to;
    /* Lesson 096: the new screen fades in from black — the fade's
       clock starts with the screen. */
    game.fade = 0.0;
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
    game.score = 0.0;
    game.fade = 0.0;
    game.wave = 0;
    game.camera = { 0, 0, 0, 0 };
    std::printf("engine: game: %d state%s, starting on %s\n", 5, "s",
                GameStateName(game.state));
}

void GameInput(Game &game, platform::Window *window, Entity &hero,
               double wall_dt)
{
    /* Lesson 096: the screen's fade runs on wall time — the
       presentation's clock. Game time is zero wherever a screen shows
       (the world stands still behind the panel), so a fade on game
       time would never arrive; this is lesson 078's split again, on
       the other side of it. */
    if (game.fade < GAME_FADE_S) {
        game.fade += wall_dt;
        if (game.fade >= GAME_FADE_S && game.state != GAME_PLAY) {
            /* The fade arrives exactly at the screen's color — the
               eased value's contract (lesson 093), measured on the
               screen's own backdrop. Play's screen is the world and
               fades nothing. */
            int r, g, b;
            GameScreenColor(game, r, g, b);
            std::printf("engine: screen: %s fade arrived at %d,%d,%d (its own color)\n",
                        GameStateName(game.state), r, g, b);
        }
    }

    switch (game.state) {
    case GAME_TITLE:
        /* The title screen accepts one thing: the start key. */
        if (platform::KeyPressed(window, platform::KEY_ENTER)) {
            /* A fresh game restores the hero's health, the game's waves,
               and the hero's rest — the row's facts, not remembered
               state. */
            hero.health = game.hero_health_full;
            game.waves_remaining = GAME_WAVES;
            game.wave = 0; /* lesson 091: the fight starts over */
            game.play_clock = 0.0;
            game.score = 0.0; /* lesson 094: a fresh game's score */
            hero.move_x = 0.0;
            hero.move_y = 0.0;
            Transition(game, GAME_PLAY, "the player started");
        }
        break;

    case GAME_PLAY: {
        /* Play's movement is the hero's own (HeroMove, lesson 085) and
           its fire is the hero's weapon's (HeroFire, lesson 087) — both
           the run's, in play. What is left here is the play state's
           other input and the named conditions. */

        game.play_clock += wall_dt;

        /* The named transitions, read from the game's own state. Defeat
           first: a hero at zero health is dead, whatever else holds. */
        if (hero.health <= 0) {
            Transition(game, GAME_DEATH, "the hero's health reached zero");
            break;
        }

        /* Completion is the game's waves reaching zero — spent for
           real by the wave fight (GameWaves, lesson 091) now, not by a
           key. */
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

/* Lesson 096: one centered line of a screen. */
static void Line(Framebuffer &fb, const Font &font, const char *text, int y)
{
    DrawText(fb, font, text, (FRAME_WIDTH - TextWidth(text)) / 2, y);
}

/* The run's final numbers, the same values the HUD reads — the score,
   the play clock as minutes and seconds, and the wave the game ended
   on. The end screens show them; the game's state is the game's story. */
static void Numbers(const Game &game, char *line, int size)
{
    int secs = (int)game.play_clock;
    std::snprintf(line, size, "SCORE %06d   TIME %d:%02d   WAVE %d/%d",
                  (int)game.score, secs / 60, secs % 60, game.wave,
                  GAME_WAVES);
}

/* Lesson 096: the screens in final form. Each state's screen is its
   own — the world is not drawn behind it — and each names the input it
   acts on, so the screen documents the very input the machine listens
   for. The report below prints the screen's content the first frame it
   draws: a run shows every screen it staged. */
static void Report(const Game &game, const char *title, const char *body,
                   const char *prompt)
{
    static GameState was = GAME_TITLE;
    static bool first = true;
    if (!first && was == game.state)
        return;
    first = false;
    was = game.state;
    std::printf("engine: screen: %s: \"%s\" / \"%s\" / \"%s\"\n",
                GameStateName(game.state), title, body, prompt);
}

void GameDrawPanel(const Game &game, Framebuffer &fb, const Font &font)
{
    char body[64];
    switch (game.state) {
    case GAME_TITLE:
        Line(fb, font, "THE FRAMEBUFFER GAME", 140);
        Line(fb, font, "ARROWS  MOVE      SPACE  FIRE", 196);
        Line(fb, font, "1 / 2   WEAPONS   ESCAPE  PAUSE", 212);
        Line(fb, font, "ENTER: PLAY", 268);
        Report(game, "THE FRAMEBUFFER GAME",
               "ARROWS MOVE ... ESCAPE PAUSE", "ENTER: PLAY");
        break;

    case GAME_PAUSE:
        Numbers(game, body, sizeof body);
        Line(fb, font, "PAUSED", 180);
        Line(fb, font, body, 228);
        Line(fb, font, "ESCAPE: RESUME", 268);
        Report(game, "PAUSED", body, "ESCAPE: RESUME");
        break;

    case GAME_DEATH:
        Numbers(game, body, sizeof body);
        Line(fb, font, "GAME OVER", 180);
        Line(fb, font, body, 228);
        Line(fb, font, "ENTER: TITLE", 268);
        Report(game, "GAME OVER", body, "ENTER: TITLE");
        break;

    default:
        Numbers(game, body, sizeof body);
        Line(fb, font, "VICTORY", 180);
        Line(fb, font, body, 228);
        Line(fb, font, "ENTER: TITLE", 268);
        Report(game, "VICTORY", body, "ENTER: TITLE");
        break;
    }
}

void GameScreenColor(const Game &game, int &r, int &g, int &b)
{
    /* The screen's own backdrop, faded in from black by the ease — the
       toolkit's easing (lesson 093) applied to a screen's fade, the
       value arriving exactly at the screen's color. The end screens
       carry their own tint: the death screen reddens, the victory
       screen greens. */
    int tr = 24, tg = 24, tb = 40;
    if (game.state == GAME_DEATH) {
        tr = 56;
        tg = 16;
        tb = 16;
    } else if (game.state == GAME_VICTORY) {
        tr = 16;
        tg = 48;
        tb = 16;
    }
    double t = game.fade < GAME_FADE_S ? game.fade / GAME_FADE_S : 1.0;
    double k = EaseInOutQuad(t);
    r = (int)(tr * k);
    g = (int)(tg * k);
    b = (int)(tb * k);
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
    /* The camera's additive offset is the juice hook — lesson 086's
       feedback (FeelUpdate) drives it now, and rests it at zero. */
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
        BlitSpriteFrame(fb, *e.sprite, e.frame * ANIM_FRAME_W, ANIM_FRAME_W,
                        (int)e.x - CameraX(game.camera),
                        (int)e.y - CameraY(game.camera));
    }
}

int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
             const EntityTable &shots, Feedback &feel, const EntityDef &spark,
             double dt, Sound &sound)
{
    /* Lesson 084: the walk — every live entity, once per frame, in slot
       order, its movement resolved against the tilemap. The per-entity
       work is expressed once here, not per type: the mover (MoveEntity)
       turns the request into motion one axis at a time, so an entity
       that meets a solid tile stops on that axis and slides along the
       wall on the other — and the facing follows where it is going.

       Lesson 087: the per-entity work branches on the entity's behavior
       — the fact its row carries. A projectile flies: its own
       sub-stepped flight through the same mover, retiring at walls, at
       its range's end, and at the entity it hit.

       Lesson 089: the rest of the branch is the enemy behaviors —
       chase, keep-distance, flee — each a small function writing this
       entity's movement request the way the player's input writes the
       hero's. One branch on the behavior, per-entity work expressed
       once: the boss (lesson 090) is one more value here, not one more
       shape.

       Lesson 092: the flight's events — a hit lands, a death falls —
       fire the feedback hooks in their own frame (the toolkit is handed
       along through `feel`). */
    int visited = 0;
    for (int i = 0; i < ENTITY_CAP; ++i) {
        if (!store.slots[i].live)
            continue;
        visited += 1;
        Entity &e = store.slots[i];
        switch (e.behavior) {
        case BEHAVIOR_FLY:
            CombatFly(map, store, hero, e, feel, spark, dt, sound);
            continue; /* the flight moves itself, through the mover */
        case BEHAVIOR_SETTLE:
            FeelParticle(store, e, dt);
            continue; /* the settle moves itself — eased travel, no
                        collision: sparks fly over the scene */
        case BEHAVIOR_CHASE:
            AiChase(e, hero);
            break;
        case BEHAVIOR_KEEP:
            AiKeep(e, hero);
            break;
        case BEHAVIOR_FLEE:
            AiFlee(e, hero);
            break;
        case BEHAVIOR_BOSS:
            AiBoss(e, hero, dt);
            break;
        default:
            /* `none` stands where it stands — the request is its row's
               (lesson 084's stand-in walk used to write one). */
            break;
        }

        /* Lesson 090: the attack, once per entity — an armed kind fires
           its row's weapon at the hero at its rate. The hero is exempt
           (its trigger is the player's); an unarmed kind fires
           nothing. */
        CombatAttack(store, shots, e, hero, dt, sound);
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

/* Lesson 091: the kinds that fight are the ones with a behavior to
   fight with. The hero, the scenery (behavior `none`), and the shots
   (`fly`) are none of them — so "is a fighter of the waves" is the
   row's behavior, asked once. */
static bool IsFighter(const Entity &e)
{
    return e.behavior == BEHAVIOR_CHASE || e.behavior == BEHAVIOR_KEEP ||
           e.behavior == BEHAVIOR_FLEE || e.behavior == BEHAVIOR_BOSS;
}

void GameWaves(Game &game, EntityStore &store, const EntityTable &foes)
{
    /* A fresh fight: the last game's fighters, shots, and debris leave
       the store — the hero is the game's actor and the `none` kinds are
       the world's scenery, and both stay. The debris goes too (lesson
       093): a spark settles in game time, and a frozen one — from the
       blow that ended the last game — would hang there forever. */
    if (game.wave == 0) {
        int cleared = 0;
        for (int i = 0; i < ENTITY_CAP; ++i) {
            Entity &e = store.slots[i];
            if (!e.live)
                continue;
            if (IsFighter(e) || e.behavior == BEHAVIOR_FLY ||
                e.behavior == BEHAVIOR_SETTLE) {
                EntityRetire(store, e);
                cleared += 1;
            }
        }
        if (cleared)
            std::printf("engine: wave: the last fight leaves the store (%d retired)\n",
                        cleared);
    }

    /* The wave is being fought while any of its fighters lives. */
    int live = 0;
    for (int i = 0; i < ENTITY_CAP; ++i)
        if (store.slots[i].live && IsFighter(store.slots[i]))
            live += 1;
    if (live > 0)
        return;

    /* The wave is clear: the next begins — or the last one ended the
       game's waves, which is the named condition for victory. */
    if (game.wave >= GAME_WAVES) {
        if (game.waves_remaining > 0) {
            std::printf("engine: the waves are complete (t=%.3f)\n",
                        game.play_clock);
            game.waves_remaining = 0;
        }
        return;
    }
    if (game.wave > 0)
        std::printf("engine: wave %d cleared — the next begins\n", game.wave);
    game.wave += 1;

    /* The composition is the table's: every kind whose row's wave has
       come joins the wave (a kind joins at its wave and every wave
       after it), its row's count of them. The copies stand in a line
       beside their row's spot. */
    int spawned = 0;
    for (int i = 0; i < foes.count; ++i) {
        const EntityDef &def = foes.rows[i];
        if (def.wave <= 0 || def.wave > game.wave)
            continue;
        for (int n = 0; n < def.count; ++n) {
            EntityResult made = EntityCreate(store, def);
            if (made.error != ENTITY_OK) {
                std::printf("engine: wave %d: the store refused %s\n",
                            game.wave, def.name);
                continue;
            }
            made.entity->x = def.x + n * ANIM_FRAME_W;
            std::printf("engine: wave %d: spawns %s at %d,%d — speed %d, health %d, %s\n",
                        game.wave, made.entity->name, (int)made.entity->x,
                        (int)made.entity->y, made.entity->speed,
                        made.entity->health,
                        BehaviorName(made.entity->behavior));
            spawned += 1;
        }
    }
    std::printf("engine: wave %d begins — %d enemies\n", game.wave, spawned);
}

} /* namespace engine */
