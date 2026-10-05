# Tasks

## 1. Part 0 authoring setup

- [ ] 1.1 Create the `sandbox/` layout for the four programs with a README stating the contract (each program builds standalone with no engine sources; the directory is discarded when Part 1's engine is born) per design D1-D2; verify `./build.sh` still reports the empty `src/` tree and `mdbook build` is unaffected.
- [ ] 1.2 Extend `README.md`'s repository layout and build documentation for `sandbox/` (one taught `gcc` command per program, no build abstraction) and verify every command it documents runs exactly as written.

## 2. P1 `wordcount` — the pipeline and the memory (lesson-001…006)

- [ ] 2.1 Author lesson-001 (argv and file input: the first program and its `gcc` command) — prose + one code step in `sandbox/wordcount/`, 3-4 drills written before the prose with diff solutions linked after each prompt; verify the program builds standalone with the lesson's command, the page renders with each solution link after its prompt, and the co-committed commit is tagged `lesson-001`.
- [ ] 2.2 Author lesson-002 (gdb taught as a skill: breakpoints, stepping, stack frames) with its drills and diff solutions; verify the debugging commands run as written against `sandbox/wordcount/`, the page renders correctly, and the commit is tagged `lesson-002`.
- [ ] 2.3 Author lesson-003 (char buffers: strings by hand) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-003`.
- [ ] 2.4 Author lesson-004 (malloc/free: growing buffers on the heap) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-004`.
- [ ] 2.5 Author lesson-005 (leaks made visible with sanitizers) with its drills and diff solutions; verify the sanitizer commands run as written and surface the leak, the page renders correctly, and the commit is tagged `lesson-005`.
- [ ] 2.6 Author lesson-006 (undefined behavior and buffer overflows) with its drills and diff solutions; verify the sanitizer run makes the overflow visible, the page renders correctly, and the commit is tagged `lesson-006`.

## 3. P2 `dynarray` + `hashtable` — layout and linkage (lesson-007…012)

- [ ] 3.1 Author lesson-007 (structs: sizeof, alignment, and padding surprises) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-007`.
- [ ] 3.2 Author lesson-008 (dynarray growth: realloc strategy and capacity) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-008`.
- [ ] 3.3 Author lesson-009 (function pointers: comparators and hooks) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-009`.
- [ ] 3.4 Author lesson-010 (genericity via `void*` and its pain) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-010`.
- [ ] 3.5 Author lesson-011 (hashtable on the kit: hashing, buckets, and lookup) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-011`.
- [ ] 3.6 Author lesson-012 (multi-file builds into the compiler/linker deep dive) with its drills and diff solutions; verify the multi-file build commands run as written, the deep dive covers translation units and linking, the page renders correctly, and the commit is tagged `lesson-012`.

## 4. P3 `paint` — bytes and pixels (lesson-013…018)

- [ ] 4.1 Author lesson-013 (raw bytes and pixel formats) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-013`.
- [ ] 4.2 Author lesson-014 (endianness and image-header layout) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-014`.
- [ ] 4.3 Author lesson-015 (fill-rect onto a memory buffer) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-015`.
- [ ] 4.4 Author lesson-016 (drawing lines onto the buffer) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-016`.
- [ ] 4.5 Author lesson-017 (writing a real image file by hand) with its drills and diff solutions; verify the program's output opens in an image viewer, the page renders correctly, and the commit is tagged `lesson-017`.
- [ ] 4.6 Author lesson-018 (the `-O2`/optimizer UB lesson) with its drills and diff solutions; verify the optimizer command runs as written and demonstrates the UB, the page renders correctly, and the commit is tagged `lesson-018`.

## 5. P4 `snek` — loops and state (lesson-019…024)

- [ ] 5.1 Author lesson-019 (the game loop) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-019`.
- [ ] 5.2 Author lesson-020 (`clock_gettime` timing) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-020`.
- [ ] 5.3 Author lesson-021 (raw-terminal input via escape codes) with its drills and diff solutions; verify the program builds standalone and reads terminal input as written, the page renders correctly, and the commit is tagged `lesson-021`.
- [ ] 5.4 Author lesson-022 (the double-buffered character grid) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-022`.
- [ ] 5.5 Author lesson-023 (the state machine: title, play, death) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-023`.
- [ ] 5.6 Author lesson-024 (the function-pointer command table) with its drills and diff solutions; verify the program builds standalone, the page renders correctly, and the commit is tagged `lesson-024`.

## 6. C++ subset transition (lesson-025)

- [ ] 6.1 Author lesson-025 converting part of `snek` in place — the command table becomes a class, the draw path a drawable interface with a vtable — covering every admitted subset feature in `plan/conventions.md` §5 (references, overloading, namespaces, `constexpr`, classes with vtables; templates never) with its drills and diff solutions; verify each admitted feature appears in the lesson's code step, the program still builds standalone as C++, the page renders correctly, and the commit is tagged `lesson-025`.

## 7. M1 boundary review and integration checks

- [ ] 7.1 Write `plan/part0-review.md` recording authoring velocity against the 10-20 h/week estimate, exercise counts per lesson against the conventions density table (3-4 drills), the gcc/gdb versions used, and a recommendation on freezing a first prefix; verify every Part 0 lesson is counted and every Part 0 density expectation is checked.
- [ ] 7.2 From a clean checkout following only `README.md`: build all four sandbox programs with their documented commands, run `mdbook build`, run `./build.sh`, and run `openspec validate`; verify all succeed and `git tag` shows consecutive `lesson-001`…`lesson-025` where each consecutive tag diff equals that lesson's code step.
