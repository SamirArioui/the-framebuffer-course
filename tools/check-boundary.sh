#!/usr/bin/env bash
#
# check-boundary.sh — the single-OS-boundary check.
#
# Lesson 042: the seam's contract, audited mechanically. Engine code may
# include the platform interface and the language's own headers — nothing
# that knows which OS it is on. The platform implementation files are the
# only place OS headers and OS calls may appear; a second OS replaces those
# files and nothing else.
#
# Usage:
#   ./tools/check-boundary.sh
#
# Exits 0 when the boundary holds, 1 when it is breached.

set -euo pipefail

cd "$(dirname "$0")/.."

# The files a second OS replaces. Everything else is engine code.
IMPL="src/platform_x11.cpp"

# Headers that only an OS has. The language's own headers (<cstdio>,
# <cstring>, <stddef.h>, ...) are fine anywhere — they are not an OS.
OS_HEADERS='<X11/|<sys/|<unistd\.h>|<fcntl\.h>|<poll\.h>|<signal\.h>|<errno\.h>|<time\.h>'

# Calls only an OS answers. The list grows with the seam.
OS_CALLS='(^|[^A-Za-z0-9_:])(X[A-Z][A-Za-z]+|mmap|munmap|mprotect|clock_gettime|nanosleep|sysconf|open|close|read|write|fstat|poll|signal)\s*\('

status=0

echo "boundary: engine code must not name an OS"
for f in src/*.h src/*.cpp; do
    [ "$f" = "$IMPL" ] && continue
    if hits=$(grep -nE "$OS_HEADERS" "$f"); then
        echo "BREACH: $f includes an OS header:"
        echo "$hits"
        status=1
    fi
    if hits=$(grep -nE "$OS_CALLS" "$f"); then
        echo "BREACH: $f calls an OS function:"
        echo "$hits"
        status=1
    fi
done

if [ "$status" -eq 0 ]; then
    echo "boundary: OK — OS headers and OS calls appear only in $IMPL"
fi

echo "boundary: the contract a second OS implements is declared in src/platform.h"
echo "boundary: $(grep -cE '^[A-Za-z].*\(.*\);' src/platform.h) single-line declarations there (multi-line ones are in the header)"

exit "$status"
