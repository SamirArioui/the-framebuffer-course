# Lesson 051 — text on screen

{{#include ../../stability-horizon.md}}

## Prose

Last lesson the font became sprites; today the sprites become **text**:
strings laid out, glyphs drawn, and the rule for characters the font does
not have. This lesson delivers the MVD's third obligation — *bitmap text
for screens* — in the form the whole rest of the course uses: a HUD line
drawn through the one blit. Strings, spacing, and a missing-glyph rule
sound like small things; they are the difference between a renderer and a
renderer that can talk.

### The layout rule

`DrawText` is a loop and a rule: **one character, one slot**.

```c++
void DrawText(Framebuffer &fb, const Font &font, const char *text, int x,
              int y)
{
    for (int i = 0; text[i]; ++i) {
        const Sprite *glyph = FontGlyph(font, text[i]);
        if (glyph) /* the missing-glyph rule: draw nothing, keep the slot */
            BlitSprite(fb, *glyph, x + i * FONT_CELL, y);
    }
}
```

Character *i* draws at `x + i × FONT_CELL` — the character's position is
computed from its index alone. That is the whole layout: no cursor state,
no kerning, no measuring. `TextWidth` is the same arithmetic read out
(`length × FONT_CELL`), which is what centering or right-aligning a line
will need later.

The fixed 8-pixel advance is a deliberate simplification — a *monospace*
layout. Proportional fonts (where `i` is narrower than `W`) need per-glyph
advance widths in the sheet's format and a cursor that accumulates them;
that is a format extension for a later edition, not a hidden feature
here. The course's format stays what lesson 050 fixed.

### The missing-glyph rule

Some bytes are not in the sheet — the `µ` of `µs`, a UTF-8 accent, a
control character. The rule (the spec's words): *a character the font
does not provide is skipped without corrupting the layout of the
following characters.* In the loop above that is exactly one `if`: no
glyph, no draw — **but the slot advances anyway**. The next character
sits where the layout says it sits, not where a redraw-without-the-missing
character would put it.

The check draws the case and reads it back:

```
engine: font assets/font.ppm: 96 glyphs of 8x8 from a 128x48 sheet
...
engine: text check: "AB" at 200,120 — 2 of 2 slots have ink, width 16
engine: text check: "A?B" with a missing character — ink pixels per slot: 18, 0, 0, 23
```

The second line is `A`, `µ`, `B` — and the µ is **two bytes** (U+00B5
in UTF-8 is `0xC2 0xB5`), so the string is four characters to the engine
and the check reads four slots: `18` ink pixels for `A`, `0` and `0` for
the two missing bytes' slots, and `23` for `B` — at `x + 3 × 8`, exactly
where the layout put it. The gap is *empty* and the text is
*undisturbed*. Those are two different claims and the slot map checks
both.

One more distinction the rule depends on, worth seeing explicitly:

| | `space` (byte 32) | byte 200 |
| --- | --- | --- |
| `FontGlyph` | **has a glyph** — cell 0 | **returns 0** — outside the sheet |
| The glyph's pixels | 64 key-colored pixels | — |
| What draws | nothing (the key writes nothing) | nothing (there is no sprite) |
| The slot | advances | advances |

Two different mechanisms, the same pixels on screen. A space is a glyph
that happens to be all transparent; a missing byte has no glyph at all.
The layout does not care which — which is why the rule can be one `if`.

### The record names text

Text is its own subsystem now, and the frame record says so — the second
named phase inside `render`, following lesson 046's `sprites`:

```
frame 1: update 0.000 ms, render 0.388 ms (sprites 0.001, text 0.002), present 0.808 ms, total 1.197 ms
...
engine: 1 frames — avg 1.197 ms (update 0.000, render 0.388 incl. sprites 0.001, text 0.002, present 0.808)
```

`text 0.002` is five glyphs through the blit — a fifth of a microsecond
per glyph, the same per-pixel cost as any sprite draw because it *is* a
sprite draw with a loop around it. The named phase exists so the frame
budget can say that, and so the final table (lesson 058) has a text row
that grows with the HUD instead of hiding inside `render`.

### O3, delivered

The MVD's checklist line — *bitmap text for screens* — is now
implemented and demonstrable: strings of any length, any of the 96
characters, laid out and drawn with the same clipping and transparency as
everything else on screen. The demo's HUD (the `SCORE` label) draws
through `DrawText` from this lesson on; lesson 058's frame-budget table
uses the same call to print its own numbers. Text was the last drawing
capability the closing demo needs — the world behind it is next.

## Code step

One change for this lesson: `src/text.h` / `src/text.cpp` bring the
layout loop and the missing-glyph rule, `frame.h` / `frame.cpp` grow the
record's named `text` phase, and `main.cpp` checks the text claims
(including the missing-character case), draws the HUD through
`DrawText`, and carries the new field through the log line and the
account. The font, the sprite, and the blitter are untouched. Its end
state is tagged `lesson-051`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index e104e47..36a2827 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -14,6 +14,7 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     stats.present_sum += frame.present;
     stats.total_sum += frame.total;
     stats.sprites_sum += frame.sprites;
+    stats.text_sum += frame.text;
     if (frame.total > stats.worst) {
         stats.worst = frame.total;
         stats.worst_number = frame.number;
diff --git a/src/frame.h b/src/frame.h
index bea2fbf..3a68029 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -23,6 +23,7 @@ struct FrameRecord {
        (lesson 058) grows from. The named times are inside render, never
        instead of it: render stays the phase, these say where it went. */
     double sprites; /* sprite draws through the blit */
+    double text;    /* lesson 051: text drawing — glyphs through the blit */
 };
 
 /* The running account: every frame measured so far. */
@@ -33,6 +34,7 @@ struct FrameStats {
     double present_sum;
     double total_sum;
     double sprites_sum; /* lesson 046's named sub-phase, summed like the rest */
+    double text_sum;
     double worst;      /* the longest frame so far */
     long worst_number; /* and which one it was */
 };
diff --git a/src/main.cpp b/src/main.cpp
index 9aba782..c60358b 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -14,6 +14,7 @@
 #include "frame.h"
 #include "platform.h"
 #include "sprite.h"
+#include "text.h"
 
 namespace engine {
 
@@ -289,6 +290,48 @@ int Run(void)
                     ink, key);
     }
 
+    /* Lesson 051: the text claims — glyphs laid out in order, and the
+       missing-glyph rule: the character the font lacks draws nothing and
+       the characters after it keep their slots. */
+    ClearBuffer(*fb, 32, 32, 64);
+    DrawText(*fb, font, "AB", 200, 120);
+    {
+        int slots_with_ink = 0;
+        for (int s = 0; s < 2; ++s) {
+            bool ink = false;
+            for (int j = 0; j < FONT_CELL && !ink; ++j)
+                for (int i = 0; i < FONT_CELL && !ink; ++i) {
+                    unsigned char r, g, b;
+                    GetPixel(*fb, 200 + s * FONT_CELL + i, 120 + j, r, g, b);
+                    if (r != 32 || g != 32 || b != 64)
+                        ink = true;
+                }
+            if (ink)
+                ++slots_with_ink;
+        }
+        std::printf("engine: text check: \"AB\" at 200,120 — %d of 2 slots have ink, width %d\n",
+                    slots_with_ink, TextWidth("AB"));
+    }
+    ClearBuffer(*fb, 32, 32, 64);
+    DrawText(*fb, font, "A\xC2\xB5" "B", 200, 140); /* "AµB": µ is not in the sheet */
+    {
+        /* Four bytes, four slots: the µ is two bytes, and each keeps its
+           slot — the layout follows the bytes, and B lands where the
+           layout says. */
+        int slot_state[4] = { 0, 0, 0, 0 };
+        for (int s = 0; s < 4; ++s) {
+            for (int j = 0; j < FONT_CELL; ++j)
+                for (int i = 0; i < FONT_CELL; ++i) {
+                    unsigned char r, g, b;
+                    GetPixel(*fb, 200 + s * FONT_CELL + i, 140 + j, r, g, b);
+                    if (r != 32 || g != 32 || b != 64)
+                        ++slot_state[s];
+                }
+        }
+        std::printf("engine: text check: \"A?B\" with a missing character — ink pixels per slot: %d, %d, %d, %d\n",
+                    slot_state[0], slot_state[1], slot_state[2], slot_state[3]);
+    }
+
     double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
     double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
@@ -345,19 +388,15 @@ int Run(void)
                         (int)sprite_y, platform::Now() - started);
 
         /* Render: every frame draws the whole scene — clear, then the
-           sprite through the one blit. The sprite draw is timed as its own
-           named phase: the first subsystem the frame record can name.
-           Glyphs are sprites too (lesson 050) — they count here until
-           lesson 051 names the text phase. */
+           sprite and the text, each timed as its own named phase: the
+           subsystems the frame record can name. */
         ClearBuffer(*fb, 32, 32, 64);
         double t_sprites = platform::Now();
         BlitSprite(*fb, sprite, (int)sprite_x, (int)sprite_y);
-        for (int li = 0; HUD_LABEL[li]; ++li) {
-            const Sprite *glyph = FontGlyph(font, HUD_LABEL[li]);
-            if (glyph)
-                BlitSprite(*fb, *glyph, 8 + li * FONT_CELL, 8);
-        }
         frame.sprites = platform::Now() - t_sprites;
+        double t_text = platform::Now();
+        DrawText(*fb, font, HUD_LABEL, 8, 8);
+        frame.text = platform::Now() - t_text;
 
         frame.render = platform::Now() - t1;
         double t2 = platform::Now();
@@ -380,20 +419,21 @@ int Run(void)
 
         /* The frame log: one line per record — the format grows its named
            fields, one per subsystem, as the parts name them. */
-        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f), present %.3f ms, total %.3f ms\n",
+        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f, text %.3f), present %.3f ms, total %.3f ms\n",
                     frame.number, frame.update * 1e3, frame.render * 1e3,
