# c2sva run summary

## Per tool / HLS configuration

| tool | config | designs | frontend_ok | hls_ok | checked | pass_full | pass_partial | fail | bounded | vacuous | agree | disagree | clauses | clause_rate | access_speculative | access_out_of_memory | overlap_mode | mutants | mutants_checked | killed | mutation_score | survivors | survivors_missed_bug | survivors_safety_only | survivors_untranslated | survivors_no_violation | survivors_not_analysed | equivalent_pass | kills_confirmed_in_C | median_total_s |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| vitis | base | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 1 | 0 | 11/11 | 100.0 | 1 | 0 | 1 | 2 | 2 | 1 | 50.0 | 1 | 0 | 0 | 0 | 0 | 1 | 0 | 1 | 48.4 |

### vitis/base

- original outcomes: {'pass': 1}
- replay of RTL failures on originals: {}
- replay of mutant kills: {'c-contract-violation': 1}
- mutants surviving although C-level proof fails: 1
- median seconds per stage: {'frontend': 1.3, 'wp': 1.9, 'hls': 17.7, 'latency': 2.0, 'bmc': 24.0, 'access': 0.5, 'cover': 0.5, 'total': 48.4}

## Why originals were not checked


## Why clauses were not translated


## Translation strategies used

- 2 x \forall i
- 1 x \exists i
- 1 x cell of a outside 0..3
- 1 x exact frame over all 4 element(s) of a
- 1 x holds by construction

## RTL failures / vacuity on C-proven originals (investigate)

- none

## Memory-access violations on originals

- Nonmutating__find/vitis/base/original: ['P7: requires \\valid_read ( a + ( 0 .. n - 1 ) )  [access in range: a bank 0 port 0]']

Latency: {'established': 1, 'report_undefined': 1, 'not_established': 0}
