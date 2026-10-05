# Spec Delta

## ADDED Requirements

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
