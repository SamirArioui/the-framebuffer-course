# Spec Delta

## ADDED Requirements

### Requirement: Audio output
The platform layer SHALL open an audio output in the engine's fixed sample format and present the engine's mixed sample buffers to the device at the device's rate. The run's wait for news SHALL NOT outlast the output's need for samples, so the loop keeps the output fed without extra threads. A machine with no usable output SHALL be a typed failure.

#### Scenario: The output opens at the engine's format
- **WHEN** the engine requests an audio output
- **THEN** an output opens at the engine's sample rate and format, or the failure names the step that failed

#### Scenario: Submitted samples reach the device
- **WHEN** the engine submits a mixed buffer of samples
- **THEN** those samples are played at the output's rate before the buffer is reused

#### Scenario: The run's wait stays short enough to feed the output
- **WHEN** an output is open and the engine waits for news
- **THEN** the wait ends in time for the engine to submit the next buffer rather than starving the device

#### Scenario: A machine without an audio output fails typed
- **WHEN** the engine requests an audio output on a machine with no usable device
- **THEN** the failure is reported as a value naming that step and the run continues or ends by its own policy
