# c2sva — ACSL contracts → RTL properties for HLS designs

`c2sva` takes C functions annotated with ACSL contracts, synthesizes them with Vitis HLS, translates
the contracts into properties over the generated RTL (requires → assumptions at call start,
ensures/assigns → assertions at call completion, `\valid` → memory-access checks), checks them
with SymbiYosys, and classifies every violation at the C level (E-ACSL, sanitizers and a
concrete contract evaluator). Translation rules: [docs/methodology.md](docs/methodology.md); semantics and soundness
theorems: [docs/theory.md](docs/theory.md).
Related work: [docs/related_work.md](docs/related_work.md). Project log: [plan.md](plan.md).

## Requirements
Vitis HLS 2026.1 (with Vitis HLS + Vivado free licences in `~/.Xilinx`), Frama-C 33 (WP, E-ACSL),
OSS CAD Suite (yosys, sby, yices, bitwuzla), Python 3.12 with `requirements.txt`.
`source setup/env.sh` sets everything up.

## Usage
```bash
bin/c2sva run --bench conformance --mutants 3            # one design per translation rule
bin/c2sva run --bench demo                                # demo designs
bin/c2sva run --bench abe --select 'MinMax/*' --mutants 2 # ACSL by Example subset
bin/c2sva run --bench acslbench --proven-only --sample 50 --seed 1
bin/c2sva run --bench path/to/file_or_dir -I path/to/include
bin/c2sva run --bench abe --config base,cyclic2,block2,complete,reshape2,bram,nopipe,unroll,fastclk,chain
bin/c2sva configs        # list HLS configurations
bin/c2sva list --bench abe
bin/c2sva report runs/<name>
bin/c2sva translate --bench acslbench --proven-only       # translation coverage only (no HLS)
bin/c2sva revalidate runs/<name>                          # re-run C-level replay of all RTL failures
bin/c2sva survivors runs/<name>                           # classify surviving mutants (random testing)
bin/c2sva prove runs/<name> --originals                   # A1: unbounded proofs (C invariants -> RTL, Houdini)
bin/c2sva prove-report runs/<name>                        # A1 tables -> prove_summary.{md,json}
PYTHONPATH=src python scripts/merge_runs.py runs/<target> runs/<rerun>...   # fold re-runs into a run
scripts/run_campaign.sh 8                                 # full evaluation (E1-E6), hours
PYTHONPATH=src python scripts/paper_tables.py             # evaluation tables -> runs/paper_tables.{md,tex}
PYTHONPATH=src python -m pytest tests -q                  # unit tests of the rules
```
Main options: `--tool vitis` (default), `--config`, `--mutants K`, `--array-size N` (memory cells,
default 4), `--lat-cap`, `--engines yices,bitwuzla`, `--sby-timeout`, `--jobs`, `--no-wp`,
`--no-access`, `--no-replay`.

## Outputs (`runs/<name>/`)
* `summary.md`, `summary.json`, `results.csv`, `table.tex` — aggregated results
* `cases/<design>/<tool>/<config>/<variant>/` — per case:
  `src/framac_norm.c` (canonical contract), `src/hls_src/` (HLS input), `hls/` (Vitis project),
  `interface.json` (C object ↔ RTL port map), `ir.json` (property IR with notes),
  `props.sva` (readable SVA), `formal/harness*.sv`, `formal/check_*/` (SymbiYosys runs, traces),
  `replay/` (C-level replay of a counterexample), `result.json`

## Outcome labels
`pass` (all clauses translated, proved to the bound, call reachable), `pass-partial`
(some clauses not translated), `fail` (+ violated clauses and C replay class),
`fail-inconclusive` (a precondition could not be assumed), `bounded(k)` (solver timeout after k
steps without violation), `vacuous`, `unsupported`, `error`.
Memory access: `ok`, `speculative` (beyond the `\valid` range but inside the memory),
`out-of-memory`.
Unbounded (A1, `prove.json`): `unbounded` + `proved_by` ∈ {kind, pdr, kind+inv, pdr+inv}; `deep_cex`
lists strategies that found a counterexample from reset beyond the BMC depth.

## Layout
```
src/c2sva/   acsl.py logic.py (front end) frontend.py (Frama-C, top, linking)
             hls.py (Vitis + interface/layout model) translate.py (rules -> IR)
             harness.py (environment, memory models, properties) formal.py (SBY, latency search)
             replay.py oracle.py (C-level validation) survivors.py (mutant analysis)
             invariants.py prove.py prove_report.py (A1: unbounded proofs from the C proof)
             mutate.py pipeline.py report.py bench.py cli.py dryrun.py (translation-only check)
tests/       test_rules.py test_invariants.py (unit), conformance/ (one design per rule)
benchmarks/  ACSL by Example, ACSLBench          docs/  methodology, related work
```

## Repository contents
- `src/c2sva/`, `bin/`, `scripts/`, `tests/`, `setup/`, `docs/` — the tool, campaign scripts, tests, docs.
- `benchmarks/raw/acsl-by-example` — git submodule (pinned commit); clone with `git clone --recursive`
  or run `git submodule update --init`. `benchmarks/raw/acslbench` — ACSLBench snapshot.
- `results/wp/` — Frama-C WP results on the benchmarks (used by `--proven-only`).
- `runs/` — results only: per-design verdicts (`result.json`, `prove.json`, `qed.json`), run summaries,
  final tables (`runs/paper_tables_F.md`), campaign logs, and the full demo case `runs/DEMO_find`.
  Solver work directories (~110 GB of models and traces) are not committed; `bin/c2sva run`/`prove`
  regenerate them.
