// camera.cpp — the sum.
//
// Lesson 054: base plus additive, in one place, so no draw ever sums the
// two its own way.

#include "camera.h"

namespace engine {

int CameraX(const Camera &camera)
{
    return camera.base_x + camera.add_x;
}

int CameraY(const Camera &camera)
{
    return camera.base_y + camera.add_y;
}

} /* namespace engine */
