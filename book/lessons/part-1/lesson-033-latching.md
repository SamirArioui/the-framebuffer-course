# Lesson 033 — latching brief presses and tracking focus

{{#include ../../stability-horizon.md}}

## Prose

Lesson 032 ended at a cliff: the polled state answers "what is down *now*",
and a press that is already over is not down now — press and release between
two polls and the engine never sees it. This lesson closes that hole with a
second piece of state, the **latch**, and then fixes the next bug waiting in
input state: the key that keeps moving after the window lost the keyboard.

### The latch

The seam grows one query:

```c++
bool KeyPressed(Window *window, Key key);
```

`KeyDown` says *is down now*; `KeyPressed` says **went down since the last
time you asked**. It is the spec's promise made mechanical: *a key pressed
and released within one frame is not lost* — the press lights the latch when
it happens and the latch stays lit until the engine has seen it. Reading it
clears it: the latch is a small piece of state with one consumer, and the
engine polls it like everything else. Still no event stream.

The fold lights it on the **edge** — the key going from up to down:

```c++
if (event.type == KeyPress) {
    if (!window->keys[key])
        window->pressed[key] = true;
    window->keys[key] = true;
} else {
    window->keys[key] = false;
}
```

The edge matters because of lesson 032's auto-repeat: a held key streams
presses, but a held key did not *go down* again. One press, one latch,
however long the hold — and however many repeats arrive.

The proof, with a tap faster than the poll — press and release landing in
one batch:

```
$ DISPLAY=:99 xdotool key --delay 0 --window <id> space
engine: polled: -
engine: pressed space
engine: polled: -
```

The polled state is honest — nothing is down — and the press is *still
there*, reported by the latch on the very next poll. (Whether a tap lands
in one pump batch or two is timing; the latch makes the outcome identical
either way. That is the whole point of it.)

### Tracking focus

The second bug is older than this lesson and it has a name: **the stuck
key**. Hold the movement key, alt-tab away, release the key in the other
window. The release happens over there — our window never sees it — and
`keys[KEY_LEFT]` stays true. Let go of the keyboard and watch the marker
keep going.

The OS knows when this happens: `FocusIn` and `FocusOut` are events like any
other, and the fold treats them as input news — because that is exactly what
they are:

```c++
} else if (event.type == FocusOut) {
    window->focused = false;
    for (int i = 0; i < KEY_COUNT; ++i)
        window->keys[i] = false;
}
```

Every held key is **dropped** on focus loss. Not marked released, not left
for later — dropped, because no release event will ever arrive for them and
state that can never end is not state. Focus comes back (`FocusIn`) but the
keys do not: focus is not input state, it is *news about* input state.

The engine sees the other half through a second query, `HasFocus` — plain
state again — and can dim, pause, or ignore input when the window is not the
one being typed at:

```
$ DISPLAY=:99 xdotool windowfocus <id>
$ DISPLAY=:99 xdotool keydown --window <id> Left
$ DISPLAY=:99 xdotool windowfocus 0
engine: polled: -
engine: focus gained
engine: polled: left
engine: pressed left
engine: polled: -
engine: focus lost
```

Held `left`, focus left, and the very next poll is empty. The press is not
even lost — `pressed left` was already reported while the key went down. The
state is exactly as much input as the window actually owns.

### What the engine now knows

Three queries, three questions, all state:

| Query | Question |
| ----- | -------- |
| `KeyDown` | is the key down right now? |
| `KeyPressed` | did it go down since I last looked? |
| `HasFocus` | is this window the one being typed at? |

Lesson 034 moves a marker with the first one — and the latches are why the
marker's jump will not need a second keypress "just to be sure".

## Code step

One change for this lesson: the seam grows `KeyPressed` and `HasFocus`, the
fold latches presses on their edge and drops held keys when focus is lost,
and `main.cpp` reports the latches and the focus changes alongside the
polled state. Its end state is tagged `lesson-033`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 91be4f0..0d0d64b 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -74,6 +74,7 @@ int Run(void)
        React first: if the news was "the window is gone", there is nothing
        left to present to. */
     int exit_code = 0;
+    bool had_focus = platform::HasFocus(opened.window);
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -90,6 +91,18 @@ int Run(void)
         }
         std::printf(any ? "\n" : " -\n");
 
+        /* The latches: presses that ended before this poll are not lost. */
+        for (int k = 0; k < platform::KEY_COUNT; ++k)
+            if (platform::KeyPressed(opened.window, (platform::Key)k))
+                std::printf("engine: pressed %s\n", key_names[k]);
+
+        /* Focus: reported when it changes. */
+        bool focus = platform::HasFocus(opened.window);
+        if (focus != had_focus) {
+            std::printf("engine: focus %s\n", focus ? "gained" : "lost");
+            had_focus = focus;
+        }
+
         if (!platform::Present(opened.window, fb->pixels, fb->width,
                                fb->height)) {
             /* A present can fail because the window died mid-copy — that
diff --git a/src/platform.h b/src/platform.h
index aac8d48..643bdef 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -46,6 +46,16 @@ enum Key {
    call. State, not events — the engine asks, it never consumes a stream. */
 bool KeyDown(const Window *window, Key key);
 
+/* True if the key went down since the last time that key was polled this
+   way. A press that ended before the poll is not lost; the latch clears
+   when the engine has seen it. */
+bool KeyPressed(Window *window, Key key);
+
+/* True while the window has keyboard focus. Keys held when focus is lost
+   are dropped by the platform layer — no release event will ever arrive
+   for them. */
+bool HasFocus(const Window *window);
+
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
    state afterwards. Blocks until there is news or the run is interrupted. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 22f4c19..4e23966 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -29,6 +29,8 @@ struct Window {
     ::Window xwindow;
     bool close_requested;
     bool keys[KEY_COUNT];
+    bool pressed[KEY_COUNT]; /* latched: went down since last observed */
+    bool focused;
 };
 
 /* The OS state for one window, in static storage: no new, no delete — the
@@ -99,7 +101,7 @@ WindowResult OpenWindow(int width, int height)
     XSetWMProtocols(display, xwindow, &wm_delete_window, 1);
     XSelectInput(display, xwindow,
                  StructureNotifyMask | ExposureMask | KeyPressMask |
-                     KeyReleaseMask);
+                     KeyReleaseMask | FocusChangeMask);
 
     /* Auto-repeat would otherwise look like release-then-press every
        repeat: a held key would flicker in polled state. Detectable
@@ -114,8 +116,11 @@ WindowResult OpenWindow(int width, int height)
     window_state.display = display;
     window_state.xwindow = xwindow;
     window_state.close_requested = false;
-    for (int i = 0; i < KEY_COUNT; ++i)
+    for (int i = 0; i < KEY_COUNT; ++i) {
         window_state.keys[i] = false;
+        window_state.pressed[i] = false;
+    }
+    window_state.focused = false;
 
     /* The interrupt is part of the window's take: from here on, Ctrl+C is
        news like any other — and X errors are recorded, not fatal. */
@@ -154,8 +159,26 @@ static void HandleEvent(Window *window, XEvent &event)
         window->xwindow = 0;
     } else if (event.type == KeyPress || event.type == KeyRelease) {
         int key = KeyIndex(XLookupKeysym(&event.xkey, 0));
-        if (key >= 0)
-            window->keys[key] = (event.type == KeyPress);
+        if (key >= 0) {
+            if (event.type == KeyPress) {
+                /* The latch lights on the edge — a key going from up to
+                   down. Auto-repeat presses (a held key) arrive as presses
+                   too, but a held key did not go down again. */
+                if (!window->keys[key])
+                    window->pressed[key] = true;
+                window->keys[key] = true;
+            } else {
+                window->keys[key] = false;
+            }
+        }
+    } else if (event.type == FocusIn) {
+        window->focused = true;
+    } else if (event.type == FocusOut) {
+        /* Keys held while focus left will never send their release here —
+           drop them or they stay down forever. */
+        window->focused = false;
+        for (int i = 0; i < KEY_COUNT; ++i)
+            window->keys[i] = false;
     }
 }
 
