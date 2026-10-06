# Solution: exercise 2 — Where the boundary is

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 027 — the platform seam and the first X11 window](../../lessons/part-1/lesson-027-first-window.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The claim — *engine code never names an OS type* — is not a style rule to
take on faith; it is visible in the objects the compiler produces. Build the
lesson's end state and look at the two object files (`build.sh` keeps one
object per source, so the boundary shows up as two files):

```
$ nm build/obj/main.o | c++filt
0000000000000000 T engine::Run()
0000000000000000 r engine::WINDOW_WIDTH
0000000000000004 r engine::WINDOW_HEIGHT
                 U platform::OpenWindow(int, int)
                 U platform::CloseWindow(platform::Window*)
                 U fprintf
                 U getchar
000000000000009b T main
                 U printf
                 U puts
                 U stderr
```

The prediction holds: `main.o` references the two seam functions and the C
standard library, and not one `X*` name. The two `U` rows it does have are
the seam itself — `platform::OpenWindow` and `platform::CloseWindow`, with
`platform::Window*` demangling to a *pointer to an incomplete type*: the
object file carries the name and no idea what is inside. (`puts` is there
because the compiler turned a `printf` of one constant line into it — the
same kind of compile-down thinking the language law is built on.)

The other side of the boundary:

```
$ nm build/obj/platform_x11.o | c++filt
                 U XCloseDisplay
                 U XCreateSimpleWindow
                 U XDestroyWindow
                 U XFlush
                 U XMapWindow
                 U XOpenDisplay
                 U XStoreName
0000000000000000 T platform::OpenWindow(int, int)
000000000000018a T platform::CloseWindow(platform::Window*)
0000000000000000 b platform::window_state
                 U __stack_chk_fail
```

Every `X*` reference in the whole program lives in this one object — the
undefined symbols are exactly the Xlib calls the implementation makes, and
the two `T` symbols are the same seam functions, defined here.

What a Win32 port would touch, then, is decided by the link step: a Win32
implementation provides `platform::OpenWindow` and `platform::CloseWindow`
with the same signatures, moves the `window_state` guts to Win32's own
handles, and `main.o` is *reused unchanged* — it was compiled against
`platform.h` and knows nothing else. Only `platform_x11.cpp`'s twin file
(and the build line's one OS library) changes.

The diff above is the smallest confirming change there is: one instrumenting
print inside the implementation, so a run names its own boundary —

```
platform: x11 implementation active
engine: window 640x480 open — press enter to close
```

— while the engine stays silent about which OS it is talking to. It cannot
know. That is the seam.
