# Solution: exercise 2 — the measure pass on your machine

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 098 — pass 1: measure](../../lessons/part-5/lesson-098-measure.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is the card — `tools/profile-card.sh`, which turns a `gmon.out`
into one pasteable block: the flat profile's top lines and the profiled
CPU summed from the self-seconds column, so two machines' answers to
"what is hot" compare line for line. This machine's card, from the
lesson's own measurement run:

```
$ tools/profile-card.sh build-pg/game gmon.out
 59.77      2.11     2.11  3501190     0.00     0.00  engine::BlitSprite(engine::Framebuffer&, engine::Sprite const&, int, int)
 37.68      3.44     1.33     2584     0.00     0.00  engine::ClearBuffer(engine::Framebuffer&, unsigned char, unsigned char, unsigned char)
  1.13      3.48     0.04     2204     0.00     0.00  engine::DrawTileMap(engine::Framebuffer&, engine::TileMap const&, engine::TileSheet const&, int, int)
  0.57      3.50     0.02    14745     0.00     0.00  engine::BlitSpriteFrame(engine::Framebuffer&, engine::Sprite const&, int, int, int, int)
  0.28      3.51     0.01 29905680     0.00     0.00  engine::(anonymous namespace)::ChannelFrame(engine::Channel&)
  0.28      3.52     0.01     2584     0.00     0.00  platform::Present(platform::Window*, unsigned char const*, int, int)
  0.28      3.53     0.01     2543     0.00     0.00  engine::MixBuffer(engine::Mixer&, short*, int)
  0.00      3.53     0.00   160083     0.00     0.00  engine::(anonymous namespace)::ReadFrame(unsigned char const*)
profiled CPU: 3.53 s in 353 samples of 0.01 s
```

— and the frame account from the *same run* (the card does not replace
it; the two instruments answer different questions, exercise 1's
split being the difference):

```
engine: frame budget — 2583 frames, avg 1.834 ms, worst 3.488 ms (frame 2322)
engine:     clear      0.443      24%
engine:     tilemap    0.835      46%
engine:   present      0.433      24%
```

What the port is *for* is the comparison. On this machine the order is
the map's draw first (`BlitSprite` + `DrawTileMap`, ~61% of the CPU)
and the clear second (`ClearBuffer`, 37.7%), with the present almost
absent from the CPU (0.28%) though it holds a quarter of the frame's
wall. On yours, expect the *same two names* — the game draws the same
map and clears the same frame everywhere — but watch three things
before concluding anything from a different order:

- **The present's split.** A local display's copy costs the *process*
  real CPU; this machine's X server round trip costs it almost none and
  charges the difference as wait. `platform::Present` rising on your
  card is the seam being closer, not the engine being slower.
- **The sound device's feed.** This machine mixes in silence (no
  `/dev/snd` to speak of). A real output makes `MixBuffer` and
  `ChannelFrame` show the *same* CPU as here — the mix runs either way
  (lesson 095's rule) — but `SubmitSamples` and the device's pacing may
  shift where the frames land.
- **Your compiler's level.** The card above is `-O0 -pg`. At `-O3` the
  map's copy loop changes shape entirely (lesson 049 priced that gap:
  several times), so name the build beside the numbers — a card without
  its flags is a number without a machine.

Report the card, the frame table, the build's flags, and the machine's
name — and if your top-2 differ from the book's, the interesting
question is never "who is right" but *which measurement explains the
difference*. That question is the whole measure pass, learned on your
own hardware.
