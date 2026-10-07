# Spec Delta

## Purpose

Defines the HUD — the play screen's score, health, and timers — so the player can read the game's state at a glance while the world runs behind it.

## ADDED Requirements

### Requirement: The play screen's readouts
The play screen SHALL show the score, the hero's health, and the game's timers, each reflecting the game's actual state.

#### Scenario: The readouts match the game
- **WHEN** the score, health, or a timer changes in the game
- **THEN** the HUD shows the changed value in the same frame

#### Scenario: Health shows what the hero has
- **WHEN** the hero's health falls or is restored
- **THEN** the health readout reflects it

### Requirement: Readable over the world
The HUD SHALL be drawn over the scene and SHALL NOT move with the camera, so it stays legible wherever the world scrolls.

#### Scenario: The HUD does not scroll
- **WHEN** the camera moves across the map
- **THEN** the HUD stays in place on screen, drawn over the world