-                    frame.sprites * 1e3, frame.present * 1e3,
-                    frame.total * 1e3);
+                    frame.sprites * 1e3, frame.text * 1e3,
+                    frame.present * 1e3, frame.total * 1e3);
     }
 
     /* The account: what the frames actually cost, including the honest
        price of the presentation copy. */
     if (stats.frames) {
         double n = (double)stats.frames;
-        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, present %.3f)\n",
+        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, text %.3f, present %.3f)\n",
                     stats.frames, stats.total_sum / n * 1e3,
                     stats.update_sum / n * 1e3, stats.render_sum / n * 1e3,
-                    stats.sprites_sum / n * 1e3, stats.present_sum / n * 1e3);
+                    stats.sprites_sum / n * 1e3, stats.text_sum / n * 1e3,
+                    stats.present_sum / n * 1e3);
         std::printf("engine: worst frame %.3f ms (frame %ld); present is %.0f%% of the frame\n",
                     stats.worst * 1e3, stats.worst_number,
                     100.0 * stats.present_sum / stats.total_sum);
diff --git a/src/text.cpp b/src/text.cpp
new file mode 100644
index 0000000..3d9edff
--- /dev/null
+++ b/src/text.cpp
@@ -0,0 +1,31 @@
+// text.cpp — the layout loop.
+//
+// Lesson 051: every character advances the layout; only characters with
+// a glyph draw. Two rules, and the second one is the whole of the
+// missing-glyph behavior.
+
+#include "text.h"
+
+#include "blit.h"
+
+namespace engine {
+
+void DrawText(Framebuffer &fb, const Font &font, const char *text, int x,
+              int y)
+{
+    for (int i = 0; text[i]; ++i) {
+        const Sprite *glyph = FontGlyph(font, text[i]);
+        if (glyph) /* the missing-glyph rule: draw nothing, keep the slot */
+            BlitSprite(fb, *glyph, x + i * FONT_CELL, y);
+    }
+}
+
+int TextWidth(const char *text)
+{
+    int n = 0;
+    while (text[n])
+        ++n;
+    return n * FONT_CELL;
+}
+
+} /* namespace engine */
diff --git a/src/text.h b/src/text.h
new file mode 100644
index 0000000..93c0bb4
--- /dev/null
+++ b/src/text.h
@@ -0,0 +1,27 @@
+// text.h — strings drawn through the blit, glyph by glyph.
+//
+// Lesson 051: text is sprites with bookkeeping — a layout rule and the
+// one blit. The missing-glyph rule lives here: a character the font does
+// not have draws nothing, and the layout does not notice.
+#ifndef TEXT_H
+#define TEXT_H
+
+#include "font.h"
+#include "framebuffer.h"
+
+namespace engine {
+
+/* Draws a string with its top-left at (x, y): one glyph per character,
+   each FONT_CELL pixels to the right of the last — including characters
+   with no glyph, which draw nothing but keep their slot. The layout is
+   per character, so the string's shape never depends on which glyphs the
+   font happens to have. Clipping and transparency are the blitter's. */
+void DrawText(Framebuffer &fb, const Font &font, const char *text, int x,
+              int y);
+
+/* How many pixels a string's layout covers: its length x FONT_CELL. */
+int TextWidth(const char *text);
+
+} /* namespace engine */
+
+#endif
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The HUD that counts *(extend-the-code)*

A label is static; a HUD is data. Build the string at runtime — the
sprite's position, formatted into a buffer with `snprintf` — and draw it
as a second text line every frame, with a startup check that reports the
line's slot count and how many slots carry ink. Predict the two numbers
before you run, then reconcile — and answer the question the format
asks: what happens to the slot count when the position crosses from
`312` to `99`?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-051/ex1.md)

### Exercise 2 — The two kinds of nothing *(predict-the-output)*

A space and a missing byte both draw nothing — but not the same way.
Before you run anything, write down what `FontGlyph` returns for `' '`
and for byte `200`, and what the slot map of `"A B"` will report. Then
extend the run to print both answers and the slot map, and explain why
the two "nothings" of this lesson's table are the same pixels by
*different mechanisms* — and what that means for a font whose sheet
genuinely lacks a glyph you need.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-051/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 050 — the bitmap font as an asset](lesson-050-font.md) ·
**Next:** [Lesson 052 — the tilemap asset format](lesson-052-tilemap.md) ·
**Code tag:** [`lesson-051`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-051)
