# Spec Delta

## Purpose

Defines the game-state machine — title, play, pause, death, victory — so the game has five screens with their own input and their own transitions rather than one loop with flags.

## ADDED Requirements

### Requirement: The state machine
The game SHALL run in exactly one of five states — title, play, pause, death, victory — and each state SHALL own its screen and its input.

#### Scenario: One state at a time
- **WHEN** the game runs
- **THEN** exactly one state is active, and only that state's input acts

#### Scenario: Each state shows its own screen
- **WHEN** the game enters a state
- **THEN** that state's screen is what the window shows

### Requirement: Transitions are named
Transitions between states SHALL happen on named conditions: play begins from the title, pause suspends play and resumes it, death follows the hero's defeat, and victory follows the game's completion.

#### Scenario: Pause and resume
- **WHEN** the player pauses during play
- **THEN** the play state is suspended and the pause screen shows, and resuming continues play from where it stood

#### Scenario: Death and victory
- **WHEN** the hero's health reaches zero
- **THEN** the game enters the death state; and when the game's waves are complete, the victory state

#### Scenario: The simulation stands still outside play
- **WHEN** the game is not in the play state
- **THEN** the simulation does not advance while the presentation keeps drawing the state's screen
