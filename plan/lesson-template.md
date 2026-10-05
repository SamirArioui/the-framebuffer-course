<!--
  Lesson page template.
  Copy to book/lessons/lesson-NNN.md and fill every placeholder.
  The authoring contract is plan/conventions.md:
    - exactly one code step per lesson, co-committed with this prose
    - prose references symbols and structural locations, never line numbers
    - exercises are written before the prose; no lesson publishes without them
    - each prompt ends with its solution link, never the solution itself
  The stability-horizon include below is the correct path from book/lessons/.
-->

# Lesson NNN — <lesson title>

{{#include ../stability-horizon.md}}

## Prose

<Explain the idea and the machine underneath it. Reference code only by
symbol and structure: "the clipping branch of `RenderRectangle`" — never by
line number.>

## Code step

<Exactly one code step: the change to apply to `src/` in this lesson. It is
committed together with this prose and the lesson's end state is tagged
`lesson-NNN`.>

```diff
<the change, as a diff against the previous lesson's end state>
```

## Exercises

<Density is set by the part — Part 0: 3-4 short drills; Parts 1-2: 1-2
"make it yours" extensions; Parts 3-4: 1-2 mixed; Part 5: at most 2 larger
challenges. Each exercise fits exactly one archetype: predict-the-output,
fix-the-crash, extend-the-code, measure-the-performance, explain-in-prose,
port-to-your-own-machine.>

### Exercise 1 — <title> *(<archetype>)*

<The prompt states the task and nothing else. No solution content appears
before the prompt's end.>

> **Solution:** [ex1 — diff + walkthrough](../solutions/lesson-NNN/ex1.md)

### Exercise 2 — <title> *(<archetype>)*

<The prompt states the task and nothing else.>

> **Solution:** [ex2 — diff + walkthrough](../solutions/lesson-NNN/ex2.md)

---

**Part:** [<part title>](../index.md) ·
**Previous:** [<previous lesson title>](lesson-NNN.md) ·
**Next:** [<next lesson title>](lesson-NNN.md) ·
**Code tag:** [`lesson-NNN`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-NNN)

<!-- Code-tag links point at the lesson's end-of-lesson code state in the
     repository: https://github.com/SamirArioui/the-framebuffer-course -->
