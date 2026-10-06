# Change notes

## Toolchain verification for the rendering batch

**Date:** 2026-10-06 · **Task:** 1.1 · Every command below runs as written on
this authoring machine (headless; `Xvfb :99` up). The probes live outside the
repository (`/tmp/opencode/part2-verify/`), Part 1's pattern: verification
machinery is authoring-side, the repo keeps `tools/check-boundary.sh` only.

### Versions used

| Tool | Version |
| ---- | ------- |
| g++ / gcc | 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) |
| GNU objdump (binutils) | 2.42 (Ubuntu) |
| libx11-dev (Xlib headers + `-lX11`) | 2:1.8.7-1build1 |
| Xvfb | 2:21.1.12-1ubuntu1.8 (display `:99`, 800x600x24) |
| xdotool (scripted input) | 3.20160805.1 |
| taskset (util-linux) | 2.39.3 |
| glibc | 2.39 |
| mdBook | 0.5.4 (pinned) |
| CPU | Intel(R) Core(TM) Ultra 5 225F — L1d 48K, L1i 64K, L2 3M (unified), L3 20M (unified), 64-byte cache lines (`/sys/devices/system/cpu/cpu0/cache/index*`) |
| perf / valgrind | **not installed** — the cache method below is clock-based on purpose |

### 1. Pixel-level readback against the framebuffer (sprite, clip, camera)

The drawing claims check headlessly at two levels; both ran today.

**Engine-side (`fbcheck`)** — draw through the engine's own functions, read
every pixel back with `GetPixel`, compare byte-for-byte. The harness draws a
4x3 sprite (opaque corners, transparent middle column) through a miniature
blit — the shape lesson-045 grows — and checks all four claim classes:

```
$ g++ -std=c++17 -O0 -g -Wall -Wextra -Isrc fbcheck.cpp src/framebuffer.cpp \
      src/arena.cpp src/platform_x11.cpp -o fbcheck -lX11
$ ./fbcheck
framebuffer 640x480 at 0x762f30200000 (1228800 bytes in arena, used 1228800)
camera move (37,25): same pixels, moved
fbcheck: OK
```

- **sprite** — every non-transparent pixel read back with the exact color it
  carried; untouched pixels keep their background;
- **transparency** — the key-colored pixels leave the background unchanged;
- **clip** — draws at `(-2,-1)` and `(638,479)` land only the in-bounds
  pixels, at the right places, with no wrap;
- **camera** — the same scene at offsets `(0,0)` and `(37,25)` read back as
  *the same pixels, moved* (world position minus the offset).

**Window-side (`winread`)** — the presentation path, lesson 031's check in
readback form: an Xlib probe finds the engine's window by name, `XGetImage`s
it, prints pixels at requested coordinates.

```
$ DISPLAY=:99 ./build/game &        # the lesson-043 demo, marker at 308,228
$ DISPLAY=:99 ./winread "the framebuffer engine" 310 230 10 10 308 228
winread: window 4194305 is 640x480
winread: 310,230 -> r=240 g=220 b=80
winread: 10,10 -> r=32 g=32 b=64
winread: 308,228 -> r=240 g=220 b=80
```

Marker color `240,220,80` at the marker's top-left and inside it, background
`32,32,64` away from it — window pixels match the framebuffer byte-for-byte.
The same two probes verify every Part 2 drawing claim: `fbcheck` for the
spec's pixel scenarios, `winread` for "the demo shows what it says".

### 2. Asset round-trip through the seam's whole-file I/O

`assetcheck` writes 256 patterned bytes with `platform::WriteFile`, reads
them back with `platform::ReadFile`, compares byte-for-byte, and takes the
typed-failure path on a missing file (lesson 037's contract — what every
sprite, font, and tilemap load rides):

```
$ g++ -std=c++17 -O0 -g -Wall -Wextra -Isrc assetcheck.cpp src/platform_x11.cpp \
      -o assetcheck -lX11
$ ./assetcheck
assetcheck: missing file -> error 1 (FILE_NOT_FOUND=1)
assetcheck: OK — 256 bytes round-trip, missing file typed
```

### 3. Disassembly tooling (what the deep dives quote)

