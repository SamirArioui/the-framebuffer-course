#!/usr/bin/env bash
#
# frame-card.sh — the 60 fps hardware card, as one command.
#
# The MVD's perf line says the game sustains 60 fps on modest hardware.
# The authoring machine cannot demonstrate that (design D12); this script
# turns any real machine into the instrument that can. It builds the
# finished game with lesson 101 exercise 1's percentiles instrument
# applied (book/solutions/lesson-101/ex1.patch), runs it on the machine
# you are sitting at, and writes the card: the machine named, the run's
# shape named, the report verbatim, and the sentence the checklist needs.
#
# Usage:
#   ./tools/frame-card.sh [--optimize] [--auto SECONDS] [--machine NAME] [--out DIR]
#
#   --optimize      build with -O2 instead of the course's -O0 -g
#                   (run the script twice to carry both rows)
#   --auto SECONDS  drive the run with xdotool for SECONDS instead of a
#                   hand-played one (reproducible; a jiggle-paced run)
#   --machine NAME  the machine's name for the report's machine line
#                   (default: its CPU, cores, and RAM)
#   --out DIR       where the card and the run's log land (default: here)
#
# Everything happens in a throwaway export of the committed tree — your
# working tree is never touched. Play hard: walk, fire, die, restart —
# then close the window; the report prints on close and the card is
# written. The card is honest about its run's shape: a window on a real
# display, a sound device's pacing or the jiggles', played by hand or
# scripted — all of it named in the card.

set -euo pipefail

cd "$(dirname "$0")/.."

OPTIMIZE=0
AUTO_SECONDS=0
MACHINE=""
OUT="."

while [ $# -gt 0 ]; do
    case "$1" in
        --optimize) OPTIMIZE=1 ;;
        --auto)     AUTO_SECONDS="${2:?--auto needs a number of seconds}"; shift ;;
        --machine)  MACHINE="${2:?--machine needs a name}"; shift ;;
        --out)      OUT="${2:?--out needs a directory}"; shift ;;
        *) echo "frame-card: unknown argument: $1" >&2; exit 2 ;;
    esac
    shift
done

# --- the machine's facts (the card's first duty: a number's machine) ---

cpu_model="$(grep -m1 'model name' /proc/cpuinfo 2>/dev/null | cut -d: -f2- | sed 's/^ *//')"
[ -n "$cpu_model" ] || cpu_model="$(uname -m)"
cpu_cores="$(nproc)"
ram_kb="$(awk '/MemTotal/ {print $2}' /proc/meminfo)"
ram_gb=$(( (ram_kb + 512 * 1024) / (1024 * 1024) ))
os_name="$(. /etc/os-release 2>/dev/null && echo "$PRETTY_NAME" || uname -s)"
kernel="$(uname -r)"

if [ -z "$MACHINE" ]; then
    MACHINE="$cpu_model, $cpu_cores cores, ${ram_gb} GB RAM"
fi
# The machine line travels through sed's replacement text; keep it tame.
MACHINE="$(printf '%s' "$MACHINE" | tr -d '"\\|&')"

if [ -n "${DISPLAY:-}" ]; then
    display="X11 display $DISPLAY"
elif [ -n "${WAYLAND_DISPLAY:-}" ]; then
    display="Wayland ($WAYLAND_DISPLAY)"
else
    echo "frame-card: no display — the game needs a window to render into" >&2
    exit 1
fi

snd_entries="$(ls /dev/snd 2>/dev/null | grep -v -x -e timer -e seq || true)"
if [ -n "$snd_entries" ]; then
    sound="present (the audio horizon paces the loop at ~60 feeds a second)"
    pacing="the sound device's feeding schedule"
else
    sound="none (the game reports it and continues without)"
    pacing="input events"
fi

commit="$(git rev-parse --short HEAD)"
state_note="not the lesson-103 state — say why in the card"
git diff --quiet lesson-103 -- src/ assets/ 2>/dev/null && state_note="src/ and assets/ equal to the lesson-103 tag"

