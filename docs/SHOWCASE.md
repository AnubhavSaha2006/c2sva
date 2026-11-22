# Showcase: complete end-to-end results on the 17 conformance designs (frozen re-run, final code)

Each conformance design exercises one translation rule (behaviors, ∃/∀, recursion, pointers, typedefs,
Verilog keywords, frames, \old memory, nested ∀, products, builtins, void outputs, shifts/casts,
predicates, structs).

| Stage | Result | Source |
|---|---|---|
| Contract → RTL properties | 111/111 clauses translated (100 %) | runs/F1_conformance/summary.md |
| Formal check (BMC) of the Vitis RTL | 16 pass, 1 bounded (c10, triple products), **0 fail** | same |
| Mutants (3 per design, C-level faults) | 42 checked, all **36 non-equivalent killed** (100 %), every kill confirmed on the C code; 6 equivalent pass | same |
| Spec-free baseline (A-QED-style consistency, QED-FC) | kills **0** of the 36 mutants (24 pass, 12 time out) | runs/F1_conformance/cases/*/…/qed.json |
| Proof for all executions (A1) | **15/17** (plain k-induction 7, IC3/PDR 8). Open: c10 (nonlinear) and c12 (Houdini timeout in the long run; proved in the earlier campaign) | runs/F1_conformance/prove_summary.md |
| C loop invariants that hold on the RTL | 18/21 | same |
| Latency bound proved for all executions | 14/17 (plain k-induction 6); contract **and** latency for all executions: 14 | same |
| Robustness: 11 Vitis settings (base, cyclic2, block2, complete, reshape2, reshapeb2, bram, nopipe, unroll, fastclk, chain) | 16 pass + 1 bounded under **every** setting, **0 fail** | runs/F2_conformance_cfg/summary.md |
| Array size N = 2 / 4 / 8 | identical verdicts; k_min tables | runs/F6_conformance_N*/prove_summary.md |

Live single-design demo: scripts/demo_monday.sh (ACSL by Example `find`); guide: docs/DEMO.md.
