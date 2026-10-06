// platform_internal.h — the platform layer's own state. Not the seam's
// contract.
//
// Lesson 060: platform.h is the engine's whole view of the OS — that header
// is the seam's contract, and a second OS owes exactly what it declares and
// nothing more. Nothing here crosses it: no OS header is included, no OS
// type is named, no device or display handle appears. What lives here is
// the platform layer's own bookkeeping — how long this platform's event
// pump may sleep before the sound output needs feeding — shared between the
// audio implementation and the wait that obeys it. A second OS defines
// these in its own implementation files, beside its platform.h functions.
#ifndef PLATFORM_INTERNAL_H
#define PLATFORM_INTERNAL_H

namespace platform {

/* How long the run may wait before the output needs its next buffer —
   the bound PumpEvents puts on its wait. Negative when there is no
   output to feed: the wait is then unbounded, exactly as before sound. */
double AudioWaitSeconds(void);

} /* namespace platform */

#endif
