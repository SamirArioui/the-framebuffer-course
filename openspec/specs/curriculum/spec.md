# curriculum Specification

## Purpose

Defines the ordered six-part learning arc from C foundations to a finished game, so every part has a clear goal and later parts' needs constrain earlier parts before their authoring begins.

## Requirements

### Requirement: Six-part sequential arc
The course SHALL present content in six sequential parts: Part 0 C foundations, Part 1 platform layer, Part 2 software rendering, Part 3 sound, Part 4 services, Part 5 the game. Every concept a lesson uses SHALL be introduced in that lesson or an earlier one.

#### Scenario: Ordered completion
- **WHEN** a learner finishes a part's lessons in order
- **THEN** the next part's lessons require no material from parts beyond the one before it

#### Scenario: No forward dependencies
- **WHEN** a lesson is authored
- **THEN** every concept it uses is introduced in that lesson or an earlier one

### Requirement: Throwaway Part 0 sandbox
Part 0 SHALL teach C through four standalone sandbox programs — `wordcount`, a data-structures kit (`dynarray` + `hashtable`), `paint` (a BMP/PPM painter), and `snek` (a terminal game) — in that order. These programs SHALL build independently of the engine and be discarded after Part 0.

#### Scenario: Sandbox independence
- **WHEN** a Part 0 program is built
- **THEN** it compiles standalone with no engine sources

#### Scenario: Engine born at Part 1
- **WHEN** Part 1 lesson 1 begins
- **THEN** the engine codebase starts from an empty state and engine files are authored in the admitted C++ subset

### Requirement: C++ subset transition at Part 0's end
Part 0's final lesson SHALL introduce the admitted C++ subset — references, overloading, namespaces, constexpr, and classes with vtables — so learners can read and write it before the engine begins.

#### Scenario: Subset readiness
- **WHEN** a learner completes Part 0
- **THEN** they can read and write a small program using the admitted subset features

### Requirement: Deep dives embedded in their home parts
The curriculum SHALL embed four mandatory deep dives in their designated parts: compiler/linker internals in Part 0, virtual memory in Part 1, and caches plus SIMD/assembly reading in Part 2, each revisited during Part 5's optimization passes.

#### Scenario: Dive placement
- **WHEN** the arc is planned
- **THEN** each mandatory deep dive has a designated lesson home inside its part

#### Scenario: Optimization revisits the dives
- **WHEN** Part 5 fixes a measured hotspot
- **THEN** the fix explicitly applies a technique from the caches or SIMD/assembly deep dive

### Requirement: Backward propagation from Part 5
Part 5's lesson skeleton and the frozen target-game contract SHALL exist before Part 1 is authored, and Parts 1-4 SHALL deliver the capabilities those needs require.

#### Scenario: Skeleton first
- **WHEN** Part 1 authoring begins
- **THEN** the Part 5 lesson skeleton (L1-L22) and the frozen game checklist already exist as planning artifacts

#### Scenario: Capstone gates Part 4
- **WHEN** Part 4 completes
- **THEN** its closing vertical-slice lesson can draw a hero walking a tilemap with a following camera using only finished services

### Requirement: Part 5 lesson cap
Part 5 SHALL contain at most 22 lessons (L1-L22): game assembly (L1-L15), one debt/refactor lesson (L16), the three optimization passes (L17-L20), and closing retrospective and challenge lessons (L21-L22).

#### Scenario: Cap enforced
- **WHEN** new content is proposed for Part 5
- **THEN** it replaces existing scope or moves to extras instead of exceeding 22 lessons

### Requirement: Toolchain is curriculum
The course SHALL teach its build and debugging toolchain explicitly as lesson content: compiler flags and the debugger in Part 0, sanitizers where memory bugs and undefined behavior are taught, and the profiler before optimization work begins. The toolchain SHALL NOT be a hidden prerequisite learners are expected to already trust.

#### Scenario: Debugger taught early
- **WHEN** a learner works through Part 0's first sandbox program
- **THEN** the debugger is taught explicitly as lesson content, not assumed

#### Scenario: Sanitizers make bugs visible
- **WHEN** a lesson teaches memory bugs or undefined behavior
- **THEN** the lesson makes them visible with sanitizer tooling

#### Scenario: Flags are explained, not pasted
- **WHEN** a lesson's build command introduces or changes compiler flags
- **THEN** the lesson explains what those flags do

#### Scenario: Profiler precedes optimization
- **WHEN** optimization work begins
- **THEN** the profiler has already been taught as lesson content
