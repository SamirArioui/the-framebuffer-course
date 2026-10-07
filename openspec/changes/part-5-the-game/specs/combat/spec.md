# Spec Delta

## Purpose

Defines combat — the two weapon types and the projectiles they fire — so the game's fights are driven by data the tables carry and resolved by the engine's entity services.

## ADDED Requirements

### Requirement: Two weapon types
The game SHALL have exactly two weapon types, each defined by a row of the archetype tables, each firing the projectile kind its row names.

#### Scenario: A weapon fires its own kind
- **WHEN** the player fires a weapon
- **THEN** the projectile created is the kind that weapon's row states, carrying that row's values

#### Scenario: The weapons are data
- **WHEN** a weapon's row is changed in the table and the game runs again
- **THEN** the fired projectiles carry the changed values without a rebuild

### Requirement: Projectile lifetime
A projectile SHALL travel while the game advances and end when its life is over: at a solid tile, at the end of its range, or at the entity it hit. Its slot SHALL return to the store when it ends.

#### Scenario: A projectile ends at a wall
- **WHEN** a projectile is moved into a solid tile
- **THEN** it is retired rather than entering the tile

#### Scenario: A projectile advances in game time
- **WHEN** the game-time scale is 0
- **THEN** no projectile moves, and at a fraction they move at that fraction

### Requirement: Hits resolve against health
A projectile that hits an entity SHALL retire and reduce that entity's health by its damage; an entity whose health reaches zero SHALL be retired.

#### Scenario: A hit is resolved
- **WHEN** a projectile reaches an entity
- **THEN** the projectile is retired and the entity's health falls by the projectile's damage

#### Scenario: A defeated entity is retired
- **WHEN** an entity's health reaches zero
- **THEN** it is retired and no later walk visits it
