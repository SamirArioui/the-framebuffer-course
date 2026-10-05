# lesson-format Specification

## Purpose

Defines the anatomy and authoring rules of a lesson so prose, code steps, and exercises stay mutually consistent within a predictable student time budget.

## Requirements

### Requirement: Lesson anatomy
Each lesson SHALL contain prose, exactly one code step, and its exercises. A lesson SHALL target 30-60 minutes of student read-and-code time.

#### Scenario: Complete lesson
- **WHEN** a learner opens a published lesson
- **THEN** it presents prose, the code change to apply, and its exercises

#### Scenario: Time budget
- **WHEN** a lesson is drafted
- **THEN** its expected student effort is between 30 and 60 minutes

### Requirement: Prose and code co-committed
Each lesson's prose and its code step SHALL be committed together in the same change so a lesson tag identifies both.

#### Scenario: Tag completeness
- **WHEN** a learner checks out a lesson's tag
- **THEN** both the lesson's prose and its end-of-lesson code state are present

### Requirement: Symbol-reference discipline
Lesson prose SHALL reference code only by symbol names and structural descriptions, never by line numbers.

#### Scenario: Code shifts do not break prose
- **WHEN** a code change shifts line positions in files a later lesson describes
- **THEN** that lesson's prose remains accurate without edits

#### Scenario: References name symbols
- **WHEN** prose directs attention to code
- **THEN** it names a symbol or a structural location rather than a line number

### Requirement: Exercises gate publication
A lesson SHALL NOT be published before its exercises exist; the exercise draft defines the lesson's scope and acts as its acceptance test.

#### Scenario: Unfinished lesson
- **WHEN** a lesson has prose and code but no exercises
- **THEN** it remains unpublished

#### Scenario: Mis-sized lesson
- **WHEN** no suitable exercise can be drafted for a lesson
- **THEN** the lesson is split or merged before publication
