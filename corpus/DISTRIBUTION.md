# Phase 1 corpus: mechanism distribution

Cases: **21**

## Primary resolution mechanism

| mechanism            | n | share |                      |
|----------------------|---|-------|----------------------|
| LITERATURE_RECALL    | 4 | 19%   | ####................ |
| CONSTRUCTION         | 4 | 19%   | ####................ |
| NOVEL_ARGUMENT       | 5 | 24%   | #####............... |
| SEARCH_PLUS_VERIFIER | 8 | 38%   | ########............ |

**7 of 21 primary classifications are flagged `contested: true`.** Read the distribution above with that in mind.

## Mechanisms appearing anywhere (primary or secondary)

| mechanism            | as primary | as secondary | total |
|----------------------|------------|--------------|-------|
| LITERATURE_RECALL    | 4          | 1            | 5     |
| CONSTRUCTION         | 4          | 7            | 11    |
| NOVEL_ARGUMENT       | 5          | 5            | 10    |
| SEARCH_PLUS_VERIFIER | 8          | 3            | 11    |

## Certificate size

| size   | n  | share |                      |
|--------|----|-------|----------------------|
| SMALL  | 14 | 67%   | #############....... |
| MEDIUM | 6  | 29%   | ######.............. |
| LARGE  | 1  | 5%    | #................... |

## Mechanism x certificate size

| mechanism            | SMALL | MEDIUM | LARGE |
|----------------------|-------|--------|-------|
| LITERATURE_RECALL    | 3     | 1      | 0     |
| CONSTRUCTION         | 2     | 2      | 0     |
| NOVEL_ARGUMENT       | 2     | 2      | 1     |
| SEARCH_PLUS_VERIFIER | 7     | 1      | 0     |

## Verification method

| method           | n | share |
|------------------|---|-------|
| LEAN             | 8 | 38%   |
| HUMAN_REFEREEING | 7 | 33%   |
| COMPUTER_ALGEBRA | 4 | 19%   |
| EXPERT_INFORMAL  | 2 | 10%   |

## Status

| status    | n  | share |
|-----------|----|-------|
| RESOLVED  | 18 | 86%   |
| DISPUTED  | 1  | 5%    |
| RETRACTED | 1  | 5%    |
| ADVANCED  | 1  | 5%    |

## Autonomy

| autonomy                                        | n  | share |
|-------------------------------------------------|----|-------|
| AUTONOMOUS                                      | 11 | 52%   |
| AI_LED_HUMAN_VERIFIED                           | 3  | 14%   |
| AI_ASSISTED                                     | 2  | 10%   |
| COLLABORATIVE                                   | 2  | 10%   |
| None                                            | 2  | 10%   |
| MIXED - the taxonomy's primary axis IS autonomy | 1  | 5%    |

## Cross-field technique transfer

Occurred in **5 of 21** cases (24%).

