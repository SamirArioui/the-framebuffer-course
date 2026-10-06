// camera.h — the camera: one struct, summed at draw time.
//
// Lesson 054: the camera is two offsets and one rule. The base scrolls
// the world (the view's origin over the map); the additive offset is the
// hook the juice toolkit will drive for screenshake (O2) — zero at rest.
// Every scene draw uses the sum, once, at its origin. The HUD is not
// scene: text on screen belongs to the player and does not move with the
// world.
#ifndef CAMERA_H
#define CAMERA_H

namespace engine {

struct Camera {
    int base_x, base_y; /* where the view sits over the world */
    int add_x, add_y;   /* the juice hook: added on top, zero at rest */
};

/* The summed offset every scene draw applies to its origin. */
int CameraX(const Camera &camera);
int CameraY(const Camera &camera);

} /* namespace engine */

#endif
