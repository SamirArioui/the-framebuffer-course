# Spec Delta

## Purpose

Defines the tilemap as loadable world data — the asset format the engine
reads from disk and the tile-level collision queries game logic runs
against it — so worlds can be authored as files and tested without a
screen.

## ADDED Requirements

### Requirement: Tilemap asset format
The engine SHALL load a tilemap from a single file read whole: the map's dimensions and one tile index per cell, in a format the course defines by hand. A load SHALL yield either a complete map or a typed failure — never a partial map presented as success.

#### Scenario: A map loads completely
- **WHEN** the engine loads a tilemap file that exists and is well-formed
- **THEN** the loaded map's dimensions and every cell match the file's contents

#### Scenario: A missing or malformed file is a typed failure
- **WHEN** the engine loads a path that does not exist or whose contents do not form a complete map
- **THEN** the failure is reported as a value and no partial map is used

### Requirement: Tile collision queries
The engine SHALL answer, for a position or rectangle in world coordinates, whether it overlaps solid tiles of a loaded map. Queries SHALL return the same answer for the same map and coordinates every time, and positions outside the map SHALL have a defined answer rather than undefined behavior.

#### Scenario: A solid tile reports overlap
- **WHEN** a query covers a cell marked solid
- **THEN** the query reports a collision

#### Scenario: An empty region reports no overlap
- **WHEN** a query covers only cells marked empty
- **THEN** the query reports no collision

#### Scenario: Out-of-bounds queries have a defined answer
- **WHEN** a query covers positions outside the map
- **THEN** the engine returns its defined out-of-bounds answer and does not read outside the map's data

### Requirement: Tile kinds carry solidity
A tilemap's format SHALL record, for each tile kind it names, whether that kind is solid for collision, so collision queries are answered from the map data alone and not from drawing code.

#### Scenario: Solidity comes from the map data
- **WHEN** two tile kinds differ in solidity and cells use them
- **THEN** collision queries distinguish those cells by the kinds' solidity alone
