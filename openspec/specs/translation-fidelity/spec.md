# translation-fidelity Specification

## Purpose

Defines the honesty contract for translated course content: every translated page is answerable to a named English source and revision, and everything the learner types, runs, or reads as code is identical across editions.

## Requirements

### Requirement: Source marker on every translated page
Each translated page SHALL name the English source page and the English revision its translation follows, in a machine-readable marker.

#### Scenario: Marker present
- **WHEN** a translated page is published
- **THEN** it names its English source page and the English revision it follows

#### Scenario: Marker is machine-readable
- **WHEN** the translation check runs
- **THEN** it reads each translated page's declared source page and revision without human interpretation

### Requirement: Code is identical across editions
Code blocks, build commands, symbol names, file names, and exercise patches in a translated page SHALL be byte-identical to those of its English source page.

#### Scenario: Code steps match
- **WHEN** a translated lesson and its English source are compared
- **THEN** every fenced code block in the translated page is byte-identical to the corresponding block in the source

#### Scenario: Patches match
- **WHEN** a translated solution page is published
- **THEN** the diff it presents is byte-identical to the diff its English source presents

#### Scenario: Divergence is caught
- **WHEN** a code block or patch in a translated page differs from its English source
- **THEN** the translation check fails and names the page and block

### Requirement: Drift is detected mechanically
A translation check SHALL report every translated page whose English source has changed after the revision the page declares as its source.

#### Scenario: Stale translation reported
- **WHEN** an English page is edited after a translation declared its source revision
- **THEN** the check reports that translated page as behind, naming the page and the revisions

#### Scenario: Current translation passes
- **WHEN** every translated page declares the revision its English source stands at
- **THEN** the check reports no drift

### Requirement: Terminology follows the stated conventions
Translated prose SHALL follow the project's translation conventions: what is translated, which code-adjacent terms are kept in English, and how the two editions refer to the same course objects.

#### Scenario: Conventions are written down
- **WHEN** a translation batch begins
- **THEN** the terminology and translation rules it follows are stated in a planning document the batch can be checked against

#### Scenario: Code-adjacent terms stay in English
- **WHEN** prose names a symbol, a tool, a flag, or a command
- **THEN** that name appears in English in the translation, unchanged
