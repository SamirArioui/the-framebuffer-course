# Lesson 048 — the blitter's compiled assembly

{{#include ../../stability-horizon.md}}

## Prose

You wrote `BlitSprite`. This lesson reads **what the compiler made of
it** — the instructions that actually run when a sprite is drawn. This is
the assembly-reading deep dive, and its discipline is the same as every
reading in this course: the text in front of you is *real output* of the
build you can run, quoted line for line. You are not learning to write
assembly (that skill is not on this course's road); you are learning to
*read* the machine's answer to your code — because the answers to "why is
this slow" and "what did the compiler do with my loop" are written in
exactly this language. Lesson 049 reads the optimized answer; today's is
the faithful one.

### The tool

The code step is `tools/disasm.sh` — one command that prints the
instructions for one function out of `build/game`:

```
$ ./tools/disasm.sh              # the blitter
$ ./tools/disasm.sh ClearBuffer  # anything else, by substring
```

It wraps `objdump -d -C --no-show-raw-insn`, narrowed to the function's
definition (call sites name functions too; those are not what we are
reading), and it prints nothing the compiler did not emit. What follows is
its output for `BlitSprite`, built with the course's default
`./build.sh` — `-O0`, the build every lesson so far has run.

### The prologue: how a call arrives

```
0000000000001764 <engine::BlitSprite(engine::Framebuffer&, engine::Sprite const&, int, int)>:
    1764:	endbr64
    1768:	push   %rbp
    1769:	mov    %rsp,%rbp
    176c:	mov    %rdi,-0x38(%rbp)
    1770:	mov    %rsi,-0x40(%rbp)
    1774:	mov    %edx,-0x44(%rbp)
    1777:	mov    %ecx,-0x48(%rbp)
```

Seven instructions and the whole calling convention is on the table:

- **`endbr64`** is a landing pad: modern x86-64 marks valid jump targets
  so the CPU can refuse returns and jumps to nowhere (control-flow
  enforcement — the hardware's defense, appearing in every function).
- **`push %rbp; mov %rsp,%rbp`** is the frame pointer: the function keeps
  a fixed anchor for its local variables. You can ask a debugger to walk
  frames precisely because of these two instructions.
- **The four `mov`s are the arguments coming home.** The x86-64
  convention passes the first six integer or pointer arguments in
  registers — here `%rdi` (the `Framebuffer&`), `%rsi` (the `Sprite&`),
  `%edx` (x), `%ecx` (y) — and at `-O0` the compiler immediately spills
  each one to its stack slot. Every local in this function lives at
  `-0xNN(%rbp)`: that is what `-O0` *means*. No variable has been
  assigned a register; the stack is the home of everything.

### The clip rectangle, compiled

```
    177a:	mov    -0x44(%rbp),%eax
    177d:	mov    $0x0,%edx
    1782:	test   %eax,%eax
    1784:	cmovs  %edx,%eax
    1787:	mov    %eax,-0x20(%rbp)
```

That is `int left = x < 0 ? 0 : x;` — read it as: load x, load zero, test
x's sign, **conditionally move** zero over x if it was negative. `cmovs`
("conditional move if sign") is a comparison and an assignment with *no
branch* — the CPU executes both paths' data and picks one. The same shape
compiles `top`; and the other two clamps:

```
    179a:	mov    -0x38(%rbp),%rax
    179e:	mov    0x8(%rax),%edx
    17a1:	mov    -0x40(%rbp),%rax
    17a5:	mov    0x8(%rax),%ecx
    17a8:	mov    -0x44(%rbp),%eax
    17ab:	add    %ecx,%eax
    17ad:	cmp    %eax,%edx
    17af:	cmovle %edx,%eax
    17b2:	mov    %eax,-0x18(%rbp)
```

`right = x + s.width < fb.width ? x + s.width : fb.width`, in one run.
Two details worth pausing on: the field offsets — `0x8(%rax)` is
`width`, `0xc(%rax)` is `height`, *the struct layout of lesson 007 made
visible* (a `Framebuffer` is `pixels` at 0, `width` at 8, `height` at 12);
and the comparison is `cmovle`, matching the C++ `<` reversed — the
compiler reads `a < b` as `b <= a`-style and picks the move accordingly.
The condition is the same; the shape is the compiler's.

### Loops are branches with bookkeeping

```
    17d0:	mov    -0x1c(%rbp),%eax
    17d3:	mov    %eax,-0x28(%rbp)
    17d6:	jmp    18e5 <engine::BlitSprite(...)+0x181>
    17db:	mov    -0x20(%rbp),%eax
    17de:	mov    %eax,-0x24(%rbp)
    17e1:	jmp    18d5 <engine::BlitSprite(...)+0x171>
```

`j = top; goto the_test; i = left; goto its_test`. The C++ `for` loops
compiled into the classic shape: initialize, jump to the *bottom*, where
the test lives, so the loop runs its check once per iteration and not
twice. `-0x28(%rbp)` is `j`; `-0x24(%rbp)` is `i`. From here on, "the
loop" is these two slots being incremented and compared against the clip
rectangle's bottom and right.

### One pixel: the source address and the key check

```
    17e6:	mov    -0x40(%rbp),%rax
    17ea:	mov    (%rax),%rcx
    17ed:	mov    -0x28(%rbp),%eax
    17f0:	sub    -0x48(%rbp),%eax
    17f3:	movslq %eax,%rdx
    17f6:	mov    -0x40(%rbp),%rax
    17fa:	mov    0x8(%rax),%eax
    17fd:	cltq
    17ff:	imul   %rax,%rdx
    1803:	mov    -0x24(%rbp),%eax
    1806:	sub    -0x44(%rbp),%eax
    1809:	cltq
    180b:	add    %rax,%rdx
    180e:	mov    %rdx,%rax
    1811:	add    %rax,%rax
    1814:	add    %rdx,%rax
    1817:	add    %rcx,%rax
    181a:	mov    %rax,-0x10(%rbp)
```

Eighteen instructions for
`&s.pixels[(((j - y) * s.width) + (i - x)) * 3]`. The arithmetic is all
here: `sub` computes `j − y`; `movslq`/`cltq` **sign-extend** the 32-bit
int into a 64-bit index (the lesson 007-sized truth: `int` math widened
where pointers live); `imul` does the row stride; `add %rax,%rax; add
%rdx,%rax` is `× 3` — multiplication by a small constant compiled into
shift-and-add. The result lands in `-0x10(%rbp)`: `src`, the pointer.

Then the key check — the transparency rule, in three compares:

```
    181e:	mov    -0x10(%rbp),%rax
    1822:	movzbl (%rax),%edx
    1825:	mov    -0x40(%rbp),%rax
    1829:	movzbl 0x10(%rax),%eax
    182d:	cmp    %al,%dl
    182f:	jne    185f <engine::BlitSprite(...)+0xfb>
```

`movzbl` is "move byte, zero-extend to long" — loading one unsigned byte
into a full register so the compare is clean. `0x10(%rax)` is `key_r`:
the `Sprite` struct's fields at offsets 0 (pixels), 8 (width), 12
(height), 16-18 (the key). The `jne 185f` jumps *past the copy* at the
first difference — and the second and third compares (key_g, key_b) do
the same. Only if all three match does the flow reach `je 18d0` — the
skip: a single `nop`, the `continue` of the C++.

### The copy itself

```
    185f:	mov    -0x38(%rbp),%rax
    1863:	mov    (%rax),%rcx
    ...
    1881:	shl    $0x2,%rax
    1885:	add    %rcx,%rax
    1888:	mov    %rax,-0x8(%rbp)
```

The destination address: the same index math in framebuffer units, and
`shl $0x2,%rax` is `× 4` — four bytes per pixel, the shift replacing the
multiply. Then the four stores, **in the order the C++ wrote them**:

```
    188c:	mov    -0x10(%rbp),%rax
    1890:	add    $0x2,%rax
    1894:	movzbl (%rax),%edx
    1897:	mov    -0x8(%rbp),%rax
    189b:	mov    %dl,(%rax)
```

`dst[0] = src[2];` — load the source's *third* byte (red), store it at
the destination's first (blue). The RGB→BGR swap is right there: `add
$0x2` picks the byte, the store places it. The next pairs do
`dst[1] = src[1]` and `dst[2] = src[0]`, and finally `movb $0x0,(%rax)`
is `dst[3] = 0` — a store of a literal zero, one byte wide.

The loop closes with four instructions you can now read cold:

```
    18d1:	addl   $0x1,-0x24(%rbp)
    18d5:	mov    -0x24(%rbp),%eax
    18d8:	cmp    -0x18(%rbp),%eax
    18db:	jl     17e6 <engine::BlitSprite(...)+0x82>
```

`++i`, load it, compare against `right`, jump back if less. The outer
loop's version is the same against `bottom`. And the function ends as it
began: `nop; nop; pop %rbp; ret` — the frame taken apart, the return.

### What the reading tells you

Count the inner loop on the copy path — the address math (18
instructions), the key check (20), the copy (34), the increment and test
(4) — and one opaque pixel costs **76 instructions**; a key-colored pixel
about 43 (the address and key check, then the `nop` of the skip). That is
the `-O0` price of "a variable per stack slot and a statement per
instruction run".

The measurement says what that costs: drawing one 16×16 sprite — 130
opaque pixels and 126 key-colored ones — takes about 882 ns, which is
**3.45 ns per sprite pixel** averaged over that mix. At this machine's
3.26 GHz that is about 11 cycles per pixel against ~76 instructions. The
gap is the modern out-of-order core retiring several simple instructions
per cycle; the *ratio* is what `-O0` loses and `-O3` recovers. Lesson
047's flat 1.5 GB/s table row is this loop from the other side.

And the honest boundary of this reading: the C++ standard promises only
**observable behavior** — the compiler may compute anything, in any order,
by any instructions, as long as the pixels and the failures come out
right. What you just read is one compiler's *current* answer at `-O0`,
not a promise about tomorrow's, and not the only answer. The next lesson
reads a second answer to the same C++ — the vectorized one — and both are
true at once. Part 5's profiler pass ends up here again: when the blit
shows up in the top-2 hotspots, this listing is where the fix gets read
before it gets written.

## Code step

One change for this lesson: `tools/disasm.sh` — the assembly-reading tool
that prints one function's compiled instructions out of `build/game`
(`objdump -d -C --no-show-raw-insn`, narrowed to the function's
definition). The engine's code is untouched. The tool's end state is
tagged `lesson-048`.

```diff
diff --git a/tools/disasm.sh b/tools/disasm.sh
new file mode 100755
index 0000000..f54a09c
--- /dev/null
+++ b/tools/disasm.sh
@@ -0,0 +1,32 @@
+#!/usr/bin/env bash
+#
+# disasm.sh — print what the compiler made of one function.
+#
+# Lesson 048: the assembly-reading tool. build.sh compiles the engine; this
+# prints the instructions that ended up in build/game for one symbol —
+# exactly the compiler's output, nothing added. The default symbol is the
+# blitter, whose copy loop lessons 048-049 read.
+#
+# Usage:
+#   ./tools/disasm.sh                 # the blitter (BlitSprite)
+#   ./tools/disasm.sh ClearBuffer     # any other symbol by substring
+#
+# build/ must be current: run ./build.sh first. To read the optimized
+# build's instructions (lesson 049), build with the flags first:
+#   CXXFLAGS="-std=c++17 -O3 -g -Wall -Wextra" ./build.sh
+
+set -euo pipefail
+
+cd "$(dirname "$0")/.."
+
+SYMBOL="${1:-BlitSprite}"
+
+if [ ! -x build/game ]; then
+    echo "disasm: build/game is missing — run ./build.sh first" >&2
+    exit 1
+fi
+
+# Only definition lines start at column 0 with an address and end in ':' —
+# call sites name the symbol too, and those are not what we are reading.
+objdump -d -C --no-show-raw-insn build/game |
+    sed -n "/^[0-9a-f]* <.*${SYMBOL}.*>:/,/^\$/p"
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The instruction budget *(predict-the-output)*

The listing is a bill of materials; price it. Count the instructions on
the copy path of the inner loop yourself (the source address, the key
check, the copy, the increment — the lesson gives the sections, you give
the totals), then predict the nanoseconds one 16×16 sprite should cost at
your CPU's clock, assuming nothing about parallelism. Add the lesson's
benchmark to the startup block — draw the sprite ten thousand times,
timed, printed as nanoseconds per pixel — and compare your prediction
with the measurement. Where does the difference live, and what does it say
about counting instructions on a modern core?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-048/ex1.md)

### Exercise 2 — The compiler's signature *(port-to-your-own-machine)*

This listing is one compiler's opinion — the book's column is GCC 13.3.0.
Make your build record its opinion too: print `__VERSION__` (the
compiler's own predefined macro) in the run's startup report, so every
number your machine produces carries the compiler that produced it. Then
run `./tools/disasm.sh` against your build — same C++, your compiler —
and compare three landmarks with the book's listing: the function
prologue (the argument spills), the clip clamps (conditional moves or
branches?), and the four stores of the copy. What stayed, what moved, and
which of the two lists is the language and which is the compiler?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-048/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 047 — the caches deep dive](lesson-047-caches.md) ·
**Next:** [Lesson 049 — the SIMD lens](lesson-049-simd.md) ·
**Code tag:** [`lesson-048`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-048)
