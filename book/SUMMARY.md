# Summary

[Course home](index.md)

<!-- The curriculum parts (Part 0 through Part 5) land here as lessons are
     published, in curriculum order. plan/ holds authoring artifacts and is
     never part of this site: the book builds only from book/. -->

# Part 0 — C foundations

- [Lesson 001 — argv and file input: your first `gcc` command](lessons/part-0/lesson-001-first-program.md)
  - [Solution: ex1 — standard input](solutions/lesson-001/ex1.md)
  - [Solution: ex2 — the silent directory](solutions/lesson-001/ex2.md)
  - [Solution: ex3 — one character at a time](solutions/lesson-001/ex3.md)
  - [Solution: ex4 — bytes, not characters](solutions/lesson-001/ex4.md)

- [Lesson 002 — gdb: breakpoints, stepping, stack frames](lessons/part-0/lesson-002-gdb.md)
  - [Solution: ex1 — the second hit looks the same](solutions/lesson-002/ex1.md)
  - [Solution: ex2 — the file that closes twice](solutions/lesson-002/ex2.md)
  - [Solution: ex3 — conditions see one frame](solutions/lesson-002/ex3.md)
  - [Solution: ex4 — a counter in two scopes](solutions/lesson-002/ex4.md)

- [Lesson 003 — char buffers: strings by hand](lessons/part-0/lesson-003-char-buffers.md)
  - [Solution: ex1 — three hundred characters](solutions/lesson-003/ex1.md)
  - [Solution: ex2 — the quoted line lies](solutions/lesson-003/ex2.md)
  - [Solution: ex3 — the longest word](solutions/lesson-003/ex3.md)
  - [Solution: ex4 — the debugger on your machine](solutions/lesson-003/ex4.md)

- [Lesson 004 — malloc and free: growing buffers on the heap](lessons/part-0/lesson-004-heap-buffers.md)
  - [Solution: ex1 — the doubling sequence](solutions/lesson-004/ex1.md)
  - [Solution: ex2 — one byte at a time](solutions/lesson-004/ex2.md)
  - [Solution: ex3 — who owns the bytes](solutions/lesson-004/ex3.md)
  - [Solution: ex4 — standard input, revisited](solutions/lesson-004/ex4.md)

- [Lesson 005 — leaks made visible with sanitizers](lessons/part-0/lesson-005-leaks.md)
  - [Solution: ex1 — three verdicts](solutions/lesson-005/ex1.md)
  - [Solution: ex2 — what the certainty costs](solutions/lesson-005/ex2.md)
  - [Solution: ex3 — how the sanitizer knows](solutions/lesson-005/ex3.md)

- [Lesson 006 — undefined behavior and buffer overflows](lessons/part-0/lesson-006-undefined-behavior.md)
  - [Solution: ex1 — the value that is not promised](solutions/lesson-006/ex1.md)
  - [Solution: ex2 — the empty line that reads backwards](solutions/lesson-006/ex2.md)
  - [Solution: ex3 — three fates](solutions/lesson-006/ex3.md)
  - [Solution: ex4 — the price of the check](solutions/lesson-006/ex4.md)

