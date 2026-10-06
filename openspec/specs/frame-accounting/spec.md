# frame-accounting Specification

## Purpose

Defines the per-frame measurement the engine keeps — what each frame's
phases cost, recorded and logged in a stable format — that the profiler
lesson and the final frame-budget report both read.

## Requirements

### Requirement: Per-frame record
The engine SHALL measure every frame into a record of its phase durations — at least the world update, the render, and the presentation — plus the frame's total, all measured on the platform's monotonic clock. Every frame the run executes SHALL produce exactly one record.

#### Scenario: Every frame is recorded
- **WHEN** the engine completes a frame
- **THEN** one record exists holding that frame's update, render, presentation, and total durations

#### Scenario: The record's numbers are measured, not estimated
- **WHEN** a frame's phases are measured
- **THEN** each phase's duration is the elapsed time between the platform clock readings around that phase

### Requirement: Frame-time log
The run SHALL emit one log line per frame, in a stable text format naming the frame's number and its measured durations, and SHALL keep a running account of the frames measured so far. The account SHALL report at least the frame count, the average total, and the worst frame — the summary the final frame-budget report grows from.

#### Scenario: One line per frame
- **WHEN** the run completes N frames
- **THEN** the log contains exactly N frame lines in the recorded format

#### Scenario: The account summarizes the run
- **WHEN** the run ends
- **THEN** the account reports the frame count, the average frame duration, and the worst frame's duration with its frame number

### Requirement: Timing hooks stay behind the measurement boundary
Per-frame measurement SHALL read only the platform clock and the frame's own phase boundaries — the engine's measured numbers SHALL NOT depend on the OS-specific timing calls of any platform implementation.

#### Scenario: The measurement is platform-neutral
- **WHEN** the frame account is computed on any OS implementation of the platform seam
- **THEN** its numbers come from the seam's clock readings alone
