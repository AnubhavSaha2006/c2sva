# Evaluation tables (generated)

### T1 Coverage funnel (originals, Vitis base)

| Suite | Designs | Front end | HLS | Checked | Clauses | Clause % |
|---|---|---|---|---|---|---|
| Conformance | 17 | 17 | 17 | 17 | 111/111 | 100.0 |
| ACSL by Example | 82 | 73 | 72 | 72 | 725/727 | 99.7 |
| ACSLBench (proven) | 419 | 380 | 352 | 352 | 1827/1902 | 96.1 |

### T2 Correctness on originals (Vitis base)

| Suite | Pass | Pass partial | Fail | Bounded | Vacuous | Agree w/ C proof | RTL fail on proven C | C replay of failures |
|---|---|---|---|---|---|---|---|---|
| Conformance | 16 | 0 | 0 | 1 | 0 | 16 | 0 | -- |
| ACSL by Example | 54 | 2 | 0 | 16 | 0 | 56 | 0 | -- |
| ACSLBench (proven) | 300 | 32 | 0 | 19 | 1 | 332 | 0 | -- |

### T3 Property strength (automatic mutants, Vitis base)

| Suite | Mutants checked | Non-equivalent | Killed | Mutation score % | Kills confirmed in C | Equivalent passing |
|---|---|---|---|---|---|---|
| Conformance | 42 | 36 | 36 | 100.0 | 36 | 6/6 |
| ACSL by Example | 109 | 106 | 98 | 92.5 | 98 | 3/3 |
| ACSLBench (proven) | 519 | 383 | 305 | 79.6 | 305 | 136/136 |

### T4 Robustness across Vitis optimisations (originals)

| Suite | Config | Designs | Pass | Fail | Bounded | Error/unsupported | Overlap mode | Speculative | Out-of-memory |
|---|---|---|---|---|---|---|---|---|---|
| Conformance | block2 | 17 | 16 | 0 | 1 | 0 | 2 | 9 | 0 |
| Conformance | bram | 17 | 16 | 0 | 1 | 0 | 0 | 0 | 9 |
| Conformance | chain | 17 | 16 | 0 | 1 | 0 | 2 | 9 | 0 |
| Conformance | complete | 17 | 16 | 0 | 1 | 0 | 2 | 0 | 0 |
| Conformance | cyclic2 | 17 | 16 | 0 | 1 | 0 | 2 | 9 | 0 |
| Conformance | fastclk | 17 | 16 | 0 | 1 | 0 | 2 | 8 | 0 |
| Conformance | nopipe | 17 | 16 | 0 | 1 | 0 | 0 | 9 | 0 |
| Conformance | reshape2 | 17 | 16 | 0 | 1 | 0 | 2 | 9 | 0 |
| Conformance | reshapeb2 | 17 | 16 | 0 | 1 | 0 | 2 | 8 | 0 |
| Conformance | toppipe | 17 | 16 | 0 | 1 | 0 | 3 | 9 | 0 |
| Conformance | unroll | 17 | 16 | 0 | 1 | 0 | 2 | 9 | 0 |
| ACSL by Example | bram | 82 | 59 | 0 | 13 | 10 | 0 | 2 | 58 |
| ACSL by Example | cyclic2 | 82 | 56 | 0 | 16 | 10 | 25 | 52 | 0 |
| ACSL by Example | nopipe | 82 | 56 | 0 | 16 | 10 | 0 | 50 | 0 |
| ACSL by Example | reshape2 | 82 | 56 | 0 | 16 | 10 | 25 | 47 | 0 |
| ACSL by Example | toppipe | 82 | 54 | 0 | 18 | 10 | 1 | 45 | 0 |
| ACSL by Example | unroll | 82 | 56 | 0 | 16 | 10 | 25 | 49 | 0 |

### T5 Memory-access findings on originals

