# Lesson 044 — a sprite as loaded bytes

{{#include ../../stability-horizon.md}}

## Prose

Part 1 ended with every pixel on the screen written by code we own — and
every pixel a flat color. Part 2 starts where the pixels will come from: a
**sprite is a file's bytes in the engine's memory**. Before the renderer
copies anything anywhere, the bytes have to arrive — from disk, through the
seam's whole-file read of lesson 037, into the arena of lesson 041, as a
format small enough to define by hand and read by eye. Today defines the
format and gets the bytes in. That is the whole lesson: no drawing yet, and
that is the point — a renderer that trusts its assets is a renderer that
draws garbage honestly.

### The format, defined by hand

The format is **PPM, the P6 variant** — the same one Part 0's lesson 017
wrote from scratch. Its complete definition:

```
P6\n
<width> <height>\n
<maxval>\n
<width × height × 3 bytes of pixels>
```

Three lines of ASCII, then raw bytes. `P6` says "binary pixels follow";
`<width> <height>` are decimal numbers; `<maxval>` is `255` here, meaning
one byte per color channel. Then come every pixel's three bytes — red,
green, blue — in row order, top row first. No compression, no palette, no
lengths to get wrong. This is our sprite file, `assets/sprite.ppm`, a 16×16
image with a magenta key color:

```
$ xxd assets/sprite.ppm | head -2
00000000: 5036 0a31 3620 3136 0a32 3535 0aff 00ff  P6.16 16.255....
00000010: ff00 ffff 00ff ff00 ffff 00ff ff00 ffff  ................
```

Read it in bytes, Part 0 style: `50 36` is `P6`; `0a` is a newline;
`31 36 20 31 36` is the text `16 16`; `0a`; `32 35 35` is `255`; `0a` —
and that is the whole header, **thirteen bytes**. Then the pixels begin:
`ff 00 ff` is pixel `(0,0)`, magenta; the next `ff 00 ff` is pixel `(1,0)`,
also magenta (the sprite's top rows are mostly key color). One pixel is
three bytes, the file is `13 + 16 × 16 × 3 = 781` bytes, and every byte
after the header is a pixel with no other meaning. That property — every
byte is what it looks like — is why the format is worth defining by hand:
you can hold a hexdump next to the image and check it yourself.

Two rules of the format do real work here:

- **The text part is text.** Whitespace and `#`-comments may appear
  between the header fields — the format's own rule, and the reader obeys
  it.
- **Exactly one whitespace byte separates the header from the pixels.**
  Not "skip whitespace until data" — the pixel bytes are arbitrary, and the
  first one may itself look like whitespace. A reader that skips blindly
  eats pixels; a reader that counts exactly one byte never does. When a
  format hands you a rule that sounds pedantic, it is usually standing
  between you and a bug like this one.

### Loaded through the seam, kept in the arena

The load is three moves, all lessons we have already paid for:

1. **Read the file whole** — `platform::ReadFile` (lesson 037). A missing
   or unreadable file is a typed failure, not a crash and not an empty
   sprite.
2. **Parse the header by hand** — thirteen bytes of text checked with
   character tests and one number reader. If the magic is not `P6`, the
   numbers do not parse, the maxval is not `255`, or the file does not
   hold *exactly* the pixels the header claims, the load fails. Not
   partially: `LoadSprite` either hands over a complete sprite or names
   what went wrong.
3. **Copy the pixels into the arena** — lesson 041's memory, allocated with
   `ArenaAlloc`, then the file's own bytes go back to the OS with
   `ReleaseFile`. What the engine keeps is its copy.

The copy in step 3 is a decision, not a reflex. The file's bytes belong to
the OS side of the seam; the engine's bytes belong to the arena. One
allocation at startup, one release of the arena at the end — the sprite
lives exactly as long as the engine says it does, and the "who owns the
bytes" question of lesson 004 has exactly one answer for everything in the
engine.

The typed failures are three, and each names a different lie a file can
tell:

```c++
enum SpriteError {
    SPRITE_OK = 0,
    SPRITE_MISSING,   /* the file is not there or cannot be read */
    SPRITE_MALFORMED, /* the bytes are not a complete P6 image */
    SPRITE_NO_ROOM,   /* the arena had no room for the pixels */
};
```

### The bytes, inspected

The run loads the sprite at startup and then looks at it the way Part 0
looked at everything — by byte:

```
$ DISPLAY=:99 ./build/game
engine: sprite assets/sprite.ppm: 16x16, 768 pixel bytes
engine: pixel 0,0 = 255,0,255
engine: pixel 8,8 = 220,40,40
engine: pixel bytes sum to 125580
engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames
engine: arrow keys move the marker; close the window to stop
engine: marker at 308,228
frame 1: update 0.000 ms, render 0.791 ms, present 0.480 ms, total 1.271 ms
...
engine: 24 frames — avg 1.247 ms (update 0.000, render 0.546, present 0.701)
engine: worst frame 1.947 ms (frame 23); present is 56% of the frame
engine: arena: 1229568 of 4194304 bytes used
engine: close reported
engine: closed
```

Each inspection line proves something:

- **`16x16, 768 pixel bytes`** — the header parsed and the count closed:
  `16 × 16 × 3 = 768`, and the file held exactly that many bytes after the
  header. A file claiming 32×32 would have demanded 3072 bytes and been
  refused.
- **`pixel 0,0 = 255,0,255`** — the first three pixel bytes, magenta, the
  key color the hexdump above shows. The bytes the file holds are the bytes
  the engine got.
- **`pixel 8,8 = 220,40,40`** — the sprite's center, a red pixel. One
  sampled point checked against the art you drew says the copy is
  *ordered*, not just complete.
- **`pixel bytes sum to 125580`** — every byte counted once. Change one
  byte anywhere and this number changes: a checksum of the cheapest kind,
  and the habit it comes from (bytes are data you can sum) is the one the
  deep dives will lean on.
- **`arena: 1229568 of 4194304 bytes used`** — `1228800` for the
  framebuffer (lesson 043) plus `768` for the sprite. The asset is in the
  engine's memory, counted like everything else in it.

The window still shows last lesson's marker; the sprite is not drawn yet.
Its bytes are loaded, checked, and waiting — which is the honest order.
The copy loop that will draw them is the next lesson, and it will be one
loop the caches and assembly deep dives can hold up to the light.

### What is fixed now

The format contract closes here, like every other contract in this course:
later lessons read *more* of the format (lesson 050's font sheet rides the
same P6 loader), never reinterpret it. If you author your own sprites, they
are P6 with `255` as the maxval — the reader you wrote today will load them
or tell you exactly which of its rules you broke.

## Code step

One change for this lesson: `assets/sprite.ppm` is authored (a 16×16 P6
image, 13 header bytes + 768 pixel bytes), `src/sprite.h` / `src/sprite.cpp`
bring the loader — header parsed by hand, pixels copied into the arena,
typed failure on anything else — and `main.cpp` loads the sprite at startup
and inspects its bytes. The marker's frame loop is untouched. Its end state
is tagged `lesson-044`.

```diff
diff --git a/assets/sprite.ppm b/assets/sprite.ppm
new file mode 100644
index 0000000..b6f9e58
Binary files /dev/null and b/assets/sprite.ppm differ
diff --git a/src/main.cpp b/src/main.cpp
index 75dfaa6..8febd4a 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -11,6 +11,7 @@
 #include "framebuffer.h"
 #include "frame.h"
 #include "platform.h"
+#include "sprite.h"
 
 namespace engine {
 
@@ -54,6 +55,51 @@ int Run(void)
     ArenaInit(arena, 4 * 1024 * 1024);
     Framebuffer *fb = GetFramebuffer(arena);
 
+    /* Lesson 044: the sprite is a file's bytes. It is loaded once, at
+       startup, through the seam's whole-file read into the arena — and
+       then inspected like Part 0 inspected everything: by byte. */
+    const char *sprite_path = "assets/sprite.ppm";
+    SpriteResult loaded = LoadSprite(arena, sprite_path);
+    if (loaded.error != SPRITE_OK) {
+        switch (loaded.error) {
+        case SPRITE_MISSING:
+            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
+                         sprite_path);
+            break;
+        case SPRITE_MALFORMED:
+            std::fprintf(stderr,
+                         "engine: %s: not a complete P6 image\n",
+                         sprite_path);
+            break;
+        default:
+            std::fprintf(stderr, "engine: %s: no room in the arena\n",
+                         sprite_path);
+            break;
+        }
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    Sprite &sprite = loaded.sprite;
+    long pixel_bytes = (long)sprite.width * sprite.height * 3;
+    long byte_sum = 0;
+    for (long i = 0; i < pixel_bytes; ++i)
+        byte_sum += sprite.pixels[i];
+
+    std::printf("engine: sprite %s: %dx%d, %ld pixel bytes\n", sprite_path,
+                sprite.width, sprite.height, pixel_bytes);
+    std::printf("engine: pixel 0,0 = %d,%d,%d\n", sprite.pixels[0],
+                sprite.pixels[1], sprite.pixels[2]);
+    std::printf("engine: pixel %d,%d = %d,%d,%d\n", sprite.width / 2,
+                sprite.height / 2,
+                sprite.pixels[(sprite.height / 2 * sprite.width +
+                               sprite.width / 2) * 3 + 0],
+                sprite.pixels[(sprite.height / 2 * sprite.width +
+                               sprite.width / 2) * 3 + 1],
+                sprite.pixels[(sprite.height / 2 * sprite.width +
+                               sprite.width / 2) * 3 + 2]);
+    std::printf("engine: pixel bytes sum to %ld\n", byte_sum);
+
     double marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2.0;
     double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
     double started = platform::Now();
diff --git a/src/sprite.cpp b/src/sprite.cpp
new file mode 100644
index 0000000..449b4dc
--- /dev/null
+++ b/src/sprite.cpp
@@ -0,0 +1,123 @@
+// sprite.cpp — the PPM (P6) reader: a header parsed by hand, pixels copied.
+//
+// Lesson 044: the format is small enough to read byte by byte, and this
+// file does exactly that. Part 0's habit holds: nothing in a file is
+// assumed to be there until the bytes say so.
+
+#include "sprite.h"
+
+#include "platform.h"
+
+namespace engine {
+namespace {
+
+/* The header is ASCII; the pixels are anything. These three helpers are
+   the whole "parser" — character tests, comment skipping, one number. */
+
+bool IsSpace(unsigned char c)
+{
+    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' ||
+           c == '\f';
+}
+
+bool IsDigit(unsigned char c)
+{
+    return c >= '0' && c <= '9';
+}
+
+/* Skips whitespace and #-to-end-of-line comments between header fields —
+   the PPM format's own rules for its text part. */
+void SkipBlanks(const unsigned char *data, size_t size, size_t &at)
+{
+    for (;;) {
+        while (at < size && IsSpace(data[at]))
+            ++at;
+        if (at < size && data[at] == '#') {
+            while (at < size && data[at] != '\n')
+                ++at;
+        } else {
+            return;
+        }
+    }
+}
+
+/* One decimal field, or false when the bytes do not form one. Sizes above
+   the sanity bound are refused early — a lying header is malformed, not an
+   allocation request. */
+bool ReadNumber(const unsigned char *data, size_t size, size_t &at, long &out)
+{
+    SkipBlanks(data, size, at);
+    if (at >= size || !IsDigit(data[at]))
+        return false;
+    long value = 0;
+    while (at < size && IsDigit(data[at])) {
+        value = value * 10 + (data[at] - '0');
+        if (value > 1000000L)
+            return false;
+        ++at;
+    }
+    out = value;
+    return true;
+}
+
+} /* namespace */
+
+SpriteResult LoadSprite(Arena &arena, const char *path)
+{
+    SpriteResult result = { { 0, 0, 0 }, SPRITE_OK };
+
+    platform::FileData file = platform::ReadFile(path);
+    if (file.error != platform::FILE_OK) {
+        result.error = SPRITE_MISSING;
+        return result;
+    }
+
+    const unsigned char *data = file.data;
+    size_t size = file.size;
+    size_t at = 0;
+    long width = 0, height = 0, maxval = 0;
+
+    bool ok = size >= 2 && data[0] == 'P' && data[1] == '6';
+    at = 2;
+    ok = ok && ReadNumber(data, size, at, width);
+    ok = ok && ReadNumber(data, size, at, height);
+    ok = ok && ReadNumber(data, size, at, maxval);
+    ok = ok && maxval == 255; /* one byte per channel, as lesson 017 wrote */
+    ok = ok && width > 0 && width <= 4096 && height > 0 && height <= 4096;
+
+    /* Exactly one whitespace byte separates the header from the pixels.
+       Not "skip whitespace here" — the pixel bytes are arbitrary, and the
+       first one may itself look like whitespace. This is the line that
+       keeps the parse honest. */
+    ok = ok && at < size && IsSpace(data[at]);
+    ++at;
+
+    /* Complete or nothing: the file must hold exactly the pixels the
+       header claims — no short read presented as a sprite. */
+    size_t pixel_bytes = (size_t)width * (size_t)height * 3;
+    ok = ok && size - at == pixel_bytes;
+
+    if (!ok) {
+        result.error = SPRITE_MALFORMED;
+        platform::ReleaseFile(file);
+        return result;
+    }
+
+    unsigned char *pixels = (unsigned char *)ArenaAlloc(arena, pixel_bytes, 4);
+    if (!pixels) {
+        result.error = SPRITE_NO_ROOM;
+        platform::ReleaseFile(file);
+        return result;
+    }
+    for (size_t i = 0; i < pixel_bytes; ++i)
+        pixels[i] = data[at + i];
+    platform::ReleaseFile(file);
+
+    result.sprite.pixels = pixels;
+    result.sprite.width = (int)width;
+    result.sprite.height = (int)height;
+    result.error = SPRITE_OK;
+    return result;
+}
+
+} /* namespace engine */
diff --git a/src/sprite.h b/src/sprite.h
new file mode 100644
index 0000000..5be7140
--- /dev/null
+++ b/src/sprite.h
@@ -0,0 +1,44 @@
+// sprite.h — a sprite as loaded bytes: a PPM image's pixels in our arena.
+//
+// Lesson 044: an asset is a file, read whole through the seam (lesson 037)
+// and kept in the engine's own memory (lesson 041). The format is PPM (P6)
+// — the one Part 0's lesson 017 wrote by hand: a tiny text header, then
+// every pixel's three bytes in order. No library parses it; we do, and the
+// bytes stay inspectable.
+#ifndef SPRITE_H
+#define SPRITE_H
+
+#include "arena.h"
+
+namespace engine {
+
+/* A sprite: one image's pixels in the engine's memory — row after row,
+   three bytes each (red, green, blue), exactly the file's pixel section. */
+struct Sprite {
+    unsigned char *pixels; /* width * height * 3 bytes */
+    int width;
+    int height;
+};
+
+/* A load either hands over a complete sprite or names what went wrong —
+   never a half-loaded sprite presented as success. */
+enum SpriteError {
+    SPRITE_OK = 0,
+    SPRITE_MISSING,   /* the file is not there or cannot be read */
+    SPRITE_MALFORMED, /* the bytes are not a complete P6 image */
+    SPRITE_NO_ROOM,   /* the arena had no room for the pixels */
+};
+
+struct SpriteResult {
+    Sprite sprite;
+    SpriteError error; /* SPRITE_OK exactly when sprite.pixels is non-0 */
+};
+
+/* Loads a PPM (P6) image from a file. The header is parsed byte by byte,
+   the pixel bytes are copied into the arena, and the file's own bytes go
+   back to the OS — what the engine keeps is its copy. */
+SpriteResult LoadSprite(Arena &arena, const char *path);
+
+} /* namespace engine */
+
+#endif
```

The sprite's bytes are binary and so is its diff — git prints `Binary files
… differ` for it. The header and the first pixel bytes are the hexdump in
the prose above; `git diff --binary` prints the full patch.

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The header that lies *(predict-the-output)*

Copy `assets/sprite.ppm` to a file of your own and edit *only* its header
so it claims `32 32`. Before you run anything, write down what the run will
report and which check will refuse — the size fields or the pixel count.
Then make the failure report name which check refused: one message for the
header fields, one for the pixel bytes, and one for a file that cannot be
read at all — and give the run a way to take the path from its first
argument so you can aim the loader anywhere. Run against your corrupted
copy and compare with your prediction. Finish with the question the bytes
ask: if the loader had trusted the header, those 768 pixels would have
been drawn as 32 rows of what — and where would the missing 2304 bytes
have come from?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-044/ex1.md)

### Exercise 2 — Sprite, meet window *(extend-the-code)*

The sprite's bytes are in the arena; the window still shows last lesson's
marker. Put them together with the loop lesson 045 turns into a function:
draw the sprite into the framebuffer with `PutPixel` at a fixed spot,
every frame, before the marker. Then read three pixels back with
`GetPixel` — the top-left, the center, and one pixel of the key color —
and print the file's bytes against the framebuffer's at each point. What
does the key-colored pixel tell you about what lesson 045 owes you?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-044/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 043 — the closing demo: platform layer done](../part-1/lesson-043-demo.md) ·
**Next:** [Lesson 045 — the clipped, transparent blit](lesson-045-blit.md) ·
**Code tag:** [`lesson-044`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-044)
