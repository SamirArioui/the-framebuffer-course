# localized-site Specification

## Purpose

Defines the course site's second language edition: a parallel French book that builds, publishes, and navigates beside the English edition, so a French reader has a complete path through the same course.

## Requirements

### Requirement: Parallel edition
The site SHALL publish a French edition of the course as a parallel book whose page paths mirror the English edition's paths, and the English edition's source tree SHALL NOT be altered to produce it.

#### Scenario: Mirrored counterpart
- **WHEN** a page exists in the French edition
- **THEN** its English counterpart sits at the mirrored path within the English edition, and vice versa

#### Scenario: English tree untouched
- **WHEN** the French edition is authored or built
- **THEN** the English edition's pages, code tags, and published URLs are unchanged

### Requirement: Editions publish side by side
The published static output SHALL hold both editions at their own paths — the English edition at the published root and the French edition under `fr/` — from the documented publication procedure.

#### Scenario: Both editions in one publication
- **WHEN** the documented build and publication procedure runs
- **THEN** the output contains the complete English edition and the complete French edition, each browsable from its own root

#### Scenario: English URLs stay stable
- **WHEN** the site is republished after the French edition is added
- **THEN** every previously published English page URL still resolves to that page

### Requirement: Local operation per edition
Each edition SHALL build and run locally by its own documented command, so the author and any learner can browse either edition on their own machine.

#### Scenario: French edition browsable locally
- **WHEN** the documented French local-run command is executed
- **THEN** the French edition is browsable on the local machine

#### Scenario: Independent local runs
- **WHEN** one edition is being served locally
- **THEN** the other edition's build output is not required to be present

### Requirement: Language switching
Every page of each edition SHALL offer a switch to the same page in the other edition, working at every navigation depth.

#### Scenario: Counterpart switch
- **WHEN** a learner is on any page of either edition
- **THEN** a switch to that page's counterpart in the other edition is available

#### Scenario: Deep pages switch correctly
- **WHEN** the switch is used from a lesson or solution page several levels deep
- **THEN** it opens the counterpart page, not the other edition's home page

### Requirement: Mirrored navigation
The French edition SHALL present lessons in curriculum order with the same per-lesson navigation and exercise-to-solution linkage as the English edition.

#### Scenario: Curriculum order
- **WHEN** a learner opens the French table of contents
- **THEN** lessons appear in the same curriculum order as the English edition, with the same part structure

#### Scenario: Solution linkage
- **WHEN** a learner reaches the end of an exercise prompt in the French edition
- **THEN** a link to that exercise's French solution walkthrough is available

### Requirement: Stability horizon in French
Every page of the French edition SHALL state, in French, the same stability horizon as the English edition: the same frozen prefix and the same volatile tail.

#### Scenario: Banner present
- **WHEN** any French page renders
- **THEN** the current frozen prefix is stated in French

#### Scenario: Horizon agrees across editions
- **WHEN** the English horizon names a frozen range
- **THEN** the French pages name that same range
