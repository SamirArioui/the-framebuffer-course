#!/usr/bin/env bash
#
# disasm.sh — print what the compiler made of one function.
#
# Lesson 048: the assembly-reading tool. build.sh compiles the engine; this
# prints the instructions that ended up in build/game for one symbol —
# exactly the compiler's output, nothing added. The default symbol is the
# blitter, whose copy loop lessons 048-049 read.
#
# Usage:
#   ./tools/disasm.sh                 # the blitter (BlitSprite)
#   ./tools/disasm.sh ClearBuffer     # any other symbol by substring
#   ./tools/disasm.sh --census        # vector-register instructions per function
#
# build/ must be current: run ./build.sh first. To read the optimized
# build's instructions (lesson 049), build with the flags first:
#   CXXFLAGS="-std=c++17 -O3 -g -Wall -Wextra" ./build.sh

set -euo pipefail

cd "$(dirname "$0")/.."

SYMBOL="${1:-BlitSprite}"

if [ ! -x build/game ]; then
    echo "disasm: build/game is missing — run ./build.sh first" >&2
    exit 1
fi

# Only definition lines start at column 0 with an address and end in ':' —
# call sites name the symbol too, and those are not what we are reading.
listing() {
    objdump -d -C --no-show-raw-insn build/game |
        sed -n "/^[0-9a-f]* <.*${1}.*>:/,/^\$/p"
}

if [ "$SYMBOL" = "--census" ]; then
    # The SIMD lens: how many vector-register instructions (xmm/ymm/zmm)
    # each function's compiled code actually uses.
    objdump -d -C --no-show-raw-insn build/game |
        awk '/^[0-9a-f]+ <.*>:$/ { name = $0
                                  sub(/^[0-9a-f]+ </, "", name)
                                  sub(/>:$/, "", name)
                                  next }
             /%[xyz]mm/ { count[name]++ }
             END { for (n in count) printf "%5d  %s\n", count[n], n }' |
        sort -rn
    exit 0
fi

listing "$SYMBOL"
