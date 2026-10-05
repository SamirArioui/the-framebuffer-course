# Spec Delta

## Purpose

Defines the locally runnable course website that presents lessons, exercises, and solutions and tells learners which content is stable.

## ADDED Requirements

### Requirement: Local operation
The course site SHALL build and run locally from the repository so the author and any learner can browse the full course on their own machine.

#### Scenario: Local browsing
- **WHEN** the documented local-run command is executed
- **THEN** the course site is browsable on the local machine

### Requirement: Lesson navigation
The site SHALL present lessons in curriculum order and provide per-lesson navigation to the previous lesson, the next lesson, the containing part, and the lesson's code tag.

#### Scenario: Adjacent navigation
- **WHEN** a learner views a lesson
- **THEN** links to the previous lesson, the next lesson, its part, and its code tag are available

### Requirement: Exercise and solution presentation
The site SHALL show each lesson's exercises and link their diff-based solutions from the exercise section.

#### Scenario: Solution linkage
- **WHEN** a learner reaches the end of an exercise prompt
- **THEN** a link to that exercise's solution walkthrough is available

### Requirement: Stability horizon display
The site SHALL state the current stability horizon, naming which lessons are frozen and which remain subject to change.

#### Scenario: Horizon visible
- **WHEN** any page of the site renders
- **THEN** the current frozen prefix is stated

### Requirement: Static publication
The site SHALL build into self-contained static output deployable to any static file host.

#### Scenario: Static build
- **WHEN** the site build command runs
- **THEN** it produces a directory of static files requiring no server-side runtime
