# platform-layer Specification

## Purpose

Defines the observable contract of the engine's platform layer — the single
OS boundary (window, input state, timing, file I/O) every later part builds
on — so implementations can evolve and OS ports can be added without
touching engine code.

## Requirements

### Requirement: Window and presentation
The platform layer SHALL open a window of the size the engine requests and present the engine's pixel framebuffer in it, so the pixels the engine wrote are the pixels the window shows. A user-initiated close SHALL be reported to the engine and the run SHALL release its OS resources.

#### Scenario: Window opens
- **WHEN** the engine starts and requests a window size
- **THEN** a window of that size opens

#### Scenario: Pixels are presented unchanged
- **WHEN** the engine presents a framebuffer
- **THEN** the window displays the pixels the engine wrote

#### Scenario: Close is reported and resources are released
- **WHEN** the user closes the window
- **THEN** the platform layer reports it and the run ends with its OS resources released

### Requirement: Polled input state
The platform layer SHALL expose keyboard input as queryable state — which keys are down at the moment of the poll — maintained from OS input behind the interface. State SHALL reflect held keys as held and SHALL NOT require the engine to consume an event stream.

#### Scenario: Polling reports current state
- **WHEN** the engine polls input
- **THEN** it receives the current down/up state of the keys it tracks

#### Scenario: Held keys stay held
- **WHEN** a key is held across several polls
- **THEN** every poll reports that key as down

#### Scenario: Brief presses are not lost
- **WHEN** a key is pressed and released within one frame
- **THEN** the engine's next poll still observes that the key went down

### Requirement: Monotonic frame timing
The platform layer SHALL provide a monotonic clock whose readings never go backwards and whose resolution is fine enough to measure per-frame duration — the clock Part 2's frame-time instrumentation and Part 5's frame-budget report depend on. Wall-clock adjustments SHALL NOT affect it.

#### Scenario: Readings never go backwards
- **WHEN** the clock is sampled repeatedly
- **THEN** each reading is greater than or equal to the previous one even if the system wall clock is adjusted

#### Scenario: Frame durations are resolvable
- **WHEN** the engine measures the duration of one frame
- **THEN** the measurement distinguishes frame durations at sub-millisecond resolution

### Requirement: Whole-file I/O
The platform layer SHALL read and write whole files against the OS and report failure as a value: a read returns a file's complete bytes or a typed failure, never partial data presented as success; a write creates or replaces the file with exactly the bytes given.

#### Scenario: Successful read
- **WHEN** the engine reads a path that exists and is readable
- **THEN** it receives the file's complete bytes

#### Scenario: Failure is a reported value
- **WHEN** the engine reads a path that does not exist or cannot be opened
- **THEN** the failure is returned to the caller and no partial data is presented as success

#### Scenario: Write round-trips
- **WHEN** the engine writes bytes to a path and then reads that path back
- **THEN** the bytes read match the bytes written

### Requirement: Audio output
The platform layer SHALL open an audio output in the engine's fixed sample format and present the engine's mixed sample buffers to the device at the device's rate. The run's wait for news SHALL NOT outlast the output's need for samples, so the loop keeps the output fed without extra threads. A machine with no usable output SHALL be a typed failure.

#### Scenario: The output opens at the engine's format
- **WHEN** the engine requests an audio output
- **THEN** an output opens at the engine's sample rate and format, or the failure names the step that failed

#### Scenario: Submitted samples reach the device
- **WHEN** the engine submits a mixed buffer of samples
- **THEN** those samples are played at the output's rate before the buffer is reused

#### Scenario: The run's wait stays short enough to feed the output
- **WHEN** an output is open and the engine waits for news
- **THEN** the wait ends in time for the engine to submit the next buffer rather than starving the device

#### Scenario: A machine without an audio output fails typed
- **WHEN** the engine requests an audio output on a machine with no usable device
- **THEN** the failure is reported as a value naming that step and the run continues or ends by its own policy

### Requirement: Single OS boundary
Engine code outside the platform layer SHALL reach the OS only through the platform layer's interface, and that interface SHALL expose no OS-specific idioms, so a second OS implementation can replace the first without changing engine files. Student-visible code SHALL use no external libraries beyond the language's own and the OS surface the platform layer wraps.

#### Scenario: Engine code is OS-free
- **WHEN** a source file outside the platform layer is inspected
- **THEN** it contains no OS-specific calls or headers

#### Scenario: A second OS slots in
- **WHEN** an implementation for a different OS is added behind the interface
- **THEN** only platform-layer files change
