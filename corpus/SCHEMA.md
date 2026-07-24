# Corpus schema

One YAML file per case in `corpus/`. Filenames are kebab-case slugs.

A "case" is a single (problem, resolution event) pair. Where one technique resolved
several problems at once (e.g. Erdős #1196 / #1217 / #164), that is ONE case with a
`companion_problems` list — splitting it would triple-count a single mechanism and
corrupt the distribution table.

## Fields

```yaml
id:                  # slug, matches filename
title:               # human-readable name
date:                # YYYY-MM of the resolution event
status:              # RESOLVED | ADVANCED | DISPUTED | RETRACTED

problem_statement:   # informal, one paragraph, no LaTeX beyond ASCII math
field:               # e.g. Number Theory
subfield:            # e.g. Additive combinatorics

ai_system:           # model/system name(s) as reported
human_collaborators: # named humans, with role
autonomy:            # AUTONOMOUS | AI_LED_HUMAN_VERIFIED | COLLABORATIVE | AI_ASSISTED
                     # see AUTONOMY LADDER below

resolution_mechanism:
  primary:           # exactly one of:
                     #   LITERATURE_RECALL   - surfaced existing work that already answered it
                     #   CONSTRUCTION        - explicit witness, counterexample, improved bound
                     #   NOVEL_ARGUMENT      - short new proof not reducible to (a) or (b)
                     #   SEARCH_PLUS_VERIFIER- iterated generation against automated checking
  secondary: []      # zero or more of the same vocabulary
  contested:         # true if the primary classification is genuinely arguable; explain in notes

certificate_size:    # SMALL | MEDIUM | LARGE
                     #   SMALL  - a skeptic verifies in minutes (explicit object, or machine-checked)
                     #   MEDIUM - days (short human paper, or a proof needing one expert reading)
                     #   LARGE  - months (long paper, refereeing, new definitions to absorb)
certificate_notes:   # what the certificate actually is

verification_method: # LEAN | COMPUTER_ALGEBRA | HUMAN_REFEREEING | EXPERT_INFORMAL | NONE_YET
verification_status: # what has actually happened as of the corpus date, honestly

cross_field_transfer:
  occurred:          # true/false
  from:              # source field, if any
  to:                # destination field
  notes:

sources:             # every claim above must be traceable to one of these
  - url:
    what:            # what this source establishes
    tier:            # PRIMARY (paper//official) | EXPERT_COMMENTARY | JOURNALISM | SOCIAL_MEDIA

unverifiable_claims: []  # anything asserted in sources that this corpus does NOT vouch for
notes:               # judgment calls, ambiguity about human vs model contribution
```

## Autonomy ladder

Deliberately separate from `resolution_mechanism`. The mechanism is *what kind of
mathematical object closed the problem*; autonomy is *who produced it*. Conflating
them is the main way the public record on this topic goes wrong.

- `AUTONOMOUS` — model produced the mathematical content end to end; humans posed
  the problem and checked the answer, nothing more.
- `AI_LED_HUMAN_VERIFIED` — model produced the key idea and most of the argument;
  humans verified, digested, and wrote it up.
- `COLLABORATIVE` — model produced a usable seed (a suggestion, a numeric pattern,
  a lemma); humans built the result around it. Removing the humans kills the result.
- `AI_ASSISTED` — model did supporting work (formalization, literature, computation)
  on a result whose mathematical content is human.

## Tao-wiki cross-reference

The `teorth/erdosproblems` wiki runs its own taxonomy, which this corpus records in
`notes` where applicable so the two can be reconciled:

- 1(a) AI standalone, no comparable literature found
- 1(b) AI alongside literature, comparable work found afterward
- 1(c) AI building on literature known beforehand
- 1(d) AI collaborating with humans
- 2(a) literature search / 2(b) formalization / 2(c) rewriting / 2(d) computation

Note the wiki's primary axis is *provenance*, not *mechanism*. It has no category for
"what kind of certificate closed the problem," which is exactly what the rubric needs.
