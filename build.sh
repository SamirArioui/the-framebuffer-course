#!/usr/bin/env bash
#
# build.sh — the one-command build.
#
# Compiles every C/C++ source under src/ and links the result into build/.
# This script is the build; Make/CMake are never curriculum (design D10).
# Compiler flags, the debugger, sanitizers, and the profiler are taught
# explicitly in Parts 0-2 — for this audience the toolchain is curriculum.
#
# Usage:
#   ./build.sh
#
# Environment overrides:
#   CC, CFLAGS       compiler and flags for C sources
#   CXX, CXXFLAGS    compiler and flags for C++ sources
#   LDFLAGS          extra link flags
#   BUILD_DIR        output directory (default: build)

set -euo pipefail

cd "$(dirname "$0")"

CC="${CC:-gcc}"
CXX="${CXX:-g++}"
CFLAGS="${CFLAGS:--std=c11 -O0 -g -Wall -Wextra}"
CXXFLAGS="${CXXFLAGS:--std=c++17 -O0 -g -Wall -Wextra}"
LDFLAGS="${LDFLAGS:-}"
BUILD_DIR="${BUILD_DIR:-build}"
OBJ_DIR="$BUILD_DIR/obj"
BIN="$BUILD_DIR/game"

objects=()
compiled=0

compile_source() {
    local src="$1" rel="${1#src/}" obj compiler flags
    case "$src" in
        *.c)                  compiler="$CC";  flags="$CFLAGS" ;;
        *.cpp | *.cc | *.cxx) compiler="$CXX"; flags="$CXXFLAGS" ;;
    esac
    obj="$OBJ_DIR/${rel%.*}.o"
    mkdir -p "$(dirname "$obj")"
    echo "  CC  $src"
    # Flags are word-split on purpose: one string of flags per compiler.
    # shellcheck disable=SC2086
    "$compiler" $flags -c "$src" -o "$obj"
    objects+=("$obj")
    compiled=$((compiled + 1))
}

mapfile -d '' -t sources < <(
    find src -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.cc' -o -name '*.cxx' \) -print0 | sort -z
)

if [ "${#sources[@]}" -eq 0 ]; then
    echo "build: no C/C++ sources under src/ — nothing to compile."
    echo "build: OK (0 sources compiled)"
    exit 0
fi

echo "build: compiling ${#sources[@]} source(s) from src/"
for src in "${sources[@]}"; do
    compile_source "$src"
done

mkdir -p "$BUILD_DIR"
echo "  LD  $BIN"
# shellcheck disable=SC2086
"$CXX" "${objects[@]}" $LDFLAGS -o "$BIN"

echo "build: OK ($compiled source(s) compiled -> $BIN)"