cc_name="$("${CC:-gcc}" --version 2>/dev/null | head -1)"
if [ "$OPTIMIZE" = 1 ]; then
    flags="-std=c11 -O2 -g -Wall -Wextra / -std=c++17 -O2 -g -Wall -Wextra"
else
    flags="the course's own: -O0 -g -Wall -Wextra"
fi

if [ "$AUTO_SECONDS" -gt 0 ] && ! command -v xdotool >/dev/null; then
    echo "frame-card: --auto needs xdotool" >&2
    exit 2
fi

# --- the instrument: a throwaway tree, never your own ---

tmp="$(mktemp -d "${TMPDIR:-/tmp}/frame-card.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT

git archive HEAD | tar -x -C "$tmp"
(cd "$tmp" && git apply book/solutions/lesson-101/ex1.patch)

# The machine line is yours to edit (lesson 101, exercise 1) — edited
# here, in the throwaway tree, so the card's numbers and their machine
# travel together.
old_machine="WSL2, Xvfb :99, no sound hardware (the course's authoring machine)"
sed -i "s|\"$old_machine\"|\"$MACHINE\"|" "$tmp/src/main.cpp"
if ! grep -qF "\"$MACHINE\"" "$tmp/src/main.cpp"; then
    echo "frame-card: could not set the machine line in src/main.cpp" >&2
    exit 1
fi

echo "frame-card: building ($flags) in $tmp"
if [ "$OPTIMIZE" = 1 ]; then
    (cd "$tmp" && CFLAGS="-std=c11 -O2 -g -Wall -Wextra" \
                 CXXFLAGS="-std=c++17 -O2 -g -Wall -Wextra" ./build.sh)
else
    (cd "$tmp" && ./build.sh)
fi

mkdir -p "$OUT"
LOG="$OUT/frame-log.txt"

# --- the run: played hard, or scripted ---

if [ "$AUTO_SECONDS" -gt 0 ]; then
    played="scripted (xdotool, $AUTO_SECONDS s)"
    [ -n "$snd_entries" ] || pacing="window-move jiggles (the loop is event-driven)"
    echo "frame-card: running scripted for $AUTO_SECONDS s — no play needed"
    (cd "$tmp" && ./build/game > "$LOG" 2>&1) &
    game_pid=$!
    sleep 1
    win="$(xdotool search --name 'the framebuffer engine' | tail -1 || true)"
    if [ -z "$win" ]; then
        echo "frame-card: the game's window did not appear" >&2
        wait "$game_pid" || true
        exit 1
    fi
    xdotool windowfocus "$win" 2>/dev/null || true
    x=$(( $(xdotool getdisplaygeometry | cut -d' ' -f1) / 4 ))
    y=$(( $(xdotool getdisplaygeometry | cut -d' ' -f2) / 4 ))
    end=$(( $(date +%s) + AUTO_SECONDS ))
    held=""
    tick=0
    while [ "$(date +%s)" -lt "$end" ]; do
        x=$(( x + 40 )); y=$(( y + 10 ))
        xdotool windowmove "$win" "$(( x % 300 ))" "$(( y % 200 ))"
        # The game polls key state, so walk on held keys (XTEST input),
        # change direction about a second, and fire often. Enter
        # restarts after a death; the screens' frames count too.
        tick=$(( tick + 1 ))
        if [ $(( tick % 25 )) = 1 ]; then
            [ -n "$held" ] && xdotool keyup "$held"
            case $(( (tick / 25) % 4 )) in
                0) held=Right ;; 1) held=Down ;;
                2) held=Left  ;; 3) held=Up   ;;
            esac
            xdotool keydown "$held"
        fi
        [ $(( tick % 7 )) = 0 ] && xdotool key space
        [ $(( tick % 60 )) = 0 ] && xdotool key Return
        sleep 0.04
    done
    [ -n "$held" ] && xdotool keyup "$held"
    # Close the window the way a window manager would (WM_DELETE_WINDOW):
    # the report prints on the close conversation, so a kill loses it.
    xdotool windowclose "$win" 2>/dev/null || \
        python3 - "$win" <<'PYEOF'