`objdump -d -C --no-show-raw-insn` over a copy-loop probe (the blit's shape)
compiled at three levels. Extraction by symbol is stable:

```
$ objdump -d -C --no-show-raw-insn build/game | sed -n '/<Blit/,/^$/p'
```

Verified against the probe (`CopyRow`, `extern "C"`):

- **`-O0`** — the byte loop with stack spills, `movzbl`/`mov`, frame pointer:
  exactly the naive reading lesson-048 walks through instruction by
  instruction.
- **`-O2`** — the loop is *still* byte-wise (`movzbl (%rsi,%rax,1),%ecx` /
  `mov %cl,(%rdi,%rax,1)`): GCC 13's `-O2` vectorization cost model declines
  this loop. Real and quotable — the deep dive will say so.
- **`-O3`** — the vectorized form the compiler reaches for: an aliasing
  guard, a 16-byte `movdqu (%rsi,%rdx,1),%xmm0` / `movups %xmm0,(%rcx,%rdx,1)`
  main loop, then an 8-byte tail and a byte tail. This is lesson-049's
  reading (vector registers read, not written).

So the SIMD lesson quotes `CXXFLAGS="-std=c++17 -O3 -g -Wall -Wextra" ./build.sh`
output — the same build line, one flag changed in the open.

### 4. Repeatable cache-measurement method (lesson-047)

No `perf` on this machine, so the method is **clock-based and pinned**: one
copy loop (`CopyRows`) timed over working sets doubling 4 KB → 64 MB,
`taskset -c 3` to keep the process on one core, three consecutive runs to
show what repeats and what does not:

```
$ g++ -std=c++17 -O3 -g -Wall -Wextra copyloop.cpp -o copyloop-O3
$ taskset -c 3 ./copyloop-O3
```

| Workset | run 1 | run 2 | run 3 | region |
| ------- | ----- | ----- | ----- | ------ |
| 16 KB | 199.5 GB/s | 198.7 GB/s | 192.6 GB/s | L1d (48K) |
| 256 KB | 70.1 GB/s | 71.5 GB/s | 68.1 GB/s | L1d→L2 |
| 1024 KB | 67.7 GB/s | 70.0 GB/s | 69.5 GB/s | L2 (3M) |
| 4096 KB | 21.1 GB/s | 17.5 GB/s | 22.0 GB/s | L3 (20M) |
| 32768 KB | 13.5 GB/s | 13.1 GB/s | 13.1 GB/s | memory |

**The shape repeats; the digits do not.** The knees land at the same places
in every run (past L1d at 48K, past L2 at ~2-3 MB, past L3 at ~20 MB), while
single cells move ±10%+ (see 4096 KB). Lesson-047 quotes the shape, names
the machine and the pin, and says which of the two a reader should trust.
That is the method Part 5's optimization passes reuse.

*Verified: all four check families above ran as written before any Part 2
lesson is authored.*

## Frame-accounting audit: lesson-036's record and log ↔ `frame-accounting`

**Date:** 2026-10-06 · **Task:** 1.2 · Read of `frame.h`, `frame.cpp`, and
the frame loop in `main.cpp` (the `lesson-043` end state) against
`specs/frame-accounting/spec.md`. *Recorded before the blitter batch is
authored, as the task requires.*

### Requirement-by-requirement

| Spec requirement | `lesson-036` state | Verdict |
| ---------------- | ------------------ | ------- |
| Per-frame record: update, render, presentation + total on the monotonic clock | `FrameRecord { number, update, render, present, total }`, every field a `platform::Now()` difference around phase boundaries | **holds** |
| Exactly one record per completed frame | `frame.number = ++frame_number` and `AccountFrame` run once per loop pass; a frame whose `Present` fails breaks before accounting — it never completed, so no record exists for it. The spec's scenario is phrased on *completed* frames | **holds** (edge case checked, see note) |
| Numbers measured, not estimated | each phase is bracketed by two clock reads and subtracted; `present` includes the sync, as lesson 031's contract makes honest | **holds** |
| One log line per frame, stable text format naming frame number + durations | `frame %ld: update %.3f ms, render %.3f ms, present %.3f ms, total %.3f ms` — one `printf` per record, the format the record's fields define | **holds** |
| Running account: frame count, average total, worst frame with its number | `FrameStats` sums every phase and tracks `worst`/`worst_number`; the closing summary prints count, per-phase averages, and the worst frame's duration and number | **holds** |
| Measurement behind the boundary: only the seam's clock and phase boundaries | engine code reads `platform::Now()` only; `tools/check-boundary.sh` keeps OS timing calls out of `src/` | **holds** |

Nothing in the `lesson-036` state violates the spec — the audit found no
fix needed in the tagged state.

### What Part 2 must grow: per-subsystem attribution

The spec's record is the floor ("at least the world update, the render, and
the presentation"). The frame-budget table (task 8.2 / lesson-058, O1's
payoff) attributes the frame to **subsystems**, and a single `render`
duration cannot say what inside rendering cost what. Lesson-036's promise —
"Neither will have to change what a record is — only what the engine puts in
it" — is the contract the growth honors: **additive fields, same record.**

