# Spec Delta

## Purpose

Defines the enemies — three types, the boss, their behaviors, and the waves that bring them — so the game's opposition is data-driven rows following behaviors the game composes, not per-type code.

## ADDED Requirements

### Requirement: Enemy types from the table
The game's enemy types SHALL be rows of the archetype table, and every enemy entity SHALL carry the facts its row states.

#### Scenario: A wave's types come from the table
- **WHEN** a wave spawns its enemies
- **THEN** each one is created from a definition the table holds, carrying that definition's values

#### Scenario: Three types and the boss
- **WHEN** the game's enemy roster is examined
- **THEN** it holds exactly three enemy types and one boss, and no per-type copy of their attributes appears in code

### Requirement: Enemy behaviors
Every enemy SHALL move by one of the three behaviors — chase, keep-distance, flee — over the entity services, and its behavior SHALL read the game's state rather than run hidden per type.

#### Scenario: Chase
- **WHEN** an enemy with the chase behavior acts
- **THEN** it moves toward the hero through the mover, and stops at walls

#### Scenario: Keep-distance
- **WHEN** an enemy with the keep-distance behavior acts
- **THEN** it holds its distance from the hero, closing or withdrawing to keep it

#### Scenario: Flee
- **WHEN** an enemy with the flee behavior acts
- **THEN** it moves away from the hero through the mover

### Requirement: The boss
The boss SHALL be assembled from the same three behaviors plus a pattern of its own, and SHALL carry its own table row.

#### Scenario: The boss composes the behaviors
- **WHEN** the boss acts
- **THEN** its behavior is the shared behaviors plus its own pattern, with no separate movement machinery

### Requirement: Waves
The game SHALL bring its enemies in waves that combine the three types and the boss, and SHALL advance to the next wave when the current one is cleared.

#### Scenario: A wave brings its composition
- **WHEN** a wave begins
- **THEN** the types it names appear from the table's definitions

#### Scenario: A cleared wave advances
- **WHEN** the last entity of a wave is retired
- **THEN** the next wave begins and the game continues until the waves are done
