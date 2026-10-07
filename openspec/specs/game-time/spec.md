# game-time Specification

## Purpose

Defines game-time — the scaled clock the simulation advances by — so pause and hitstop are one knob the game sets rather than special cases threaded through the update.

## Requirements

### Requirement: The game-time scale
The engine SHALL advance the simulation by a game-time step derived from the platform clock and a scale factor the game sets, where the scale runs from 0 to full speed.

#### Scenario: Full speed
- **WHEN** the scale is at full speed
- **THEN** game-time advances with the platform clock

#### Scenario: Pause
- **WHEN** the scale is set to 0
- **THEN** the simulation does not advance while the scale holds at 0, and the run keeps drawing and presenting

#### Scenario: Hitstop
- **WHEN** the scale is set to a fraction of full speed
- **THEN** the simulation advances at that fraction until the scale is set again

#### Scenario: The scale is a value the game owns
- **WHEN** the game sets the scale during a frame
- **THEN** the frame's game-time step reflects the value in force for that frame

### Requirement: Measurement is not scaled
The scale SHALL apply to the simulation's step and to nothing else: the platform clock remains the measurer, and the frame record's phases stay wall-clock durations regardless of the scale.

#### Scenario: The frame record is wall-clock at any scale
- **WHEN** frames are recorded with the scale at 0 or at a fraction
- **THEN** each phase's recorded duration is the wall-clock time that phase took

#### Scenario: The scale does not reach the seam
- **WHEN** the game changes the scale
- **THEN** no platform-layer call or contract changes as a result
