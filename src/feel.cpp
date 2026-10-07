// feel.cpp — the feedback hooks: fire, decay, rest.
//
// Lesson 086: each hook fires, runs down its own wall-time, and returns
// exactly to rest. Nothing here decides *when* to fire — that is the
// juice toolkit's job (lessons 092-093), reading the game's events.

#include "feel.h"

#include <cstdio>

namespace engine {

void FeelInit(Feedback &feel)
{
    feel.shake = 0.0;
    feel.shake_mag = 0.0;
    feel.hitstop = 0.0;
    feel.hitstop_k = 0.0;
}

void FeelShake(Feedback &feel, double magnitude, double seconds)
{
    feel.shake = seconds;
    feel.shake_mag = magnitude;
}

void FeelHitstop(Feedback &feel, double fraction, double seconds)
{
    feel.hitstop = seconds;
    feel.hitstop_k = fraction;
}

double FeelTimeScale(const Feedback &feel)
{
    /* At rest the factor is full speed; during a hitstop it is the
       fraction the hitstop was fired at. */
    return feel.hitstop > 0.0 ? feel.hitstop_k : GAMETIME_FULL;
}

void FeelUpdate(Feedback &feel, double wall_dt, Camera &camera)
{
    /* The hitstop runs on its own wall-time and returns to full speed
       when its deadline passes — the game need not remember to undo it. */
    if (feel.hitstop > 0.0) {
        feel.hitstop -= wall_dt;
        if (feel.hitstop < 0.0) {
            feel.hitstop = 0.0;
            std::printf("engine: feel: hitstop rested — full speed again\n");
        }
    }

    /* The screenshake drives the camera's additive offset — the juice
       hook lesson 054 defined. While it lasts the offset alternates;
       when it ends the offset rests at exactly zero. */
    if (feel.shake > 0.0) {
        feel.shake -= wall_dt;
        if (feel.shake <= 0.0) {
            feel.shake = 0.0;
            camera.add_x = 0;
            camera.add_y = 0;
            std::printf("engine: feel: shake rested at %d,%d\n", camera.add_x,
                        camera.add_y);
        } else {
            camera.add_x = ((int)(feel.shake * 40.0) & 1) ? (int)feel.shake_mag
                                                          : -(int)feel.shake_mag;
            camera.add_y = 0;
        }
    }
}

} /* namespace engine */
