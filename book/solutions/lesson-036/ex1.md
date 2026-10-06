# Solution: exercise 1 — The copy, isolated

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 036 — frame time as measured data](../../lessons/part-1/lesson-036-frame-time.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

`Present`'s measured time is two very different things glued together:
`XPutImage` — pushing the pixels to the server — and `XSync` — waiting
until the server is done with them (the contract from lesson 031: when
`Present` returns, the pixels are on screen). The instrument times each
half separately.

```
platform: copy 0.446 ms, wait 0.054 ms
platform: copy 0.283 ms, wait 0.043 ms
platform: copy 0.403 ms, wait 0.064 ms
platform: copy 0.574 ms, wait 0.072 ms
platform: copy 0.365 ms, wait 0.084 ms
```

The copy is the cost — roughly 0.3 to 0.9 ms for 640×480 on the machine
this was authored on — and the wait is small but never zero: it is the
round trip plus whatever the server had left to do. Both are real work;
neither is bookkeeping.

Which half would shared memory (MIT-SHM) remove? The *copy*. With a shared
segment the server reads the framebuffer in place, so `XPutImage`'s upload
becomes a pointer and the only cost left is the round trip — which is why
the design named MIT-SHM as a later optimization instead of a Part 1
lesson: this is the measurement that later lesson will be argued against.

One caution these numbers deserve: they are from Xvfb — a virtual display
with no compositor, no other clients, and no real screen. On a desktop the
same two numbers move around (window size, driver, compositing). What does
not move is the *shape*: the presentation copy is a real share of the
frame, it happens every frame, and now it is measured instead of assumed.
