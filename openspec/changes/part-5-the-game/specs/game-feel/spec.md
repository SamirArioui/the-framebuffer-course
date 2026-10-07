# Spec Delta

## Purpose

Defines the juice toolkit — hitstop, screenshake, particle bursts, and easing — so the game's feedback is immediate and readable and stays inside the four effects the contract allows.

## ADDED Requirements

### Requirement: Feedback starts with the event
A feel effect SHALL begin in the frame its triggering event happens, so the player reads cause and effect as one moment.

#### Scenario: The effect and the event are one moment
- **WHEN** an event the toolkit answers (a hit, an impact, a death) happens in a frame
- **THEN** that frame already shows the effect

### Requirement: Hitstop and screenshake
Hitstop SHALL slow the simulation to a fraction of game time for a moment and then return it to full speed; screenshake SHALL move the camera's additive offset for a moment and return it to rest.

#### Scenario: Hitstop ends on its own
- **WHEN** a hit lands
- **THEN** the game-time scale drops to a fraction and returns to full speed without the game's intervention

#### Scenario: Screenshake rests at zero
- **WHEN** screenshake fires
- **THEN** the camera's additive offset moves and then settles at exactly zero

### Requirement: Particle bursts and easing
A particle burst SHALL spawn a bounded set of particles from the entity store that move, settle, and retire; easing SHALL shape animated values so they arrive rather than step.

#### Scenario: A burst is bounded and ends
- **WHEN** a burst fires
- **THEN** its particles take slots the store documents, move under game time, and retire when their life ends

#### Scenario: Eased values arrive
- **WHEN** a value animates under easing
- **THEN** its intermediate frames follow the ease and it ends exactly at its target
