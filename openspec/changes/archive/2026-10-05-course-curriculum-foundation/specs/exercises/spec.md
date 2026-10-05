# Spec Delta

## Purpose

Defines the exercise and solution system so learners can verify their own understanding while the authoring cost of a multi-hundred-exercise course stays bounded.

## ADDED Requirements

### Requirement: Density varies by part
Each lesson SHALL carry exercises at its part's density: Part 0: 3-4 short drills; Parts 1-2: 1-2 extensions; Parts 3-4: 1-2 mixed exercises; Part 5: fewer, larger challenges.

#### Scenario: Part 0 drill count
- **WHEN** a Part 0 lesson is completed
- **THEN** it carries 3-4 short drill exercises

#### Scenario: Part 5 challenge count
- **WHEN** a Part 5 lesson is completed
- **THEN** it carries at most 2 larger challenges

### Requirement: Bounded archetypes
Every exercise SHALL match one of six recurring archetypes: predict-the-output, fix-the-crash, extend-the-code, measure-the-performance, explain-in-prose, or port-to-your-own-machine.

#### Scenario: Archetype match
- **WHEN** an exercise is drafted
- **THEN** it fits one of the six named archetypes

### Requirement: Solutions are diffs with walkthroughs
Each exercise's solution SHALL ship as a code diff against the lesson's end state plus a short prose walkthrough, not as a full code listing.

#### Scenario: Solution format
- **WHEN** a learner opens a solution
- **THEN** it shows a patch against the lesson state and a brief explanation

#### Scenario: Solutions survive revisions
- **WHEN** a surgical fix re-tags lesson states
- **THEN** the same solution diffs apply against the new states

### Requirement: Solutions stay out of the prompt
Exercise prompts SHALL present the task without its solution inline; solutions are presented separately after the learner's attempt.

#### Scenario: Spoiler-free attempt
- **WHEN** a learner reads an exercise prompt
- **THEN** no solution content appears before the prompt's end
