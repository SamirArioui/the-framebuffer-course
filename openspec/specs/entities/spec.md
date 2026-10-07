# entities Specification

## Purpose

Defines live entity storage — entities created from the game's data-driven definitions, carrying the facts the game acts on, iterated every frame and retired when done — so the game holds many of them without inventing storage per feature.

## Requirements

### Requirement: Entity storage
The engine SHALL hold live entities in one fixed-capacity store decided up front, each entity carrying the facts the game acts on. Creating an entity SHALL NOT allocate memory while the game runs.

#### Scenario: An entity is created from a definition
- **WHEN** the game asks for an entity of a definition the table holds
- **THEN** one exists carrying that definition's identity and attributes

#### Scenario: Capacity is a decision, not an event
- **WHEN** the store has no free slot
- **THEN** the request is answered by the store's documented policy rather than by allocating further memory

#### Scenario: Creating an entity allocates nothing
- **WHEN** entities are created while the game runs
- **THEN** the engine's memory use does not change

### Requirement: Iteration
The game SHALL be able to walk every live entity once per frame and read the facts it acts on, so per-entity work is expressed once rather than per type.

#### Scenario: Every live entity is visited exactly once
- **WHEN** the game walks the store
- **THEN** each live entity is visited once and no retired or empty slot is visited

#### Scenario: An entity retired during the walk
- **WHEN** an entity is retired while the walk is in progress
- **THEN** it is not visited again in that walk and no other entity is skipped

### Requirement: Lifetime
An entity SHALL be retireable, and its slot reusable — the store's live count is what the game reads, not the number of slots ever filled.

#### Scenario: A retired entity is gone
- **WHEN** the game retires an entity
- **THEN** later walks do not visit it and the live count falls

#### Scenario: A freed slot is reused
- **WHEN** an entity is created after one was retired
- **THEN** it occupies a freed slot before any never-used slot is taken

### Requirement: Entities move against the world
An entity's position SHALL be movable by the game and resolvable against the tilemap's collision queries, so a moving entity stops at walls using the same queries the hero uses.

#### Scenario: Movement is blocked by solid tiles
- **WHEN** an entity is moved into a solid tile
- **THEN** the move that would enter it does not happen and the movement along the wall still does

#### Scenario: Movement is free where nothing is solid
- **WHEN** an entity is moved into empty space
- **THEN** it arrives at the requested position