- [Lesson 007 — structs: sizeof, alignment, and padding](lessons/part-0/lesson-007-struct-layout.md)
  - [Solution: ex1 — padding in a fresh struct](solutions/lesson-007/ex1.md)
  - [Solution: ex2 — the padding is real memory](solutions/lesson-007/ex2.md)
  - [Solution: ex3 — why the machine insists](solutions/lesson-007/ex3.md)
  - [Solution: ex4 — the other machine's layout](solutions/lesson-007/ex4.md)

- [Lesson 008 — dynarray growth: realloc and capacity](lessons/part-0/lesson-008-dynarray.md)
  - [Solution: ex1 — the growth schedule](solutions/lesson-008/ex1.md)
  - [Solution: ex2 — the half-freed array](solutions/lesson-008/ex2.md)
  - [Solution: ex3 — doubling versus one-at-a-time](solutions/lesson-008/ex3.md)
  - [Solution: ex4 — what `realloc` really promises](solutions/lesson-008/ex4.md)

- [Lesson 009 — function pointers: comparators and hooks](lessons/part-0/lesson-009-function-pointers.md)
  - [Solution: ex1 — the reversed order](solutions/lesson-009/ex1.md)
  - [Solution: ex2 — searching by predicate](solutions/lesson-009/ex2.md)
  - [Solution: ex3 — why the bridge exists](solutions/lesson-009/ex3.md)

- [Lesson 010 — void*: genericity and its pain](lessons/part-0/lesson-010-void-pointer.md)
  - [Solution: ex1 — the same bytes, differently](solutions/lesson-010/ex1.md)
  - [Solution: ex2 — the wrong size](solutions/lesson-010/ex2.md)
  - [Solution: ex3 — removing in the middle](solutions/lesson-010/ex3.md)
  - [Solution: ex4 — the contract, written down](solutions/lesson-010/ex4.md)

- [Lesson 011 — the hashtable: hashing, buckets, lookup](lessons/part-0/lesson-011-hashtable.md)
  - [Solution: ex1 — count on it](solutions/lesson-011/ex1.md)
  - [Solution: ex2 — the leaderboard](solutions/lesson-011/ex2.md)
  - [Solution: ex3 — the walk's length](solutions/lesson-011/ex3.md)
  - [Solution: ex4 — one bucket](solutions/lesson-011/ex4.md)

- [Lesson 012 — multi-file builds: translation units and linking](lessons/part-0/lesson-012-multi-file.md)
  - [Solution: ex1 — the helper that clashed](solutions/lesson-012/ex1.md)
  - [Solution: ex2 — table statistics](solutions/lesson-012/ex2.md)
  - [Solution: ex3 — the lowercase letter](solutions/lesson-012/ex3.md)

- [Lesson 013 — raw bytes and pixel formats](lessons/part-0/lesson-013-raw-bytes.md)
  - [Solution: ex1 — three pixels, by hand](solutions/lesson-013/ex1.md)
  - [Solution: ex2 — the same pixel in packed 32-bit](solutions/lesson-013/ex2.md)
  - [Solution: ex3 — offsets, by hand](solutions/lesson-013/ex3.md)
  - [Solution: ex4 — the pixel that is not there](solutions/lesson-013/ex4.md)

- [Lesson 014 — endianness and image-header layout](lessons/part-0/lesson-014-image-headers.md)
  - [Solution: ex1 — width 258](solutions/lesson-014/ex1.md)
  - [Solution: ex2 — the wrong way around](solutions/lesson-014/ex2.md)
  - [Solution: ex3 — structs do not make files](solutions/lesson-014/ex3.md)
  - [Solution: ex4 — ask your own machine](solutions/lesson-014/ex4.md)

- [Lesson 015 — fill-rect onto a memory buffer](lessons/part-0/lesson-015-fill-rect.md)
  - [Solution: ex1 — the rectangle off the left](solutions/lesson-015/ex1.md)
  - [Solution: ex2 — counting what survived](solutions/lesson-015/ex2.md)
  - [Solution: ex3 — the addition that ate the clip](solutions/lesson-015/ex3.md)
  - [Solution: ex4 — why fold first](solutions/lesson-015/ex4.md)

- [Lesson 016 — drawing lines onto the buffer](lessons/part-0/lesson-016-lines.md)
  - [Solution: ex1 — the vertical line through everything](solutions/lesson-016/ex1.md)
  - [Solution: ex2 — rectangle outlines](solutions/lesson-016/ex2.md)
  - [Solution: ex3 — float versus integer](solutions/lesson-016/ex3.md)
  - [Solution: ex4 — the error term, watched](solutions/lesson-016/ex4.md)

- [Lesson 017 — writing a real image file by hand](lessons/part-0/lesson-017-image-file.md)
  - [Solution: ex1 — rows on disk](solutions/lesson-017/ex1.md)
  - [Solution: ex2 — round trip](solutions/lesson-017/ex2.md)
  - [Solution: ex3 — the flipped image](solutions/lesson-017/ex3.md)
  - [Solution: ex4 — per byte versus per row](solutions/lesson-017/ex4.md)

- [Lesson 018 — the optimizer and undefined behavior](lessons/part-0/lesson-018-optimizer-ub.md)
  - [Solution: ex1 — predict the wreckage](solutions/lesson-018/ex1.md)
  - [Solution: ex2 — the guard that gets deleted](solutions/lesson-018/ex2.md)
  - [Solution: ex3 — review a colleague's fill](solutions/lesson-018/ex3.md)
  - [Solution: ex4 — prove the fix](solutions/lesson-018/ex4.md)

- [Lesson 019 — the game loop](lessons/part-0/lesson-019-game-loop.md)
  - [Solution: ex1 — the life of `strtoul`](solutions/lesson-019/ex1.md)
  - [Solution: ex2 — the string that never ends](solutions/lesson-019/ex2.md)
  - [Solution: ex3 — pack the state](solutions/lesson-019/ex3.md)
  - [Solution: ex4 — why three phases](solutions/lesson-019/ex4.md)

- [Lesson 020 — timing with `clock_gettime`](lessons/part-0/lesson-020-timing.md)
  - [Solution: ex1 — predicting the ticks](solutions/lesson-020/ex1.md)
  - [Solution: ex2 — what the frame cap costs](solutions/lesson-020/ex2.md)
  - [Solution: ex3 — two clocks](solutions/lesson-020/ex3.md)
  - [Solution: ex4 — a thing that moves](solutions/lesson-020/ex4.md)

- [Lesson 021 — raw terminal input with escape codes](lessons/part-0/lesson-021-terminal-input.md)
  - [Solution: ex1 — bytes all the way down](solutions/lesson-021/ex1.md)
  - [Solution: ex2 — the key that vanished](solutions/lesson-021/ex2.md)
  - [Solution: ex3 — cSI, SS3, and your terminal](solutions/lesson-021/ex3.md)
  - [Solution: ex4 — wASD](solutions/lesson-021/ex4.md)

- [Lesson 022 — the double-buffered character grid](lessons/part-0/lesson-022-double-buffer.md)
  - [Solution: ex1 — counting the flush](solutions/lesson-022/ex1.md)
  - [Solution: ex2 — one off, twice](solutions/lesson-022/ex2.md)
  - [Solution: ex3 — what the diff saves](solutions/lesson-022/ex3.md)
  - [Solution: ex4 — runs, not cells](solutions/lesson-022/ex4.md)

- [Lesson 023 — the state machine: title, play, death](lessons/part-0/lesson-023-state-machine.md)
  - [Solution: ex1 — counting to the wall](solutions/lesson-023/ex1.md)
  - [Solution: ex2 — the restart that wasn't](solutions/lesson-023/ex2.md)
  - [Solution: ex3 — pause](solutions/lesson-023/ex3.md)
  - [Solution: ex4 — why states, not flags](solutions/lesson-023/ex4.md)

- [Lesson 024 — the function-pointer command table](lessons/part-0/lesson-024-command-table.md)
  - [Solution: ex1 — wASD is four rows](solutions/lesson-024/ex1.md)
  - [Solution: ex2 — enter is a carriage return](solutions/lesson-024/ex2.md)
  - [Solution: ex3 — two rows, one key](solutions/lesson-024/ex3.md)
  - [Solution: ex4 — where the functions live](solutions/lesson-024/ex4.md)
- [Lesson 025 — the C++ subset: classes and vtables](lessons/part-0/lesson-025-cpp-subset.md)
  - [Solution: ex1 — a third view](solutions/lesson-025/ex1.md)
  - [Solution: ex2 — the hidden word](solutions/lesson-025/ex2.md)
  - [Solution: ex3 — the vtable under the glass](solutions/lesson-025/ex3.md)
  - [Solution: ex4 — the copy that cannot exist](solutions/lesson-025/ex4.md)

# Part 1 — the platform layer

- [Lesson 026 — the codebase is born](lessons/part-1/lesson-026-birth.md)
  - [Solution: ex1 — the engine speaks its name](solutions/lesson-026/ex1.md)
  - [Solution: ex2 — two translation units](solutions/lesson-026/ex2.md)

- [Lesson 027 — the platform seam and the first X11 window](lessons/part-1/lesson-027-first-window.md)
  - [Solution: ex1 — the window gets your title](solutions/lesson-027/ex1.md)
  - [Solution: ex2 — where the boundary is](solutions/lesson-027/ex2.md)

- [Lesson 028 — the event pump: keeping the window alive and reporting close](lessons/part-1/lesson-028-event-pump.md)
  - [Solution: ex1 — resize is news too](solutions/lesson-028/ex1.md)
  - [Solution: ex2 — two ways the news arrives](solutions/lesson-028/ex2.md)

- [Lesson 029 — clean close and error paths: OS resources released on every exit](lessons/part-1/lesson-029-clean-close.md)
  - [Solution: ex1 — the window that gets opened twice](solutions/lesson-029/ex1.md)
  - [Solution: ex2 — the ledger](solutions/lesson-029/ex2.md)

- [Lesson 030 — the framebuffer as our own bytes](lessons/part-1/lesson-030-framebuffer.md)
  - [Solution: ex1 — fillrect, back from Part 0](solutions/lesson-030/ex1.md)
  - [Solution: ex2 — the four bytes](solutions/lesson-030/ex2.md)

- [Lesson 031 — presentation through the platform layer](lessons/part-1/lesson-031-present.md)
  - [Solution: ex1 — the presentation check](solutions/lesson-031/ex1.md)
  - [Solution: ex2 — cover and reveal](solutions/lesson-031/ex2.md)

- [Lesson 032 — polled input state](lessons/part-1/lesson-032-polled-input.md)
  - [Solution: ex1 — your own keys](solutions/lesson-032/ex1.md)
  - [Solution: ex2 — the press that vanished](solutions/lesson-032/ex2.md)

- [Lesson 033 — latching brief presses and tracking focus](lessons/part-1/lesson-033-latching.md)
  - [Solution: ex1 — the press count](solutions/lesson-033/ex1.md)
  - [Solution: ex2 — the key that will not let go](solutions/lesson-033/ex2.md)

- [Lesson 034 — the first interactive frame](lessons/part-1/lesson-034-first-frame.md)
  - [Solution: ex1 — eight directions](solutions/lesson-034/ex1.md)
  - [Solution: ex2 — the speed that belongs to the keyboard](solutions/lesson-034/ex2.md)

- [Lesson 035 — the platform clock](lessons/part-1/lesson-035-clock.md)
  - [Solution: ex1 — the diagonal is too fast](solutions/lesson-035/ex1.md)
  - [Solution: ex2 — the clock that lies](solutions/lesson-035/ex2.md)

- [Lesson 036 — frame time as measured data](lessons/part-1/lesson-036-frame-time.md)
  - [Solution: ex1 — the copy, isolated](solutions/lesson-036/ex1.md)
  - [Solution: ex2 — the frame budget](solutions/lesson-036/ex2.md)

- [Lesson 037 — whole-file reads](lessons/part-1/lesson-037-file-read.md)
  - [Solution: ex1 — read into your own memory](solutions/lesson-037/ex1.md)
  - [Solution: ex2 — the file that never ends](solutions/lesson-037/ex2.md)

- [Lesson 038 — whole-file writes and round-trips](lessons/part-1/lesson-038-file-write.md)
  - [Solution: ex1 — the screenshot](solutions/lesson-038/ex1.md)
  - [Solution: ex2 — the device that is always full](solutions/lesson-038/ex2.md)

- [Lesson 039 — the virtual-memory deep dive](lessons/part-1/lesson-039-virtual-memory.md)
  - [Solution: ex1 — find your mapping](solutions/lesson-039/ex1.md)
  - [Solution: ex2 — the file that lies about its size](solutions/lesson-039/ex2.md)

- [Lesson 040 — reservation-backed buffers](lessons/part-1/lesson-040-reservations.md)
  - [Solution: ex1 — one byte, please](solutions/lesson-040/ex1.md)
  - [Solution: ex2 — the page that fights back](solutions/lesson-040/ex2.md)

- [Lesson 041 — arenas](lessons/part-1/lesson-041-arenas.md)
  - [Solution: ex1 — the marker's trail](solutions/lesson-041/ex1.md)
  - [Solution: ex2 — the bug asan cannot see](solutions/lesson-041/ex2.md)

- [Lesson 042 — the interface as a contract](lessons/part-1/lesson-042-contract.md)
  - [Solution: ex1 — the second os](solutions/lesson-042/ex1.md)
  - [Solution: ex2 — what the check cannot see](solutions/lesson-042/ex2.md)

- [Lesson 043 — the closing demo: platform layer done](lessons/part-1/lesson-043-demo.md)
  - [Solution: ex1 — platform layer done, on your machine](solutions/lesson-043/ex1.md)
  - [Solution: ex2 — the acceptance table](solutions/lesson-043/ex2.md)

# Part 2 — software rendering

- [Lesson 044 — a sprite as loaded bytes](lessons/part-2/lesson-044-sprite-bytes.md)
  - [Solution: ex1 — the header that lies](solutions/lesson-044/ex1.md)
  - [Solution: ex2 — sprite, meet window](solutions/lesson-044/ex2.md)

- [Lesson 045 — the clipped, transparent blit](lessons/part-2/lesson-045-blit.md)
  - [Solution: ex1 — your own key](solutions/lesson-045/ex1.md)
  - [Solution: ex2 — the four corners](solutions/lesson-045/ex2.md)

- [Lesson 046 — the sprite moves](lessons/part-2/lesson-046-movable-sprite.md)
  - [Solution: ex1 — where does render go?](solutions/lesson-046/ex1.md)
  - [Solution: ex2 — the sprite that wraps](solutions/lesson-046/ex2.md)

- [Lesson 047 — the caches deep dive](lessons/part-2/lesson-047-caches.md)
  - [Solution: ex1 — your machine's knees](solutions/lesson-047/ex1.md)
  - [Solution: ex2 — the stride that doesn't fit the line](solutions/lesson-047/ex2.md)

- [Lesson 048 — the blitter's compiled assembly](lessons/part-2/lesson-048-assembly.md)
  - [Solution: ex1 — the instruction budget](solutions/lesson-048/ex1.md)
  - [Solution: ex2 — the compiler's signature](solutions/lesson-048/ex2.md)

- [Lesson 049 — the SIMD lens](lessons/part-2/lesson-049-simd.md)
  - [Solution: ex1 — two fills, predicted](solutions/lesson-049/ex1.md)
  - [Solution: ex2 — your compiler's register width](solutions/lesson-049/ex2.md)

- [Lesson 050 — the bitmap font as an asset](lessons/part-2/lesson-050-font.md)
  - [Solution: ex1 — the font dump](solutions/lesson-050/ex1.md)
  - [Solution: ex2 — the bytes the sheet never heard of](solutions/lesson-050/ex2.md)

- [Lesson 051 — text on screen](lessons/part-2/lesson-051-text.md)
  - [Solution: ex1 — the HUD that counts](solutions/lesson-051/ex1.md)
  - [Solution: ex2 — the two kinds of nothing](solutions/lesson-051/ex2.md)

- [Lesson 052 — the tilemap asset format](lessons/part-2/lesson-052-tilemap.md)
  - [Solution: ex1 — the failure that names itself](solutions/lesson-052/ex1.md)
  - [Solution: ex2 — the map as characters](solutions/lesson-052/ex2.md)

- [Lesson 053 — tilemap drawing](lessons/part-2/lesson-053-tiles.md)
  - [Solution: ex1 — the tile under the sprite](solutions/lesson-053/ex1.md)
  - [Solution: ex2 — the empty draw](solutions/lesson-053/ex2.md)

- [Lesson 054 — the camera](lessons/part-2/lesson-054-camera.md)
  - [Solution: ex1 — the shake that decays](solutions/lesson-054/ex1.md)
  - [Solution: ex2 — the additive that cancels](solutions/lesson-054/ex2.md)

- [Lesson 055 — tile kinds and solidity](lessons/part-2/lesson-055-collision.md)
  - [Solution: ex1 — the policy is yours](solutions/lesson-055/ex1.md)
  - [Solution: ex2 — the rectangle at the boundary](solutions/lesson-055/ex2.md)

- [Lesson 056 — the mover that stops at walls](lessons/part-2/lesson-056-mover.md)
  - [Solution: ex1 — the hitbox](solutions/lesson-056/ex1.md)
  - [Solution: ex2 — the step and the wall](solutions/lesson-056/ex2.md)

- [Lesson 057 — the closing demo: the world, drawn](lessons/part-2/lesson-057-demo.md)
  - [Solution: ex1 — the Part 2 demo on your machine](solutions/lesson-057/ex1.md)
  - [Solution: ex2 — the acceptance table](solutions/lesson-057/ex2.md)

- [Lesson 058 — the frame-budget table](lessons/part-2/lesson-058-budget.md)
  - [Solution: ex1 — the row that isn't there](solutions/lesson-058/ex1.md)
  - [Solution: ex2 — the worst frame's column](solutions/lesson-058/ex2.md)

# Part 3 — sound

- [Lesson 059 — sound as samples](lessons/part-3/lesson-059-samples.md)
  - [Solution: ex1 — the first eight frames](solutions/lesson-059/ex1.md)
  - [Solution: ex2 — hear it on real speakers](solutions/lesson-059/ex2.md)

- [Lesson 060 — the stream's shape](lessons/part-3/lesson-060-stream.md)
  - [Solution: ex1 — the horizon, doubled](solutions/lesson-060/ex1.md)
  - [Solution: ex2 — starvation on real speakers](solutions/lesson-060/ex2.md)

- [Lesson 061 — the WAV container](lessons/part-3/lesson-061-wav.md)
  - [Solution: ex1 — write the file back](solutions/lesson-061/ex1.md)
  - [Solution: ex2 — the file that ends too soon](solutions/lesson-061/ex2.md)

- [Lesson 062 — the sample's playback facts](lessons/part-3/lesson-062-playback.md)
  - [Solution: ex1 — the sample that is not thirty buffers](solutions/lesson-062/ex1.md)
  - [Solution: ex2 — hear it stop](solutions/lesson-062/ex2.md)

- [Lesson 063 — one channel](lessons/part-3/lesson-063-channel.md)
  - [Solution: ex1 — quarter volume, to the digit](solutions/lesson-063/ex1.md)
  - [Solution: ex2 — the fade to silence](solutions/lesson-063/ex2.md)

- [Lesson 064 — the mix](lessons/part-3/lesson-064-mix.md)
  - [Solution: ex1 — five channels at the peak](solutions/lesson-064/ex1.md)
  - [Solution: ex2 — what the mix costs](solutions/lesson-064/ex2.md)

- [Lesson 065 — channel allocation](lessons/part-3/lesson-065-allocation.md)
  - [Solution: ex1 — the twenty-first press](solutions/lesson-065/ex1.md)
  - [Solution: ex2 — the policy that refuses](solutions/lesson-065/ex2.md)

- [Lesson 066 — music as a loop](lessons/part-3/lesson-066-music.md)
  - [Solution: ex1 — the seam, predicted](solutions/lesson-066/ex1.md)
  - [Solution: ex2 — pause and resume](solutions/lesson-066/ex2.md)

- [Lesson 067 — effects as one-shots](lessons/part-3/lesson-067-effects.md)
  - [Solution: ex1 — one sample, two volumes](solutions/lesson-067/ex1.md)
  - [Solution: ex2 — the effect that must not stack](solutions/lesson-067/ex2.md)

- [Lesson 068 — music and effects together](lessons/part-3/lesson-068-together.md)
  - [Solution: ex1 — the audio phase, measured](solutions/lesson-068/ex1.md)
  - [Solution: ex2 — the music that ducks](solutions/lesson-068/ex2.md)

- [Lesson 069 — the closing demo](lessons/part-3/lesson-069-demo.md)
  - [Solution: ex1 — the Part 3 demo on your machine](solutions/lesson-069/ex1.md)
  - [Solution: ex2 — the part's acceptance table](solutions/lesson-069/ex2.md)

- [Lesson 070 — the mix's cost in the frame budget](lessons/part-3/lesson-070-audio-row.md)
  - [Solution: ex1 — the row's two insides](solutions/lesson-070/ex1.md)
  - [Solution: ex2 — the row's two populations](solutions/lesson-070/ex2.md)

# Part 4 — services

- [Lesson 071 — the archetype table](lessons/part-4/lesson-071-table.md)
  - [Solution: ex1 — the header, reordered](solutions/lesson-071/ex1.md)
  - [Solution: ex2 — the table written back](solutions/lesson-071/ex2.md)

- [Lesson 072 — the load, complete or named](lessons/part-4/lesson-072-load.md)
  - [Solution: ex1 — the table with a hole in it](solutions/lesson-072/ex1.md)
  - [Solution: ex2 — the load's cost, measured](solutions/lesson-072/ex2.md)

- [Lesson 073 — entities as rows](lessons/part-4/lesson-073-rows.md)
  - [Solution: ex1 — two heroes, one row](solutions/lesson-073/ex1.md)
  - [Solution: ex2 — every definition, an entity](solutions/lesson-073/ex2.md)

- [Lesson 074 — one fixed store](lessons/part-4/lesson-074-store.md)
  - [Solution: ex1 — the refusal that gets ignored](solutions/lesson-074/ex1.md)
  - [Solution: ex2 — the capacity you would pick](solutions/lesson-074/ex2.md)

- [Lesson 075 — the walk and the free slot](lessons/part-4/lesson-075-lifetime.md)
  - [Solution: ex1 — the walk that retires ahead](solutions/lesson-075/ex1.md)
  - [Solution: ex2 — the store's map](solutions/lesson-075/ex2.md)

- [Lesson 076 — the hero as an entity](lessons/part-4/lesson-076-hero.md)
  - [Solution: ex1 — the step, predicted](solutions/lesson-076/ex1.md)
  - [Solution: ex2 — the hero on your desktop](solutions/lesson-076/ex2.md)

- [Lesson 077 — the mover on an entity](lessons/part-4/lesson-077-mover.md)
  - [Solution: ex1 — the corner, predicted](solutions/lesson-077/ex1.md)
  - [Solution: ex2 — which wall said no](solutions/lesson-077/ex2.md)

- [Lesson 078 — the game-time scale](lessons/part-4/lesson-078-game-time.md)
  - [Solution: ex1 — hitstop that ends](solutions/lesson-078/ex1.md)
  - [Solution: ex2 — one knob, not three](solutions/lesson-078/ex2.md)

- [Lesson 079 — measurement is not scaled](lessons/part-4/lesson-079-wall-clock.md)
  - [Solution: ex1 — the paused frame, predicted](solutions/lesson-079/ex1.md)
  - [Solution: ex2 — what a paused game costs](solutions/lesson-079/ex2.md)

- [Lesson 080 — the vertical slice](lessons/part-4/lesson-080-slice.md)
  - [Solution: ex1 — the camera at the map's edges](solutions/lesson-080/ex1.md)
  - [Solution: ex2 — the gate, answered](solutions/lesson-080/ex2.md)

- [Lesson 081 — the slice's cost in the frame budget](lessons/part-4/lesson-081-entities-row.md)
  - [Solution: ex1 — the row, at two sizes](solutions/lesson-081/ex1.md)
  - [Solution: ex2 — what the row does not say](solutions/lesson-081/ex2.md)

# Part 5 — the game

- [Lesson 082 — the game skeleton](lessons/part-5/lesson-082-skeleton.md)
  - [Solution: ex1 — the score on the end screens](solutions/lesson-082/ex1.md)
  - [Solution: ex2 — why the scale](solutions/lesson-082/ex2.md)

- [Lesson 083 — the tilemap and camera](lessons/part-5/lesson-083-tilemap-camera.md)
  - [Solution: ex1 — the camera that leads](solutions/lesson-083/ex1.md)
  - [Solution: ex2 — the base, at the corners](solutions/lesson-083/ex2.md)

- [Lesson 084 — tile collision](lessons/part-5/lesson-084-collision.md)
  - [Solution: ex1 — the corner and the wall](solutions/lesson-084/ex1.md)
  - [Solution: ex2 — no tunneling](solutions/lesson-084/ex2.md)

- [Lesson 085 — hero movement](lessons/part-5/lesson-085-hero-movement.md)
  - [Solution: ex1 — the curve, predicted](solutions/lesson-085/ex1.md)
  - [Solution: ex2 — heavier to stop](solutions/lesson-085/ex2.md)

# Sample

- [Sample lesson — filling a rectangle](lessons/sample-lesson.md)
  - [Solution: ex1 — draw an outline](solutions/lesson-000/ex1.md)
  - [Solution: ex2 — the crashing rectangle](solutions/lesson-000/ex2.md)
