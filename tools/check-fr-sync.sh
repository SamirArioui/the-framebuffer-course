#!/usr/bin/env bash
# check-fr-sync.sh — the translation-fidelity checker (design D6).
#
# Verifies, for every French page under book-fr/src/, that it stays answerable
# to its English source:
#   (a) marker      the page carries a parseable
#                   `<!-- translation-source: <path> @ <revision> -->` naming
#                   its English counterpart at the mirrored path;
#   (b) code        every fenced code block matches the English page's
#                   corresponding block byte-for-byte and in order
#                   ({{#include}} directives are expanded first, so a French
#                   solution page that includes the English patch is equal to
#                   the English page that includes the same patch);
#   (c) drift       the English page's current revision equals the declared
#                   one — a page whose English source moved is reported as
#                   behind, naming both revisions. Drift is read from git
#                   history, so this check needs a full clone: in a shallow
#                   checkout it cannot tell, and says so instead of guessing;
#   (d) horizon     book/stability-horizon.md and book-fr/src/stability-horizon.md
#                   name the same frozen range.
#
# Exit 0 when everything holds; 1 with one line per finding naming the page.
# Authoring tooling — bash and awk, nothing else.

set -u

repo_root=$(cd "$(dirname "$0")/.." && pwd)
cd "$repo_root" || exit 1

fr_root="book-fr/src"
fail=0
checked=0
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

# Drift (c) is read from git history. In a shallow checkout the history is
# truncated — `git log -1 -- <path>` would name the tip commit for every file
# and make every translation look stale. Say "cannot tell" instead of lying.
shallow=0
if [ "$(git rev-parse --is-shallow-repository 2>/dev/null)" = "true" ]; then
    shallow=1
fi

report() { printf 'FAIL %s\n' "$*"; fail=1; }

# extract_blocks <file> <out> — write the file's fenced code blocks to <out>,
# {{#include}} directives expanded to their target's content, each block
# introduced by a `### block N` line.
extract_blocks() {
    awk -v file="$1" -v root="$(dirname "$1")" '
        function dirname_of(p) { sub(/\/[^\/]*$/, "", p); return p }
        function handle(line) {
            if (line ~ /^```/) {
                if (inblock) { print; inblock = 0 }
                else { n++; print "### block " n; print; inblock = 1 }
            } else if (inblock) print line
        }
        function process(path, dir, depth,   line, target) {
            if (depth > 5) return
            while ((getline line < path) > 0) {
                if (line ~ /^[[:space:]]*\{\{#include[[:space:]]+[^}]+\}\}[[:space:]]*$/) {
                    target = line
                    sub(/.*\{\{#include[[:space:]]+/, "", target)
                    sub(/\}\}.*/, "", target)
                    process(dir "/" target, dirname_of(dir "/" target), depth + 1)
                } else handle(line)
            }
            close(path)
        }
        BEGIN { inblock = 0; n = 0; process(file, root, 0) }
    ' "$1" > "$2"
}

# frozen_range <horizon-file> — print `first:last` of the numbered lesson tags
# the banner names.
frozen_range() {
    grep -o 'lesson-[0-9][0-9][0-9]' "$1" | sed -n '1p;$p' | paste -sd: -
}

# --- (d) the two horizons name the same frozen range -----------------------
en_horizon="book/stability-horizon.md"
fr_horizon="$fr_root/stability-horizon.md"
if [ -f "$en_horizon" ] && [ -f "$fr_horizon" ]; then
    en_range=$(frozen_range "$en_horizon")
    fr_range=$(frozen_range "$fr_horizon")
    if [ "$en_range" != "$fr_range" ]; then
        report "$fr_horizon: frozen range '$fr_range' does not match $en_horizon's '$en_range'"
    fi
else
    report "missing horizon file ($en_horizon or $fr_horizon)"
fi

# --- (a)(b)(c) every French page ------------------------------------------
while IFS= read -r fr_file; do
    rel=${fr_file#"$fr_root"/}
    en_file="book/$rel"
    checked=$((checked + 1))

    # (a) the marker exists, parses, and names the mirrored English page
    marker=$(sed -n 's|^<!-- translation-source: \(.*\) @ \([0-9a-fA-F]\{7,40\}\) -->$|\1 \2|p' "$fr_file")
    if [ -z "$marker" ]; then
        report "$fr_file: no parseable '<!-- translation-source: <path> @ <revision> -->' marker"
        continue
    fi
    en_path=${marker% *}
    en_rev=${marker##* }
    if [ ! -f "$en_path" ]; then
        report "$fr_file: marker names '$en_path', which does not exist"
        continue
    fi
    if [ "$en_path" != "$en_file" ]; then
        report "$fr_file: marker names '$en_path' but the mirrored English page is '$en_file'"
    fi

    # (c) drift — the English page stands where the translation declares
    if [ "$shallow" -eq 0 ]; then
        cur_rev=$(git log -1 --format=%h -- "$en_path")
        if [ "$cur_rev" != "$en_rev" ]; then
            report "$fr_file: behind — declares '$en_rev' but $en_path now stands at '$cur_rev'; retranslate and redeclare (plan/translation-conventions.md §5)"
        fi
    fi

    # (b) code blocks match, byte for byte, in order
    extract_blocks "$en_file" "$tmpdir/en.blocks"
    extract_blocks "$fr_file" "$tmpdir/fr.blocks"
    if ! diff_out=$(diff "$tmpdir/en.blocks" "$tmpdir/fr.blocks"); then
        first=$(printf '%s\n' "$diff_out" | head -1 | sed 's/^\([0-9]*\).*/\1/')
        block=$(head -n "$first" "$tmpdir/fr.blocks" | grep -c '^### block ')
        report "$fr_file: code block $block differs from $en_file's:"
        printf '%s\n' "$diff_out" | sed 's/^/      /'
    fi
done < <(find "$fr_root" -name '*.md' | sort)

if [ "$checked" -eq 0 ]; then
    report "no French pages found under $fr_root/"
fi

if [ "$shallow" -eq 1 ]; then
    report "drift not assessed: shallow checkout, git history truncated — markers and code blocks above are still checked; re-run from a full clone (git fetch --unshallow)"
fi

if [ "$fail" -eq 0 ]; then
    printf 'OK %s French page(s) in sync with their English sources\n' "$checked"
fi
exit "$fail"
