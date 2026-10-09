# Spec Delta

## Purpose

Defines the hero's movement — eight directions with accel/decel feel — so the character the player drives reads as a thing with weight rather than a position that jumps.

## ADDED Requirements

### Requirement: Eight-direction movement
The hero SHALL move in eight directions from polled input state, at the speed its row states, and SHALL travel at the same speed diagonally as in a single direction.

#### Scenario: Eight directions from input state
- **WHEN** the player holds one direction or two perpendicular directions
- **THEN** the hero moves the way the held directions ask, and diagonally at the same speed as straight

#### Scenario: The row's speed is the hero's speed
- **WHEN** the hero's speed is changed in its table row and the game runs again
- **THEN** the hero moves at the changed speed without a rebuild

### Requirement: Accel/decel feel
The hero's motion SHALL ease into and out of movement: acceleration from rest toward full speed, and deceleration to rest when the player lets go, without a step change in velocity.

#### Scenario: Accelerating from rest
- **WHEN** the player holds a direction while the hero is at rest
- **THEN** the hero's speed rises over subsequent frames to its full speed rather than arriving there at once

#### Scenario: Decelerating to rest
- **WHEN** the player releases the direction
- **THEN** the hero's speed falls over subsequent frames to rest rather than stopping at once

#### Scenario: Turning is felt
- **WHEN** the player reverses direction while moving
- **THEN** the speed passes through the ease rather than snapping to full speed the other way