@@ -164,6 +187,19 @@ bool KeyDown(const Window *window, Key key)
     return window && key >= 0 && key < KEY_COUNT && window->keys[key];
 }
 
+bool KeyPressed(Window *window, Key key)
+{
+    if (!window || key < 0 || key >= KEY_COUNT || !window->pressed[key])
+        return false;
+    window->pressed[key] = false; /* observed; the latch clears */
+    return true;
+}
+
+bool HasFocus(const Window *window)
+{
+    return window && window->focused;
+}
+
 void PumpEvents(Window *window)
 {
     if (!window || !window->display)
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The press count *(extend-the-code)*

The latch answers "did it go down?" — at least once. Two taps in one batch
light it exactly like one tap. Grow it into the general form: a count per
key — `KeyPressCount` reports how many times the key went down since the
last poll of that key — and keep `KeyPressed` meaning what it means
(`count > 0`). Show it counting: two taps in one batch
(`xdotool key --delay 0 --repeat 2 --window <id> space`) must report
`pressed space x2`. While you are there: does a held key auto-repeating
increment the count? Why not?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-033/ex1.md)

### Exercise 2 — The key that will not let go *(port-to-your-own-machine)*

A teammate's build has the classic bug: hold the movement key, alt-tab
away, release the key in the other window — and the marker keeps moving
after you let go. Predict exactly which piece of state survives and why no
event will ever fix it. Then make the fold narrate: one instrumenting line
on `FocusIn` and `FocusOut` inside the implementation. On your own desktop,
hold a key and click another window; watch the drop happen in the report —
and explain why dropping *all* held keys on focus loss is correct even
though some of them may still be physically down.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-033/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 032 — polled input state](lesson-032-polled-input.md) ·
**Next:** [Lesson 034 — the first interactive frame](lesson-034-first-frame.md) ·
**Code tag:** [`lesson-033`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-033)
