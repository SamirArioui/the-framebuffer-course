# audio-mixer Specification

## Purpose

Defines the mixer — per-channel playback of samples summed into the
engine's audio output — that music and sound effects route through, so
the game's sound is mixed by code we own.

## Requirements

### Requirement: Per-channel playback
The mixer SHALL play samples on a fixed set of channels, each channel carrying its playback position, volume, and loop flag. Starting playback on one channel SHALL NOT disturb the sounds playing on others.

#### Scenario: A channel plays its sample
- **WHEN** playback starts on a channel with a sample and a volume
- **THEN** the channel's output advances through the sample at the engine's rate and stops at the sample's end

#### Scenario: Channels play together
- **WHEN** playback is running on more than one channel
- **THEN** each channel advances independently and one channel's ending does not stop the others

#### Scenario: A looping channel continues
- **WHEN** a channel plays a sample with its loop flag set
- **THEN** playback continues from the sample's start at its end until the channel is stopped

### Requirement: The mix
Each output buffer the engine submits SHALL be the sum of the active channels' samples at their volumes, computed without wrapping: a sum outside the output format's range SHALL clamp to its limit. Idle or stopped channels SHALL contribute silence.

#### Scenario: The mix sums the active channels
- **WHEN** channels are playing while an output buffer is mixed
- **THEN** the buffer holds the sum of their samples at their volumes

#### Scenario: Overflow clamps instead of wrapping
- **WHEN** the channels' sum exceeds the output format's range
- **THEN** the sample clamps to the format's limit rather than wrapping to the opposite extreme

#### Scenario: Idle channels contribute silence
- **WHEN** a channel has no sound or its sample has ended
- **THEN** the mixed buffer is unchanged by that channel

### Requirement: Music and effects route as channels
The mixer SHALL serve long looping music and short one-shot effects through the same channel mechanism. When an effect starts and no channel is free, the mixer SHALL apply a defined reuse policy rather than failing silently.

#### Scenario: Music loops on its channel
- **WHEN** a music sample starts on a channel with looping set
- **THEN** it plays continuously while the run continues and can be stopped explicitly

#### Scenario: An effect plays as a one-shot
- **WHEN** an effect sample starts on a channel
- **THEN** it plays once to its end and its channel becomes free again

#### Scenario: A busy mixer has a defined answer
- **WHEN** an effect starts while every channel is playing
- **THEN** the mixer applies its documented reuse policy and the run continues
