# `sandbox/` — Part 0's throwaway programs

Part 0 teaches C through four standalone programs that live here and nowhere
else:

| Program     | Directory        | What it is                              |
| ----------- | ---------------- | --------------------------------------- |
| `wordcount` | `wordcount/`     | A `wc`-like file counter                |
| `ds-kit`    | `ds-kit/`        | A data-structures kit (`dynarray` + `hashtable`) with a driver |
| `paint`     | `paint/`         | A BMP painter that writes real image files |
| `snek`      | `snek/`          | A terminal game                         |

The contract, stated once here and enforced by every lesson in Part 0:

- **Each program builds standalone with no engine sources.** Nothing under
  `sandbox/` includes anything from `src/`, and nothing from `src/` is needed
  to build or run these programs. The engine does not exist yet.
- **One taught `gcc` command per program — no build abstraction.** Each
  program compiles with a literal compiler command that its lessons grow flag
  by flag. There is deliberately no `sandbox/build.sh` and no Makefile: the
  compilation model is exactly what Part 0 exists to teach. The engine's
  `build.sh` at the repository root stays untouched and empty of sandbox code.
- **This directory is thrown away on purpose.** When Part 1's first lesson
  starts and the engine is born from a blank file, `sandbox/` is deleted in
  that lesson's code step. The `lesson-001` … `lesson-025` tags keep every
  state of this directory retrievable after that; resync a state with
  `git checkout lesson-NNN -- sandbox/`.

*(Design: D1 sandbox home and discard path, D2 one build command per program.)*