| Suite / config | ok | speculative | out-of-memory | n/a / undecided |
|---|---|---|---|---|
| Conformance (base) | 1 | 9 | 0 | 7 |
| ACSL by Example (base) | 4 | 49 | 0 | 29 |
| ACSLBench (proven) (base) | 29 | 114 | 0 | 276 |
| Conformance (block2) | 1 | 9 | 0 | 7 |
| Conformance (bram) | 1 | 0 | 9 | 7 |
| Conformance (chain) | 1 | 9 | 0 | 7 |
| Conformance (complete) | 0 | 0 | 0 | 17 |
| Conformance (cyclic2) | 1 | 9 | 0 | 7 |
| Conformance (fastclk) | 2 | 8 | 0 | 7 |
| Conformance (nopipe) | 1 | 9 | 0 | 7 |
| Conformance (reshape2) | 1 | 9 | 0 | 7 |
| Conformance (reshapeb2) | 2 | 8 | 0 | 7 |
| Conformance (toppipe) | 1 | 9 | 0 | 7 |
| Conformance (unroll) | 1 | 9 | 0 | 7 |
| ACSL by Example (bram) | 4 | 2 | 58 | 18 |
| ACSL by Example (cyclic2) | 1 | 52 | 0 | 29 |
| ACSL by Example (nopipe) | 2 | 50 | 0 | 30 |
| ACSL by Example (reshape2) | 5 | 47 | 0 | 30 |
| ACSL by Example (toppipe) | 5 | 45 | 0 | 32 |
| ACSL by Example (unroll) | 4 | 49 | 0 | 29 |

### T6 Latency and call mode (originals, base)

| Measure | Designs |
|---|---|
| established | 427 |
| of which HLS report had no fixed latency | 208 |
| report bound smaller than established bound | 200 |
| not established within cap | 14 |
| overlapping-call mode (pipelined / auto-rewind top) | 59 |

### T7 Sensitivity to the memory bound N (conformance)

| N | Designs | Pass | Fail | Bounded | Median s |
|---|---|---|---|---|---|
| 2 | 17 | 16 | 0 | 1 | 18.2 |
| 4 | 17 | 16 | 0 | 1 | 18.6 |
| 8 | 17 | 16 | 0 | 1 | 21.4 |

### T8 Reasons for untranslated clauses / rejected designs (base)

| Reason | Count |
|---|---|
| global variable c | 26 |
| global variable | 16 |
| design rejected: struct Stack: array/pointer member data | 9 |
| design rejected: unsupported \valid argument | 7 |
| axiomatic logic factorial | 4 |
| global variable rr | 4 |
| design rejected: address-of (&) in contract | 4 |
| design rejected: ACSL builtin \null | 3 |
| design rejected: no function with an ACSL contract | 3 |
| unknown logic function/predicate sizeof | 3 |
| clause too large after inlining (node budget exceeded) | 2 |
| design rejected: indexing a non-variable | 2 |
| inductive logic same_elements | 2 |
| global variable behavior | 2 |
| abstract logic matcher_ab | 1 |
| design rejected: C type 'enum type2' | 1 |
| design rejected: struct _vvc: array/pointer member a | 1 |
| abstract logic matcher_a_or_b | 1 |
| design rejected: non-constant array size 'size' | 1 |
| global variable status | 1 |
| global variable at | 1 |
| inductive logic Permuta | 1 |
| quantifier under mixed polarity | 1 |
| clause too large to translate (enumeration budget exceeded) | 1 |
| design rejected: floating-point type | 1 |

### T9 Median run time per stage (s)

| Suite | frontend | wp | hls | latency | bmc | access | cover | total |
|---|---|---|---|---|---|---|---|---|
| Conformance | 0.3 | 1.0 | 12.7 | 0.3 | 1.2 | 0.5 | 0.4 | 18.6 |
| ACSL by Example | 0.5 | 2.4 | 16.9 | 1.0 | 83.2 | 0.8 | 0.6 | 195.8 |
| ACSLBench (proven) | 0.4 | 1.1 | 13.8 | 0.4 | 0.7 | 0.6 | 0.4 | 18.7 |
