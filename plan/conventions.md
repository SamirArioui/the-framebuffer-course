# Authoring conventions

The single authoring contract for this course. Every rule below maps to a
requirement in this change's specs
(`openspec/changes/course-curriculum-foundation/specs/`); no rule is allowed to
drift from them.

## 1. Lessons and lesson tags

- A lesson contains **prose, exactly one code step, and its exercises**, and
  targets **30-60 minutes** of student read-and-code time.
  *(lesson-format: Lesson anatomy)*
- Each published lesson's end-of-lesson code state is tagged **`lesson-NNN`**
  with **zero-padded** sequence numbers: `lesson-001`, `lesson-002`, … One tag
  per published lesson.
  *(codebase-continuity: Linear history with lesson tags)*
- `src/` keeps **one linear commit history**. Diffing two lesson tags equals
  exactly the code steps of the lessons between them.
  *(codebase-continuity: Linear history with lesson tags)*
- **Prose and its code step are co-committed**: a lesson's prose change and its
  code change land in the same commit, so a tag means "code and text as of
  lesson N". Checking out a tag always shows both.
  *(lesson-format: Prose and code co-committed)*

## 2. Symbol-reference discipline

Lesson prose references code **only by symbol names and structural
descriptions** — "the clipping branch of `RenderRectangle`" — **never by line
numbers**. When a code change shifts line positions, later lessons' prose stays
accurate without edits. This is what makes class-2 revisions (below) cheap.
*(lesson-format: Symbol-reference discipline)*

## 3. Stability horizon and the three-class revision policy

The published course declares a **frozen prefix** of stable lessons and a
**volatile tail**. The current horizon lives in one source-of-truth file,
`book/stability-horizon.md`, and is rendered as the banner on the site's pages.
Tags inside the frozen prefix do **not** move except for behavior-changing
correctness fixes; a redesign that would move one is routed elsewhere.
*(codebase-continuity: Stability horizon)*

| Class | Change | Procedure |
| ----- | ------ | ---------- |
| 1 | **Prose-only** (typo, clarity) | Edit `book/` in place. No tag moves. |
| 2 | **Behavior-changing code fix in the frozen prefix** | **Surgical**: apply the fix, rebase-forward the downstream lesson states, re-tag them, and fix the downstream prose touch-points (cheap because of the symbol-reference rule). Solution diffs re-apply against the new states. |
| 3 | **Redesign / wrong architecture** | Ship a fix-forward lesson at the tip ("refactor is curriculum"), or wait for an edition rewrite of the affected volume. Frozen tags never move for a redesign. |

*(codebase-continuity: Three-class revision policy)*

**Learner resync** — documented from the first lesson. When a tag moves under a
learner (class-2 fix), they restore their working tree to the current lesson
state with:

```
git checkout lesson-NNN -- src/
```

*(codebase-continuity: Learner resync path)*

## 4. Exercises and solutions

Density by part *(exercises: Density varies by part)*:

| Part | Exercises per lesson | Shape |
| ---- | -------------------- | ----- |
| Part 0 | 3-4 | short drills |
| Parts 1-2 | 1-2 | "make it yours" extensions |
| Parts 3-4 | 1-2 | mixed exercises |
| Part 5 | fewer, larger — at most 2 | bigger challenges |

- Every exercise fits **exactly one of six archetypes**: predict-the-output,
  fix-the-crash, extend-the-code, measure-the-performance, explain-in-prose,
  port-to-your-own-machine.
  *(exercises: Bounded archetypes)*
- **Exercises are written before the prose** they accompany. If no suitable
  exercise can be drafted, the lesson is mis-sized: split or merge it before
  publication. A lesson is never published without its exercises.
  *(lesson-format: Exercises gate publication)*
- A solution ships as **a code diff against the lesson's end state plus a short
  prose walkthrough** — never a full listing: the patch at
  `book/solutions/lesson-NNN/exN.patch`, the walkthrough at
  `book/solutions/lesson-NNN/exN.md`. Diffs survive class-2 revisions: they
  re-apply against the re-tagged lesson states.
  *(exercises: Solutions are diffs with walkthroughs)*
- **The prompt never contains its solution.** Each prompt ends with the link to
  its solution walkthrough — after the prompt's end, never before it.
  *(exercises: Solutions stay out of the prompt)*

## 5. C++ subset admission policy

- **Part 0 teaches C.** The engine, born at Part 1, is C++.
- A language feature is **admitted only if we can explain what it compiles down
  to**. The admitted subset: references, overloading, namespaces, `constexpr`,
  and classes with vtables. Templates appear only in clearly labeled tooling
  code.
- Each feature is admitted in the lesson that explains its code generation, and
  the subset is documented in the first engine lesson.
- The policy is **enforced by review, not tooling**.

*(curriculum: C++ subset transition at Part 0's end; design D1)*
