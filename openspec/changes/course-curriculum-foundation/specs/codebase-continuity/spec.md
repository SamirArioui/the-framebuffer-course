# Spec Delta

## Purpose

Defines how the engine repository's history carries lesson states so learners can follow the arc, diff lessons, and recover when published states change.

## ADDED Requirements

### Requirement: Linear history with lesson tags
The engine codebase SHALL keep one linear commit history in which each published lesson's end state is tagged `lesson-NNN` with zero-padded sequence numbers.

#### Scenario: Published lesson has a tag
- **WHEN** lesson NNN is published
- **THEN** a `lesson-NNN` tag marks its end-of-lesson code state

#### Scenario: Diffing across lessons
- **WHEN** a learner diffs two lesson tags
- **THEN** the diff equals the code steps of the lessons between them

### Requirement: Stability horizon
The published course SHALL declare a frozen prefix of stable lessons and a volatile tail. Tags within the frozen prefix SHALL NOT move except for behavior-changing correctness fixes.

#### Scenario: Horizon is published
- **WHEN** the course is published
- **THEN** learners can see which lessons are frozen and which may change

#### Scenario: Frozen tags resist redesigns
- **WHEN** a proposed redesign would move a frozen tag
- **THEN** it is routed to a fix-forward lesson or an edition rewrite instead

### Requirement: Three-class revision policy
Changes to past content SHALL follow the revision policy: prose-only changes edit in place; behavior-changing code fixes in the frozen prefix apply surgically with rebase-forward and re-tagging; architectural redesigns become fix-forward lessons at the tip or wait for an edition rewrite.

#### Scenario: Prose fix
- **WHEN** a typo or clarity issue is found in a frozen lesson's prose
- **THEN** the prose is edited in place and no tag moves

#### Scenario: Surgical code fix
- **WHEN** a behavior-changing bug is found in the frozen prefix
- **THEN** downstream lesson states are rebased forward, re-tagged, and affected prose references are updated

#### Scenario: Redesign
- **WHEN** an architectural flaw is discovered
- **THEN** the correction ships as an explicit fix-forward lesson at the tip or defers to an edition rewrite

### Requirement: Learner resync path
The course SHALL document a recovery procedure, available from the first lesson, for learners whose lesson state moved after a surgical fix.

#### Scenario: Learner recovers
- **WHEN** a learner's copied state diverges because a tag moved
- **THEN** the documented resync procedure restores their working tree to the current lesson state
