# audio-samples Specification

## Purpose

Defines sound as loadable data — the sample asset format and its
whole-file loading — so sound can be authored as files, loaded into the
engine's memory, and tested without an output device.

## Requirements

### Requirement: Sample asset format
The engine SHALL load a sound sample from a single file read whole: linear PCM sample data in a RIFF/WAVE container, in the format the course defines by hand. A load SHALL yield either a complete sample or a typed failure — never partial sample data presented as success.

#### Scenario: A sample loads completely
- **WHEN** the engine loads a sample file that exists and is well-formed
- **THEN** the loaded sample's length and every sample frame match the file's contents

#### Scenario: A missing or malformed file is a typed failure
- **WHEN** the engine loads a path that does not exist or whose contents do not form a complete sample in the engine's format
- **THEN** the failure is reported as a value and no partial sample is used

### Requirement: Samples carry their playback facts
A loaded sample SHALL carry the facts playback needs — its length in sample frames and its format — with its data, so playback and mixing are answered from the sample alone and never from assumptions about the file.

#### Scenario: Playback length comes from the sample
- **WHEN** a channel plays a loaded sample
- **THEN** it plays exactly the sample's frame count and stops at the sample's end unless looping

#### Scenario: A sample in another format fails typed
- **WHEN** a file's sample data is not the engine's format
- **THEN** the load fails as a typed failure rather than playing it misread
