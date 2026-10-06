# Solution: exercise 1 — The window gets your title

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 027 — the platform seam and the first X11 window](../../lessons/part-1/lesson-027-first-window.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

Three files, one idea: a title is plain text, so it belongs in the seam like
any other plain data. `platform.h` widens `OpenWindow` by one `const char *`
— still no OS idiom, still no OS header. `platform_x11.cpp` is the only file
that knows what a title is made of on this OS: the parameter lands straight
in `XStoreName`, next to the window id it applies to. `main.cpp` passes the
string at the call site and changes nothing else about its day.

The engine's window is now findable by name. Under the headless check:

```
$ DISPLAY=:99 xdotool search --name "lesson 027"
2097153
$ DISPLAY=:99 xdotool getwindowgeometry 2097153
Window 2097153
  Position: 0,0 (screen: 0)
  Geometry: 640x480
```

Count the files the change touched: one interface, one implementation, one
call site. A Win32 implementation of the seam would take the same `const char
*` and pass it to whatever Win32 calls a title — and the call site in
`main.cpp` would not notice. Which of the three files is allowed to know what
a title is made of? Only `platform_x11.cpp`. The seam carries data; the
implementations give it meaning on their OS.