import ctypes, sys
xlib = ctypes.CDLL("libX11.so.6")
d = xlib.XOpenDisplay(None)
w = int(sys.argv[1])
wm_protocols = xlib.XInternAtom(d, b"WM_PROTOCOLS", 0)
wm_delete = xlib.XInternAtom(d, b"WM_DELETE_WINDOW", 0)
class XClientMessageEvent(ctypes.Structure):
    _fields_ = [("type", ctypes.c_int), ("serial", ctypes.c_ulong),
                ("send_event", ctypes.c_int), ("display", ctypes.c_void_p),
                ("window", ctypes.c_ulong), ("message_type", ctypes.c_ulong),
                ("format", ctypes.c_int), ("data", ctypes.c_long * 5)]
e = XClientMessageEvent(33, 0, 0, d, w, wm_protocols, 32, (wm_delete, 0, 0, 0, 0))
xlib.XSendEvent(d, w, 0, 0, ctypes.byref(e))
xlib.XFlush(d)
PYEOF
    wait "$game_pid" || true
else
    played="by hand (played hard: walking, firing, dying, restarting)"
    echo "frame-card: the game is starting — play hard, then close the window."
    echo "frame-card: the report prints on close; the card is written after."
    (cd "$tmp" && ./build/game > "$LOG" 2>&1) || true
fi

# --- the card ---

report="$(awk '/^engine: frame budget/{f=1} f{print} f && /^engine:   machine/{exit}' "$LOG")"
if [ -z "$report" ]; then
    echo "frame-card: no frame-budget report in the run's log — did the game close cleanly?" >&2
    echo "frame-card: the run's log is at $LOG" >&2
    exit 1
fi

budget_line="$(printf '%s\n' "$report" | grep '^engine:   budget')"
over="$(printf '%s\n' "$budget_line" | sed -E 's/.*— ([0-9]+) of ([0-9]+) frames over it.*/\1/')"
total="$(printf '%s\n' "$budget_line" | sed -E 's/.*— ([0-9]+) of ([0-9]+) frames over it.*/\2/')"
p99="$(printf '%s\n' "$report" | sed -nE 's/.*p99 ([0-9.]+) ms.*/\1/p')"
if [ "$over" = 0 ] && [ -n "$p99" ]; then
    verdict="**The game holds 60 fps on this machine** — \`0 of $total\` frames over the \`16.667 ms\` budget, p99 \`$p99 ms\`."
else
    verdict="**The game does not hold 60 fps on this machine** — \`$over of $total\` frames over the \`16.667 ms\` budget, p99 \`${p99:-unrecorded} ms\`."
fi

CARD="$OUT/frame-card.md"
cat > "$CARD" <<EOF
# 60 fps hardware card — $MACHINE

The MVD's perf line, answered on real hardware. Every number below is
measured on the machine named here, from a run played on it — nothing
carried from any other machine.

## The machine

| | |
| - | - |
| CPU | $cpu_model |
| Cores | $cpu_cores |
| RAM | ${ram_gb} GB |
| OS | $os_name (kernel $kernel) |
| Display | $display |
| Sound | $sound |

## The build

| | |
| - | - |
| Course state | \`$commit\` ($state_note) |
| Compiler | $cc_name |
| Flags | $flags |
| Instrument | \`book/solutions/lesson-101/ex1.patch\` (the percentiles histogram) |

## The run's shape

| | |
| - | - |
| Played | $played |
| Pacing | $pacing |
| Frames | $total |

## The report

\`\`\`
$report
\`\`\`

## The sentence

$verdict

*(Card produced by \`tools/frame-card.sh\` on $(date -u +%Y-%m-%d). The
run's whole log: \`frame-log.txt\`, beside this card.)*
EOF

echo "frame-card: card written to $CARD (log: $LOG)"
printf '%s\n' "$verdict"