| case                                | from                                                 | to                           |
|-------------------------------------|------------------------------------------------------|------------------------------|
| erdos-1026-monotone-subsequence     | Discrete geometry (square/rectangle packing)         | Extremal combinatorics (mono |
| erdos-1196-primitive-sets           | Probability (Markov chain / stationary distribution  | Multiplicative number theory |
| erdos-52-sum-product-human-followup | Algebraic number theory (the same Golod-Shafarevich  | Additive combinatorics       |
| erdos-90-unit-distance              | Algebraic number theory (class field towers; Golod-S | Discrete/combinatorial geome |
| sawin-unit-distance-explicit        | Algebraic number theory (Golod-Shafarevich)          | Discrete geometry            |

## Field

| field                                                | n |
|------------------------------------------------------|---|
| Combinatorics                                        | 3 |
| Number Theory                                        | 3 |
| N/A - metascience                                    | 2 |
| Discrete Geometry                                    | 2 |
| Mixed - Discrete Geometry, Analysis, Combinatorics   | 1 |
| Algebraic Complexity                                 | 1 |
| Competition mathematics                              | 1 |
| Number Theory / Combinatorics                        | 1 |
| Mixed - Mathematics and Theoretical Computer Science | 1 |
| Number Theory (presumed)                             | 1 |
| Mixed - Number Theory, Combinatorics                 | 1 |
| Mixed - number theory, combinatorics, graph theory   | 1 |
| Optimization                                         | 1 |
| Algebra                                              | 1 |
| Mixed - the whole Erdős corpus                       | 1 |


# Two-dimensional taxonomy

## Certificate type x production process

| certificate \ process | RETR | DIRECT | SEARCH | TRANS | SEEDED | HUMAN | tot |
|-----------------------|------|--------|--------|-------|--------|-------|-----|
| WITNESS               | .    | 1      | 3      | .     | .      | 2     | 6   |
| FORMAL_PROOF          | .    | .      | 5      | 2     | .      | .     | 7   |
| INFORMAL_PROOF        | .    | 3      | 1      | .     | 1      | .     | 5   |
| CITATION              | 3    | .      | .      | .     | .      | .     | 3   |
| EVIDENCE              | .    | .      | .      | .     | .      | .     | 0   |
| TOTAL                 | 3    | 4      | 9      | 2     | 1      | 2     | 21  |

9 of 30 cells occupied.

## Frontier

| frontier | n  | share |                      |
|----------|----|-------|----------------------|
| ADVANCES | 14 | 67%   | #############....... |
| SURFACES | 5  | 24%   | #####............... |
| MATCHES  | 1  | 5%    | #................... |
| BEHIND   | 1  | 5%    | #................... |

## Inference mode (Peirce)

| inference_mode | n  | share |                      |
|----------------|----|-------|----------------------|
| INDUCTION      | 12 | 57%   | ###########......... |
| TRANSPORT      | 5  | 24%   | #####............... |
| DEDUCTION      | 4  | 19%   | ####................ |

## Gowers culture

| culture         | n  | share |                      |
|-----------------|----|-------|----------------------|
| PROBLEM_SOLVING | 14 | 67%   | #############....... |
| MIXED           | 6  | 29%   | ######.............. |
| THEORY_BUILDING | 1  | 5%    | #................... |

## Gowers culture x frontier

| culture \ frontier | ADVANCES | MATCHES | BEHIND | SURFACES |
|--------------------|----------|---------|--------|----------|
| PROBLEM_SOLVING    | 8        | 1       | 1      | 4        |
| THEORY_BUILDING    | 1        | .       | .      | .        |
| MIXED              | 5        | .       | .      | 1        |

## Frontier-advancing AI cases only (n=12 of 19 AI cases)

| case                             | certificate    | process              | culture         |
|----------------------------------|----------------|----------------------|-----------------|
| alphaevolve-math-constructions   | WITNESS        | GUIDED_SEARCH        | PROBLEM_SOLVING |
| alphaevolve-matmul-4x4           | WITNESS        | GUIDED_SEARCH        | PROBLEM_SOLVING |
| alphaproof-nexus-erdos-nine      | FORMAL_PROOF   | GUIDED_SEARCH        | PROBLEM_SOLVING |
| bolzano-eight-problems           | INFORMAL_PROOF | GUIDED_SEARCH        | MIXED           |
| erdos-1196-primitive-sets        | INFORMAL_PROOF | SEEDED_COLLABORATION | MIXED           |
| erdos-397-formalization          | FORMAL_PROOF   | GUIDED_SEARCH        | PROBLEM_SOLVING |
| erdos-728-factorial-divisibility | FORMAL_PROOF   | GUIDED_SEARCH        | PROBLEM_SOLVING |
| erdos-90-unit-distance           | INFORMAL_PROOF | DIRECT_GENERATION    | MIXED           |
| funsearch-cap-set                | WITNESS        | GUIDED_SEARCH        | PROBLEM_SOLVING |
| gemini-aletheia-autonomous-erdos | INFORMAL_PROOF | DIRECT_GENERATION    | PROBLEM_SOLVING |
| jacobian-conjecture-c3           | WITNESS        | DIRECT_GENERATION    | THEORY_BUILDING |
| knuth-claude-cycles              | FORMAL_PROOF   | GUIDED_SEARCH        | PROBLEM_SOLVING |

## Data hygiene

- Flagged-unverifiable claims recorded across corpus: **42**
- Cases flagged `data_quality: THIN`: ['erdos-397-formalization']
- Cases with no sources at all: none
- Cases resting on NO primary source: ['alphaevolve-math-constructions', 'alphaevolve-matmul-4x4', 'axiom-fel-conjecture-disputed']
