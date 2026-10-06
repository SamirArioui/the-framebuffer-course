# Spec Delta

## Purpose

Defines how the engine turns game state into pixels in its own framebuffer —
sprite, tilemap, and text drawing with clipping and transparency, and the
camera offsets that place the scene — so the renderer can grow and be
optimized without changing what the screen shows.

## ADDED Requirements

### Requirement: Sprite drawing
The engine SHALL draw a sprite's pixels into the framebuffer at a requested position, copying each source pixel exactly except the sprite's transparent color, which writes nothing. Drawing SHALL clip to the framebuffer: pixels whose destination falls outside are dropped, never wrapped into other pixels.

#### Scenario: Sprite pixels are drawn unchanged
- **WHEN** the engine draws a sprite fully inside the framebuffer
- **THEN** every non-transparent sprite pixel appears at its destination with the exact color the sprite carries

#### Scenario: Transparency writes nothing
- **WHEN** a sprite whose pixels include its transparent color is drawn over existing content
- **THEN** the transparent pixels leave the previous framebuffer contents unchanged

#### Scenario: Off-screen drawing is clipped
- **WHEN** a sprite is drawn with part of it outside the framebuffer
- **THEN** the pixels that fall inside appear and the rest are dropped

### Requirement: Bitmap text
The engine SHALL draw a string's characters as glyphs from a bitmap font into the framebuffer, each glyph at its position along the string's layout, using the same clipping and transparency rules as sprite drawing. A character the font does not provide SHALL be skipped without corrupting the layout of the following characters.

#### Scenario: Text is drawn from the font
- **WHEN** the engine draws a string the font provides
- **THEN** the glyphs appear in order at the requested position

#### Scenario: Missing characters do not corrupt layout
- **WHEN** the engine draws a string containing a character the font lacks
- **THEN** that character draws nothing and the following characters keep their positions

### Requirement: Tilemap drawing
The engine SHALL draw a tilemap's cells as tiles at their world positions, shifted by the camera offset, through the same drawing path as sprites. A map larger than the framebuffer SHALL draw correctly at any camera offset, with cells outside the framebuffer clipped.

#### Scenario: The map appears at the camera offset
- **WHEN** the engine draws a tilemap at a camera offset
- **THEN** each visible cell's tile appears at its world position minus the offset

#### Scenario: Scrolling keeps the map intact
- **WHEN** the camera moves across a map larger than the framebuffer
- **THEN** the visible region shows exactly the cells the camera covers

### Requirement: Camera offsets
The engine SHALL support a camera offset that places all scene drawing within the framebuffer, plus an additive offset applied on top of the base offset. Scene drawing SHALL use the sum of the two, so a base camera can scroll the world while a separate additive offset delivers feedback effects without disturbing the base.

#### Scenario: The base camera scrolls the scene
- **WHEN** the engine changes the base camera offset
- **THEN** all scene drawing shifts by the new offset

#### Scenario: The additive offset stacks
- **WHEN** the engine applies a non-zero additive offset over a base camera
- **THEN** drawing shifts by the sum of both offsets

#### Scenario: Clearing the additive offset restores the base view
- **WHEN** the additive offset returns to zero
- **THEN** the scene is drawn exactly at the base camera offset
