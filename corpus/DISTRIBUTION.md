# Phase 1 corpus: mechanism distribution

Cases: **20**

## Primary resolution mechanism

| mechanism            | n | share |                      |
|----------------------|---|-------|----------------------|
| LITERATURE_RECALL    | 4 | 20%   | ####................ |
| CONSTRUCTION         | 3 | 15%   | ###................. |
| NOVEL_ARGUMENT       | 5 | 25%   | #####............... |
| SEARCH_PLUS_VERIFIER | 8 | 40%   | ########............ |

**7 of 20 primary classifications are flagged `contested: true`.** Read the distribution above with that in mind.

## Mechanisms appearing anywhere (primary or secondary)

| mechanism            | as primary | as secondary | total |
|----------------------|------------|--------------|-------|
| LITERATURE_RECALL    | 4          | 1            | 5     |
| CONSTRUCTION         | 3          | 7            | 10    |
| NOVEL_ARGUMENT       | 5          | 5            | 10    |
| SEARCH_PLUS_VERIFIER | 8          | 2            | 10    |

## Certificate size

| size   | n  | share |                      |
|--------|----|-------|----------------------|
| SMALL  | 13 | 65%   | #############....... |
| MEDIUM | 6  | 30%   | ######.............. |
| LARGE  | 1  | 5%    | #................... |

## Mechanism x certificate size

| mechanism            | SMALL | MEDIUM | LARGE |
|----------------------|-------|--------|-------|
| LITERATURE_RECALL    | 3     | 1      | 0     |
| CONSTRUCTION         | 1     | 2      | 0     |
| NOVEL_ARGUMENT       | 2     | 2      | 1     |
| SEARCH_PLUS_VERIFIER | 7     | 1      | 0     |

## Verification method

| method           | n | share |
|------------------|---|-------|
| LEAN             | 7 | 35%   |
| HUMAN_REFEREEING | 7 | 35%   |
| COMPUTER_ALGEBRA | 4 | 20%   |
| EXPERT_INFORMAL  | 2 | 10%   |

## Status

| status    | n  | share |
|-----------|----|-------|
| RESOLVED  | 17 | 85%   |
| DISPUTED  | 1  | 5%    |
| RETRACTED | 1  | 5%    |
| ADVANCED  | 1  | 5%    |

## Autonomy

| autonomy                                        | n  | share |
|-------------------------------------------------|----|-------|
| AUTONOMOUS                                      | 11 | 55%   |
| AI_ASSISTED                                     | 2  | 10%   |
| COLLABORATIVE                                   | 2  | 10%   |
| None                                            | 2  | 10%   |
| AI_LED_HUMAN_VERIFIED                           | 2  | 10%   |
| MIXED - the taxonomy's primary axis IS autonomy | 1  | 5%    |

## Cross-field technique transfer

Occurred in **5 of 20** cases (25%).

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
| Number Theory                                        | 3 |
| N/A - metascience                                    | 2 |
| Combinatorics                                        | 2 |
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
| FORMAL_PROOF          | .    | .      | 4      | 2     | .      | .     | 6   |
| INFORMAL_PROOF        | .    | 3      | 1      | .     | 1      | .     | 5   |
| CITATION              | 3    | .      | .      | .     | .      | .     | 3   |
| EVIDENCE              | .    | .      | .      | .     | .      | .     | 0   |
| TOTAL                 | 3    | 4      | 8      | 2     | 1      | 2     | 20  |

9 of 30 cells occupied.

## Frontier

| frontier | n  | share |                      |
|----------|----|-------|----------------------|
| ADVANCES | 13 | 65%   | #############....... |
| SURFACES | 5  | 25%   | #####............... |
| MATCHES  | 1  | 5%    | #................... |
| BEHIND   | 1  | 5%    | #................... |

## Inference mode (Peirce)

| inference_mode | n  | share |                      |
|----------------|----|-------|----------------------|
| INDUCTION      | 11 | 55%   | ###########......... |
| TRANSPORT      | 5  | 25%   | #####............... |
| DEDUCTION      | 4  | 20%   | ####................ |

## Gowers culture

| culture         | n  | share |                      |
|-----------------|----|-------|----------------------|
| PROBLEM_SOLVING | 13 | 65%   | #############....... |
| MIXED           | 6  | 30%   | ######.............. |
| THEORY_BUILDING | 1  | 5%    | #................... |

## Gowers culture x frontier

| culture \ frontier | ADVANCES | MATCHES | BEHIND | SURFACES |
|--------------------|----------|---------|--------|----------|
| PROBLEM_SOLVING    | 7        | 1       | 1      | 4        |
| THEORY_BUILDING    | 1        | .       | .      | .        |
| MIXED              | 5        | .       | .      | 1        |

## Frontier-advancing AI cases only (n=11 of 18 AI cases)

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

## Data hygiene

- Flagged-unverifiable claims recorded across corpus: **37**
- Cases flagged `data_quality: THIN`: ['erdos-397-formalization']
- Cases with no sources at all: none
- Cases resting on NO primary source: ['alphaevolve-math-constructions', 'alphaevolve-matmul-4x4', 'axiom-fel-conjecture-disputed']
