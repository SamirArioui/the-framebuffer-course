# Solution: exercise 2 — the epilogue map on your machine

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 102 — the retrospective: our engine against real ones](../../lessons/part-5/lesson-102-retrospective.md).*

## The diff

None, on purpose: this exercise writes no code, so `ex2.patch` is
empty — there is nothing to apply and nothing to check. The deliverable
is the map; a worked answer for one machine follows, and yours will
differ where the machine differs.

## Walkthrough

A worked re-ordering for **the authoring rig itself** (WSL2 under
Windows, Xvfb `:99`, no sound device), which is the degenerate case:
a machine that is *already* inside the Windows world's seam and has
nothing to port to.

**Which piece first, and which rows decide it.** The report's `present`
row decides: `0.438 ms` of wall for `0.004 ms` of CPU is the seam's
wait, and on a real desktop display that row's *shape* changes (the
copy's cost lands in process CPU instead of server wait) — but the row
the GPU port would erase is `render` (`0.799 ms`: `clear 0.244`,
`tilemap 0.538`). On this machine the frame's cost is not the problem
(`1.313 ms` of a `16.667 ms` budget, `0 of 2583` frames over it), so
neither piece is demanded by measurement here — and per the discipline
that is the honest answer: **the map's order is for machines that hurt,
and this one does not.** If it did, the two candidates separate
cleanly: rendering cost (`render`'s rows, CPU-bound) says GPU port;
platform cost (`present`, or a device this OS's files cannot reach)
says Windows module.

**The module's first milestone, in Windows' terms.** The contract's
list from `platform.h`, in the API a `platform_win32.cpp` would sit on:
`OpenWindow`/`CloseWindow` (a Win32 window class and `HWND`), the key
state pair and `HasFocus` (the message pump's `WM_KEYDOWN` state and
`WM_SETFOCUS`), `Now` (`QueryPerformanceCounter`), `PageSize`
(`GetSystemInfo`), the reservation pair (`VirtualAlloc`/
`VirtualFree`), the file trio (`CreateFile`/`ReadFile`/`WriteFile`),
`PumpEvents`/`CloseRequested` (`PeekMessage` and `WM_CLOSE`),
`Present` (the same BGRA bytes — `SetDIBitsToDevice` or a
back-buffer copy, keeping "when it returns true the pixels are on
screen"), and `OpenAudioOutput`'s feed (WASAPI shared mode, the same
44.1 kHz mono 16-bit contract). The typed failures map one-for-one
(`OPEN_NO_DISPLAY`, `FILE_NOT_FOUND`, `AUDIO_NO_DEVICE`, …) and the
boundary check's implementation list grows to name the new files — no
engine file changes.

**The port's first milestone, in the report's terms.** Keep the pixel
contract; replace the presentation path first (the `present` row's
wall time becomes a swap and its CPU share becomes the driver's);
then the clear (`clear 0.244 ms` of CPU goes to the GPU's fill), then
the map's draw (`tilemap 0.538 ms` becomes draw calls — and with it
the lesson-099 lever stops applying). The rows that survive untouched:
`update`, `entities`, `audio`, `text`'s logic. What replaces the copy
loop is a swap — and the frame account keeps measuring, because the
report is how anyone will know the port actually helped.

**What my machine changed in the picture** — and what yours may:
nothing here (the rig is the rig); on a desktop expect two structural
shifts the lesson named: the `present` row's CPU/wait split flips with
a local display, and a sound device paces the loop through the feed
instead of the jiggle — so your frame *rate* becomes real before your
frame *cost* is even interesting. A machine on an OS the seam does not
answer yet flips the order outright: the module is then not an
optimization but the door.

Whichever piece your machine names first, one demand stays fixed
(D12): the map's numbers carry your machine's name, and the piece is
built when measurement asks for it — never because the map exists.
