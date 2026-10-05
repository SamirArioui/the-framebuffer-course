# target-game Specification

## Purpose

Defines the frozen contract for the finished game so Part 5 has an unambiguous definition of done and scope cannot expand mid-course.

## Requirements

### Requirement: Frozen feature checklist
The finished game SHALL implement exactly these features: hero 8-direction movement with accel/decel; projectiles with 2 weapon types; 3 enemy types and 1 boss using chase, keep-distance, and flee behaviors; a single scrolling tilemap with tile collision; title, play, pause, death, and victory states; the juice toolkit; and music plus sfx through the engine's own mixer.

#### Scenario: Checklist completion
- **WHEN** the game is declared complete
- **THEN** every listed feature is implemented and demonstrable

#### Scenario: Out-of-scope ideas are recorded, not added
- **WHEN** a feature idea outside the checklist arises during Part 5
- **THEN** it is recorded as extras or post-course material instead of entering the part's scope

### Requirement: Feel toolkit bounded
Game-feel treatment SHALL be limited to four effects — hitstop, screenshake, particle bursts, and easing — grounded in the principle of immediate and readable feedback.

#### Scenario: Juice scope
- **WHEN** a feel effect is added to the game
- **THEN** it is one of the four toolkit effects

### Requirement: Optimization follows the three-pass menu
Part 5's optimization work SHALL perform exactly three passes: measure and name the top-2 hotspots; fix those two hotspots; produce the final frame-budget report.

#### Scenario: Fixes target measured hotspots
- **WHEN** an optimization change is made
- **THEN** it addresses one of the two hotspots identified in the measure pass

#### Scenario: Course finale is the report
- **WHEN** Part 5 concludes
- **THEN** a frame-budget report accompanies the finished game

### Requirement: Performance definition of done
The finished game SHALL sustain 60 frames per second on modest hardware, and the final frame-budget report SHALL account for the per-frame time of each major subsystem.

#### Scenario: Frame budget met
- **WHEN** the finished game runs on the reference hardware
- **THEN** it sustains 60 fps

#### Scenario: Budget accounted
- **WHEN** the frame-budget report is produced
- **THEN** it attributes per-frame time to the major subsystems
