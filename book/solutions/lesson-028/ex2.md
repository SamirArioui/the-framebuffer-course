# Solution: exercise 2 — Two ways the news arrives

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 028 — the event pump](../../lessons/part-1/lesson-028-event-pump.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

Closing a window sounds like one event; on X11 it is a conversation with two
possible endings, and the pump handles both because either can happen:

- **The polite request.** A window manager does not destroy a window behind
  its owner's back. It *asks*: a `ClientMessage` event whose first word is
  the `WM_DELETE_WINDOW` atom — the protocol `XSetWMProtocols` registered
  when the window opened. Your desktop's close button produces this one.
- **The deed.** The window can also simply stop existing — destroyed
  directly, as `xdotool windowclose` does, or killed by the server. The news
  arrives as `DestroyNotify`, and it is not a request at all: the OS
  resource is already gone.

Both fold into the same piece of state, because to the engine they mean the
same thing: stop. But they are not the same at cleanup time — after
`DestroyNotify` the window id is dead, and asking X to destroy it again
would be an X error. That is why the fold records the difference
(`window->xwindow = 0`) even though `CloseRequested` does not.

With the instrumenting print applied, the two endings are visible. The
abrupt one:

```
platform: window destroyed (DestroyNotify)
engine: window 640x480 open — waiting for news
engine: close reported
engine: closed
```

The polite one (here sent by a stand-in window manager; on your desktop your
close button sends the same message):

```
platform: close request (ClientMessage)
engine: window 640x480 open — waiting for news
engine: close reported
engine: closed
```

(The platform's line appears first in these transcripts because `stderr` is
unbuffered while `stdout` waited in its buffer until exit — the same
stream-ordering behavior lesson 001's drills ran into. On a terminal the two
lines interleave in real time.)

What differs on your own machine is *which* ending your close gesture
produces — that is why this exercise is port-shaped. What does not differ is
the engine: it saw one flag turn true and exited the same way both times.
