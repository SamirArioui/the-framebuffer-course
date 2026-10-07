# entity-tables Specification

## Purpose

Defines the archetype table as loadable data — the data-driven definitions the game's entities are created from — so enemy types, weapons, and effects can be authored as files, changed without recompiling, and tested without running the game.

## Requirements

### Requirement: Table asset format
The engine SHALL load an entity table from a single file read whole: rows of definitions in a format the course defines by hand, parsed without a library. A load SHALL yield either a complete table or a typed failure — never partial data presented as success.

#### Scenario: A table loads completely
- **WHEN** the engine loads a table file that exists and is well-formed
- **THEN** every definition in it is available, with the values the file states

#### Scenario: A missing or malformed file is a typed failure
- **WHEN** the engine loads a path that does not exist or whose contents do not form a complete table in the engine's format
- **THEN** the failure is reported as a value and no partial table is used

#### Scenario: A row that disagrees with the format fails typed
- **WHEN** a row's values are not in the engine's format — a missing field, a value where a number is required, more fields than the format declares
- **THEN** the load fails as a typed failure rather than reading a value the file did not state

### Requirement: Definitions carry their facts
A loaded definition SHALL carry the facts creating an entity needs — its identity and its attributes — with its data, so an entity is answered from its definition alone and never from assumptions about the file.

#### Scenario: A definition's attributes come from the table
- **WHEN** an entity is created from a loaded definition
- **THEN** the entity carries the attributes the table states for that definition

#### Scenario: A definition the table does not hold is a typed failure
- **WHEN** the game asks for a definition the table does not contain
- **THEN** the request is reported as a value rather than producing an entity with assumed attributes

### Requirement: Tables are data, not code
Changing a table's values SHALL change the game's behavior without recompiling the engine, and the engine's own code SHALL contain no per-type table of entity attributes.

#### Scenario: A value edit changes behavior
- **WHEN** a table's attribute is changed in the file and the game is run again
- **THEN** entities created from that definition carry the changed value

#### Scenario: The engine holds no duplicate of the table
- **WHEN** engine source is inspected for the attributes the tables carry
- **THEN** no per-type copy of those values appears in code