Growth plan, one named phase per subsystem as the batches land (design D1):

| Batch | Record grows | Why there |
| ----- | ------------ | --------- |
| Blitter (lesson-046) | `sprites` — time in sprite draws, inside `render` | the first subsystem the render phase has; the record's render phase gets its name here (task 2.3) |
| Text (lesson-051) | `text` — time in text drawing | O3 lands; text is its own subsystem in the table |
| Tilemap (lesson-053) | `tilemap` — time in the map walk | O4 lands; the walk is a distinct cost the caches dive already explains |
| Close (lesson-058) | `clear` completes the attribution; the account sums per subsystem | the frame-budget table's rows |

Rules the growth keeps (they are what makes the table trustworthy):

1. `update`, `render`, `present`, `total` stay exactly as they are — the
   spec's minimum, and lesson-036's published format. Sub-attribution lives
   in new fields whose sum is *checked against* `render`, not substituted
   for it.
2. The log stays **one line per record**, extended with named fields — the
   "stable text format" requirement is about the format staying readable as
   data, not about being frozen at four fields.
3. Attribution reads only the seam's clock around named boundaries — the
   "behind the measurement boundary" requirement extends to the sub-phases
   (a subsystem that measures itself with its own timer breaks the rule the
   spec writes).
4. The account (`FrameStats`) grows per-subsystem sums alongside its phase
   sums; the frame-budget table is those sums as shares of the frame — the
   same "worst frame; present is 54% of the frame" honesty, per row.

*Verified: this audit and the growth plan above are recorded here before the
blitter batch (task 2.1, lesson-044) is authored.*

## Class-2 fix: the tile sheet's raster order (lesson-053)

**Date:** 2026-10-06 · Found while verifying lesson-056's mover. The
lesson-053 tile sheet (`assets/tiles.ppm`) was authored cell by cell
while the PPM format is **raster order** — row by row across the whole
sheet — so the 16×16 cells were scrambled: every cell held bands of all
three tile arts. The tilemap check of lesson-053 still passed (it
compares two draws of the same art), but the art the prose describes —
floor, brick, water — was not what the file contained, and the
window readback of any single tile contradicted the map's data.

Procedure per the authoring contract (the lessons are volatile tail, so
tags may move for correctness fixes):

1. The sheet was regenerated in raster order and verified cell by cell
   (each cell reads back as one coherent art) and through the engine
   (window readback at known map cells: wall mortar at (8,8), floor at
   (256,224), water at (528,416)).
2. The fix was amended into the **lesson-053** commit — the lesson that
   introduced the asset — and lessons 054-056 rebased forward on top of
   it (`git rebase -i lesson-052`, amend, continue).
3. Tags `lesson-053` … `lesson-056` were re-pointed at the rebased
   commits; every tag from 053 on now carries the corrected sheet
   (md5 `598ef4c7…`). A backup ref `backup-before-fix` holds the
   pre-fix history.
4. No lesson page or quoted number changed: the tilemap and camera
   checks compare draws against each other (art-independent), the
   collision checks are data-only, and the pages' art descriptions are
   now true at every tag that carries them.

*Verified: the asset reads correctly at each tag; the full check suite
(blit, font, text, map, tiles, tilemap, camera, collision) passes at the
rebased tip.*
