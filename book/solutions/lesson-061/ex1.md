# Solution: exercise 1 — Write the file back

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 061 — the WAV container](../../lessons/part-3/lesson-061-wav.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The writer is the reader's inverse, and the patch builds it the same way
the loader is built: three small `Put` helpers in the anonymous namespace
beside the `Read` helpers, then one function that assembles a file. Where
`ReadU32` takes four little-endian bytes apart, `PutU32` lays them back
down; where `ReadFrame` decodes a count with its sign in bit 15, `PutFrame`
takes `(unsigned short)frame` — the two's-complement count, negative
frames included — and writes it low byte first. If those two do not agree
on what a frame *is*, nothing else in the format can save them.

`WriteSample` writes the lesson's walk backwards: the buffer is
`44 + frame_count * 2` bytes in the arena, the RIFF id, the claim
`size - 8`, the WAVE form, the `fmt ` chunk's sixteen bytes of facts (the
tag, the channel, the rate, the byte rate that is `AUDIO_RATE * 2`, the
block align that is 2, the bits), then the `data` id and the frames' own
size, then the frames. Every field a reader will check is written to pass
that check — the writer is not a second opinion about the format, it is
the same opinion in the other direction.

One habit worth noticing: the assembled bytes are pure scratch, so the
mark and the rollback bracket them — kept or refused, the arena does not
keep forty-four kilobytes of file image. `platform::WriteFile` takes the
bytes before the rollback, exactly as `ReadFile`'s bytes go back with
`ReleaseFile`.

The round trip in `Run` writes the loaded sample to a file of its own,
loads that file back through `LoadSample`, and compares every frame. From
a real run of this lesson's end state plus the patch:

```
engine: sample: 22050 frames at 44100 Hz, 1 channel, first frames: 0 513 1024 1531 2032 2525 3009 3480
engine: round trip: 22050 frames written and read back, every frame agrees
```

And the file itself, compared against the course's asset outside the run:
`assets/tone-roundtrip.wav` is 44144 bytes and **byte-identical** to
`assets/tone.wav` — header and frames, all of it. The writer did not just
produce a file the loader tolerates; it produced *this* file.

That is what the round trip proves that a hexdump cannot. A hexdump shows
one file's bytes and a reader's patience; the round trip puts two
implementations of the format across a real file and checks they agree on
every field and every frame — the container's claims and the frames' bytes
alike. A hexdump would not have caught a sign bug in `PutFrame` (the file
would still "look" like a WAV); the comparison of −1 against −1 catches it
at once. When you later write a file the loader refuses — forget the pad
rule, lie in the byte rate — the two halves argue in public, and the
typed failure names which claim lost.

Nothing here touches the loader, the seam, or the run's stream: the round
trip is a check beside the load, and the file it writes is yours to keep
or delete.
