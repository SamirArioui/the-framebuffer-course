# Change notes — part-3-sound

Authoring-time record for this change: what was verified before the
lessons were written, and what the audit found. These notes are authoring
material (like `plan/`); they are not lesson prose and never reach the
site.

## 1.1 Authoring/verification toolchain

Verified on the authoring machine before any lesson quotes a run. The
commands below were run exactly as written.

### Tool versions used

| Tool | Version |
| ---- | ------- |
| Linux | 6.6.87.2-microsoft-standard-WSL2 x86_64 (Ubuntu 24.04.4 LTS) |
| gcc | 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) |
| g++ | 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) |
| ALSA library / headers | 1.2.11 (`libasound2-dev` 1.2.11-1ubuntu0.3) |
| mdBook | v0.5.4 (pinned) |
| openspec | 1.14.0 |
| git | 2.43.0 |

### The ALSA development headers

`libasound2-dev` is present (`/usr/include/alsa/asoundlib.h` exists) and
is now named in `README.md`'s prerequisites — the `libx11-dev` analog for
the sound line. This machine has **no sound hardware**: `/dev/snd` holds
only `timer`, and there is no card 0.

### The submission path against ALSA's `null` device

A scratch probe (not engine code — it names the OS on purpose, to verify
the platform layer's future implementation) opens the software device
that accepts and discards samples, in exactly the format the engine will
fix:

```
open null                         -> OK
set_access RW_INTERLEAVED         -> OK
set_format  S16_LE                -> OK
set_channels 2                    -> OK
set_rate_near 44100               -> OK (got 44100)
hw_params commit                  -> OK
writei 4410 frames (buffer 0)     -> 4410 OK
writei 4410 frames (buffer 1)     -> 4410 OK
writei 4410 frames (buffer 2)     -> 4410 OK
drain + close                     -> OK
```

So the open/submit/close path runs on this machine against `null`, at
44,100 Hz, 16-bit signed little-endian, stereo, interleaved — the output
format the engine fixes (D2). `snd_pcm_writei` accepts a whole buffer and
reports the frames taken, which is the return the seam's `SubmitSamples`
will translate.

### The typed-failure path on a device that does not exist

```
open no-such-device-xyz           -> FAIL No such file or directory
open default                      -> FAIL No such file or directory
```

Both report a named failure (`snd_strerror`), which is what the seam's
typed result turns into `AUDIO_NO_DEVICE`. Note the honest finding: on
this machine **`default` itself fails** — there is no card 0 — so the
typed-failure path is the one a real run exercises here, and `null` is
the verification path. Exactly the situation design D7 routes around.

### A toolchain finding the sound work introduces

ALSA's header does not compile under the strict language law's flag:

```
gcc -std=c11 -O0 -g -Wall -Wextra probe.c -o probe -lasound
# error: redefinition of 'struct timespec'  (alsa/global.h vs bits/types/struct_timespec.h)
```

`-std=c11` sets `__STRICT_ANSI__`, glibc then keeps `struct timespec`
back, and ALSA's header declares its own fallback. Two fixes were
verified, both clean:

```
gcc -std=c11  -D_POSIX_C_SOURCE=200809L ... -lasound   # -> OK
gcc -std=gnu11                                  ... -lasound   # -> OK
```

**The engine's C++ flags need no change**: `g++ -std=c++17 -O0 -g -Wall
-Wextra` compiles `asoundlib.h` as-is (C++ mode exposes `timespec`
through glibc's headers), and the link with `-lasound` verifies under
`-Wl,-z,defs`. So `build.sh` needs one thing only when the audio file
lands: `-lasound` added to `LDFLAGS` — a build flag the lesson that
first needs it explains as it appears (Toolchain is curriculum).

### The boundary check's implementation list

`tools/check-boundary.sh` grew its `IMPL` list to name the audio
implementation file (`src/platform_alsa.cpp`) beside the X11 one, so the
day the seam's audio output lands the check already knows where the OS
is allowed to live. The list tolerates a file that does not exist yet.

### The exact commands, as written

```
./build.sh
mdbook build
openspec validate --all
./tools/check-boundary.sh
```

plus the scratch probe's two compile lines quoted above. All were run as
written and their output recorded here before the first sound lesson was
authored.

## 1.2 Seam and frame-record audit

Read against the `platform-layer` delta of this change and the
`frame-accounting` spec ("at least" clause). Recorded **before** the first
sound lesson was authored — nothing in `src/` or `book/` has been touched
for lesson-059 yet.

### What the seam must grow: audio output

`src/platform.h` already owns window and presentation, polled input, the
monotonic clock, memory reservations, whole-file I/O, and the event pump.
Part 3 adds **one new requirement beside them — audio output** (this
change's `platform-layer` delta):

- `OpenAudioOutput(rate, channels)` — a typed result naming the step that
  failed, exactly as `OpenWindow` does. The failure value is
  `AUDIO_NO_DEVICE`, beside `OPEN_NO_DISPLAY`.
- `SubmitSamples(const short *samples, int frames)` — the mixed buffer
  handed to the device; the seam carries bytes, the engine makes them.
- `CloseAudioOutput` — releases what the open took.
- **`PumpEvents`' wait grows bounded.** Today it "blocks until there is
  news"; with an output open that wait must not outlast the output's need
  for samples (the delta's third scenario). The wait stays *outside* the
  frame's measured phases — `PumpEvents` runs before the record's `t0` —
  so the paced wait never inflates a frame's `total`.
- **Implementation lives in `src/platform_alsa.cpp`**, already named in
  `tools/check-boundary.sh`'s implementation list (task 1.1). No OS type
  crosses the seam: the interface carries `short *` and `int`, and the
  check keeps proving it.

Verified against the delta: all three delta requirements (open at the
engine's format; submitted samples reach the device; the wait stays short
enough) map onto these four surface changes. No existing platform
requirement is altered except `PumpEvents`' wait bound.

### What the frame record must grow: the `audio` phase

`src/frame.h` / `src/frame.cpp` grow one phase beside update, render, and
present:

- `FrameRecord.audio` and `FrameStats.audio_sum`, summed like every other
  phase in `AccountFrame`;
- carried in the `frame N:` log line, in the record's own order —
  `update, audio, render (sprites, text, tilemap), present, total`;
- **a new row in `PrintFrameBudget`** — the table grows one row; rows stay
  measured sums and shares stay shares of the average frame.

**Spec check — no spec change needed.** `frame-accounting`'s "Per-frame
record" says the record SHALL hold phase durations *"at least the world
update, the render, and the presentation"* — the "at least" clause already
admits a fourth phase, exactly as D8 claims. The "Frame-time log"
requirement's floor ("at least the frame count, the average total, and the
worst frame") is untouched.

**Measurement boundary confirmed.** `frame-accounting`'s "Timing hooks
stay behind the measurement boundary" requires the account to read only
the seam's clock. The new phase keeps that: it is measured with
`platform::Now()` readings around the mix and its submit — no ALSA timing
call enters the record or the account. The named-phase discipline from
Part 2 carries unchanged: `audio` is a phase of the frame, and the render
rows remain *inside* render.

**What the row means.** `audio` wraps the mix *and* its submit. On the
`null` device the submit returns immediately, so the row reads as the
mix's cost — which is what lesson-070 reports. On real hardware a submit
can wait for room in the device's buffer, the same way `present` includes
the copy's sync; lesson-070 names this and routes it to the port
exercise.

### A verification finding that bounds what the lessons may claim

ALSA's `null` device **accepts samples instantly and discards them** —
measured: five 100 ms buffers were accepted in 0.0 ms wall time, at every
buffer/period size tried (4096/1024, 44100/4410, 441/220). There is no
hardware device here at all (`hw:0,0` fails to open).

Consequences, carried honestly into the lessons:

- The **submission path** is verified — open at the engine's format,
  submit, close — and the **typed failure** is verified on a device that
  does not exist. Both were run (§1.1).
- The **paced wait cannot be verified from device back-pressure** here,
  because `null` never pushes back. What *is* verifiable headlessly is the
  engine's side of the schedule: the wait is bounded by the buffer horizon
  (the loop wakes at the next buffer's deadline on the platform clock), so
  an idle run with the output open ticks at the audio cadence instead of
  blocking forever or spinning. That is what lesson-060's run may claim.
- Lesson-060 must **not** claim "the device did not starve" as a verified
  result on this machine. It claims the bounded wait and the cadence, and
  names real hardware's back-pressure as the port exercise — D7's honesty
  rule applied.
