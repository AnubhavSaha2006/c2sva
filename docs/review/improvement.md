# c2sva — improvement review (demanding TCAD / FMSD reviewer)

Reviewer role: improvement-oriented, read-only. Date 2026-11-21.
Material read: problem_statement.txt, README.md, docs/{methodology,theory,paper_draft,related_work}.md,
runs/paper_tables.md, runs/E1/E3/E4 summaries and prove_summaries, runs/QED_E1.log, plan.md §10–§14,
docs/research_log.md (R1–R30, headings and R27–R30 in detail), docs/research_ideas.md §4.
Usage check before the single work round: weekly 46 %, 5-hour 0 % (below the 65 % / 55 % stop thresholds); one round was used.
Ideas already in research_log / research_ideas are referenced as [R#] / [RI §x] and not repeated.

---------------------------------------------------------------------------------------------------

## 0. Executive summary

**What is strong.** A precise transaction semantics with a polarity lemma and bounded/unbounded soundness theorems; a
layout-generic interface model validated over 11 Vitis presets; 0 failing originals out of roughly 1,500 checks; every
mutant kill replayed at C level (98/98 ABE, 305/305 ACSLBench); an unusually honest analysis of what the unbounded proofs
mean (window-covering, k_min, rho); and a by-product finding (defective ACSLBench axiomatics, speculative reads).

**The five things a reviewer will attack first.**
1. **No true positive on an RTL-origin bug.** All 0 failures on originals are "expected" (C proved, HLS assumed correct).
   All kills are *C-level mutants*; "confirmed at C level" is nearly tautological for those, since the mutant C violates the
   contract regardless of what HLS does. The paper shows the tool does not raise false alarms and that properties are
   strong against C faults. It does not show it finds anything the C proof plus co-simulation would not. Needed: real or
   injected *HLS/RTL-level* faults (unsafe pragmas, HLS-version regressions, RTL mutants) and a testing/co-sim baseline.
2. **Scale and realism.** ACSLBench/ABE are 10–50-line single functions at N=4 cells (median total 18 s, median BMC 0.7 s).
   Nothing in the problem statement's own list (MachSuite, CHStone, HLS Factory) is used; no m_axi/s_axilite/stream/dataflow,
   no globals, no fixed-point. "Scalable" and "robust across HLS optimisations" are therefore supported only for
   layout/schedule directives on tiny kernels.
3. **RQ6 is thin and partly circular.** The tool itself has no LLM in the main path; the LLM study is 10 prompts, 3 validated
   lifts, the prompt (cheat-sheet, repair loop) was tuned on those same 10, the LLM is a Claude subagent (not reproducible, no
   API/temperature log), and there is no LLM-only baseline (LLM writes SVA directly) to show that formal validation matters.
4. **"Unbounded" is over-sold.** Unbounded in time and number of calls, but at N=4 cells, and most proofs have k >= L+1
   (equivalent to BMC to the call diameter, by the project's own Theorem R10). The abstract-level numbers do not carry this.
5. **Generalisation of the headline finding is unproven.** "Speculative reads become true out-of-bounds with bram" is
   measured with the physical array sized to N (the HLS input is rewritten to `T p[N]`). In a deployment the physical array
   is usually larger than the contract's n, so the same read is in-bounds. The real-hazard setting (m_axi, unmapped address
   ranges) is untested.

**Top priorities (effort in person-days).** (a) P1 real/injected RTL-level bug study + co-sim/testing baseline (4–6 d);
(b) P1 one realistic benchmark set with LLM-written, WP-validated contracts: MachSuite/CHStone, 8–12 kernels incl. m_axi (6–10 d);
(c) P1 claim hygiene and stale-number reconciliation (0.5–1 d); (d) P1 RQ6: LLM-only SVA baseline plus a pre-registered
held-out lifting set and NL->ACSL on ACSLBench's `nl` field (3–5 d); (e) P2 second HLS tool actually run (Bambu 3–4 d, or
drop the tool-agnostic claim); (f) P2 statistical treatment (1–2 d); (g) P2 reporting/usability items in section 5 (3–4 d).

---------------------------------------------------------------------------------------------------

## 1. Per-RQ assessment

Rating: evidence strength S (strong) / M (medium) / W (weak).

### RQ1 — How can untimed C contracts be mapped to timed RTL properties?  [S for mechanism, M for evidence]
*Evidence.* Transaction semantics (theory §1), rule table (methodology §5), Lemma 1 + Theorem 1, T1 funnel
(ABE 725/727 = 99.7 % clauses; ACSLBench 1827/1902 = 96.1 %), 0 failures on 1,500+ originals, cover for non-vacuity,
overlapping-call mode for 59 designs.
*Attack lines.*
- Clause-level coverage hides design-level coverage: 32/333 ACSLBench originals are `pass-partial` (about 10 %) and only
  56 % of the 598 tasks are checked at all (598 -> 419 proved -> 380 front end -> 352 HLS -> 333 checked). Report the funnel against all 598, and
  design-level fully-translated rate.
- The 419 "proved" filter selects *easier* specifications; the 179 excluded tasks may be where translation is hardest. Selection bias
  must be stated, and a sample of unproved tasks run anyway (the translator does not need the C proof; only mutant references do).
- Three of the 419 are vacuously "proved" (task_13, task_216 inconsistent factorial; task_452 inconsistent `ordered`, `reads`-less `count`).
  They sit in the denominator today (R29-F proposes a with/without table — not yet in the docs).
- Non-vacuity is a single `cover ap_done`. A reviewer will ask for per-clause vacuity: implication antecedents
  (`ap_done && k<old_n |-> ...`, behavior `assumes`, `\forall` ranges) can be unsatisfiable while the call itself completes.
  (Per-behavior covers and output-coverage are only "quick wins queued".) Add antecedent covers for every asserted implication.
- "Discharged by interface" (`assigns \nothing` => no write port) is counted as a translated clause; separate it from
  checked-on-RTL clauses in the clause rate.
- Contract preservation is weaker than equivalence by design, which is good, but the paper never shows a case where that
  matters (a C rewrite that is not equivalent yet contract-correct). R5/R30-S3 would; it is not run.
- Sequential-call semantics for designs that write memory; overlap only for read-only blocks. Many pipelined HLS kernels (II=1 writes into
  an output array) are exactly the overlapping, writing case.

### RQ2 — How are source variables related to HLS-generated RTL signals?  [M]
*Evidence.* Port-level `locate(i)=(bank,word,slot)` model from the csynth report, 11 presets, structs; internal-register
binding for A1 (name match, schedule aliases, lag rules; C loop invariants surviving: 128/354 on ABE).
*Attack lines.*
- The interface model is validated only by the properties passing under it. A consistent mis-mapping on a symmetric design would also
  pass. Add *map-mutation sanity tests*: perturb the map (swap two cells, flip slot order, shift byte address) and show the
  checker flags >= 95 % of designs that are not symmetric. This quantifies how sensitive the whole verdict is to T1.
- Internal binding (RQ2 proper for local variables) is name-based and partial; equal_range-class (callee loops) = 0/6 survive; survival
  is reported without a baseline of "what fraction is expected by chance". The (II,D) predictor is post-hoc on 54 non-independent
  designs (families remove_copy/remove_copy2/3, equal_range/2, max_element/2); Fisher p = 7.5e-9 treats them as independent.
  Use a permutation test clustered by function family and say the predictor was fitted after seeing data (R22 pre-registered the
  numbers *after* the E1 data).
- Only Vitis names/reports are used; stability across Vitis versions (2024.x vs 2026.1) is untested: the report formats are the
  true interface between tools.

### RQ3 — Variable and dynamic latency  [M]
*Evidence.* Latency search (BMC-established bounds for 427 designs, 208 without report latency), RB proof "latency bound for all
executions" 16/17 on E1 (plain k-ind 6), total correctness N=4 15/17; overlap mode.
*Attack lines.*
- The environment is *latency-friendly*: fixed 1-cycle read memory, `ap_continue` high, inputs stable, no back-pressure. Truly variable latency
  (stalled consumers, wait-state memory, m_axi) — what the problem statement means by "dynamic latency" — is not exercised.
  The data-dependent loops tested are still bounded by N cells.
- `bounded(k)` outcomes: 19/333 ACSLBench, 16/72 ABE (22 %). Their count depends on the 600 s timeout; no sensitivity of bounded vs timeout.
- E3/E4 latency-bound *proofs* are not in the tables (T6 is only BMC-established bounds; E1 prove_summary shows "latency 0/0" because the report is stale).
- "Report bound smaller than established bound" in 200 designs is a striking claim (the HLS report latency is wrong or measured under
  different assumptions?). It is not explained in the docs. If true it is a finding (report latency is for min/typ trip counts?); if
  not, it is a bug. Explain or drop.

### RQ4 — Array and memory properties  [M]
*Evidence.* Plain/partition/reshape/complete/bram layouts, structs, `\valid`/`\valid_read` access checks, frame conditions
(exact post-state), N = 2/4/8 conformance, ABE N=8/16 sweep in progress (E7).
*Attack lines.*
- Everything is for N <= 8 (16 in progress). No size-independent result; the size-independent route is only in R3 ideas. At the very least
  show verdict invariance of the 16 loop designs at N=8/16 (planned) and report which proofs need k growing with N.
- One physical array per pointer, sized N; no aliasing between parameters (restrict-like assumption never stated; `\separated` handling?),
  no 2-D/multi-dimensional arrays beyond conformance c05, no `char*` strings, no pointers to pointers (Vitis rejects them anyway),
  no `m_axi` (where "memory" is *shared and latency-variable*, and where speculative reads are real hazards).
- Speculative-read finding: see section 3 (T-spec). The ABE `bram` row (58/82 out-of-memory) is a statement about the N-sized array, not about real memories.

### RQ5 — Robustness across HLS optimisations  [M for layout robustness, W for "optimisation" robustness]
*Evidence.* E2: 17 designs x 11 presets (16 pass, 1 bounded each); E5: ABE x 6 presets, 432 checks, 0 failing originals.
*Attack lines.*
- Presets are mostly *layout* knobs (partition/reshape/bram) plus pipeline/unroll/clock. Missing optimisations that change semantics-relevant
  structure: `dataflow` (PIPO/FIFO), `loop_flatten/merge/tripcount`, `inline off`, `function_instantiate`, `dependence inter false`
  (can *break* semantics), `bind_storage latency=2/3`, `allocation`, `array_stencil`, `stream`, `ap_fixed/ap_int` types, `config_compile -pipeline_loops`,
  `ap_ctrl_none`, `s_axilite` control. The "RTL memory read latency = 1" harness assumption (T1) breaks as soon as `bind_storage latency>1`: verify it is
  read from the report or rejected.
- Pass under all presets is the *expected* outcome of a correct tool chain; what a reviewer needs is a negative control: a design/directive pair
  where preservation really breaks (unsafe `dependence`, `array_partition` plus out-of-contract access, ap_int width truncation) and that c2sva flags.
- E5 ran on pre-fix code (plan: "run before the translator fixes"); re-run or state it. ACSLBench was run on the base config only.
- paper_draft §7 says "the same properties hold under 11 configurations" for ABE; T4 shows 6 configs on ABE (base excluded) and 11 only on conformance.

### RQ6 — Can an LLM assist the translation while formal analysis validates?  [W]
*Evidence.* `llm.py`, `c2sva lift`; S1 on 10 untranslated axiomatic names: validated 3 (gcd, fact, GCD), 2 inconsistent-axiom confirmations,
5 unproved (count, ordered, Permuta, same_elements x2), 2 abstract; 12 seeded wrong definitions, 11 rejected, 1 genuinely equivalent
accepted (0 false accepts); end-to-end kills for 3 mutants.
*Attack lines.*
- The problem statement says "write a tool *with the help of LLM*". The default pipeline is rule-based; the LLM touches 0.5 % of
  clauses (3 recovered vs. 109 untranslated in ACSLBench, so 111 -> 108). Contribution to RQ6 as stated: marginal.
- **Contamination**: the prompt (ACSL cheat-sheet, repair loop) was iterated on exactly those 10 names. There is no held-out set.
- **Reproducibility**: no API key on the machine; the LLM is a Claude subagent with only the prompt files; no model id/temperature/seed log per
  response. A reviewer cannot rerun it (backend `anthropic` exists but unexercised).
- **No LLM-only baseline.** The claim "formal analysis validates" needs: (i) LLM writes SVA/ACSL-lifts directly with no validation: how many are wrong
  (false-accept rate)? (ii) with validation: how many are filtered? Without (i) the guardrail has no demonstrated value.
- Most "interesting" results (defective benchmark axioms) came from WP consistency probes, not the LLM; credit and attribute honestly.
- S1 validates *definitions*; the original RQ6 intent also covers LLM generating *contracts* (NL->ACSL, ACSLBench has an `nl` field), LLM proposing
  invariant candidates (S2 in R30), and LLM rewriting C for HLS (S3). None is run. NL->ACSL->c2sva is a free experiment on 598 tasks with ground-truth ACSL.

### Cross-cutting: A1 (unbounded proofs) and A2 (theory)  [M/S]
- The k_min/window-covering analysis is the most original and most honest part; the headline "unbounded 48/72 (ABE)" must always be paired with rho.
- A1 baselines: AutoINV is only approximated by the `hls` ablation (structural-only); say "AutoINV-like" and never "AutoINV". The A-QED baseline (`c2sva qed`) is a partial
  log of 24 lines (9 contract=fail/qed=PASS, 6 fail/TIMEOUT, 6 pass/PASS, 3 pass/TIMEOUT): useful ("consistent but wrong" mutants are missed by FC) but 9/24 results are timeouts
  and the run is incomplete.
- Theory: Theorem 2 assumes `Env` (T2) and reset behaviour with unconstrained datapath regs (an over-approximation, sound). Not mechanised; FMSD reviewers may ask for at least a
  paper proof of Lemma 1 per construct (only a sketch) or a Coq/Isabelle encoding of the polarity rule.

---------------------------------------------------------------------------------------------------

## 2. Missing experiments and baselines (priority and effort)

P1 = needed for a credible journal submission; P2 = strengthens; P3 = nice to have. Effort in person-days (pd).

| # | Experiment | Why a reviewer wants it | Prio | Effort |
|---|---|---|---|---|
| X1 | **Real/injected HLS-level bugs.** (a) pragma-induced faults on ABE/MachSuite kernels: `dependence inter false` on loop-carried deps, wrong `array_partition` factor vs access pattern, `unroll` with remainder, `ap_int<w>` truncation, `ap_fixed` rounding, `tripcount` misuse; (b) older Vitis versions / known-bug reports (research_ideas §4 item 5); (c) **RTL mutants** (stuck-at, inverted enable, off-by-one in address register via yosys mutate) to measure property strength independent of C. Report detection by c2sva vs by co-sim. | Today all kills are C mutants (tautologically C-confirmed); no evidence on faults introduced *by* the HLS step. | P1 | 4–6 |
| X2 | **Co-simulation / random-testing baseline** under equal wall time: Vitis C/RTL cosim with contract-guided random vectors, or Verilator simulation of the generated checkers with random vectors, on the X1 faults and on mutants (paired, McNemar). Also report time-to-detection. | Standard TCAD expectation ("why not just test?"). research_ideas §4 item 3 lists it; not started. | P1 | 2–3 |
| X3 | **Realistic benchmarks**: 8–12 MachSuite/CHStone/HLS-Factory kernels (e.g. gemm, spmv, fft, stencil2d, aes, adpcm, sha) with ACSL contracts written by an LLM (RQ6 link: contract synthesis) and *WP-validated*; default Vitis kernel interfaces (`m_axi` + `s_axilite`). Report translation rate, runtime, bounded/unbounded, and any `bounded` timeouts honestly. | Problem statement names these suites; ABE/ACSLBench are toy-scale. | P1 | 6–10 |
| X4 | **m_axi / s_axilite / ap_ctrl_none / hls::stream** at least one family each, with variable-latency memory (random wait-states) and back-pressure in the harness. | "Variable and dynamic latency" and "memory properties" are truly tested only with these; also where speculative reads matter. | P1 (m_axi), P2 (stream, dataflow) | 5–8 |
| X5 | **Second HLS tool actually run**: Bambu on conformance + ABE subset (back end exists; only 3 demo designs on an early pipeline are in `runs/demo_t1`), or XLS if feasible. If not run, delete "tool-agnostic" from the abstract. | Problem statement lists 3 tools; claim of tool-agnosticism is untested. | P2 | 3–5 |
| X6 | **LLM-only baseline for RQ6**: given C + ACSL + report/ports, an LLM emits SVA directly (3 prompts, 3 samples); measure (i) syntactic validity, (ii) agreement with c2sva verdicts on originals/mutants (false alarms, misses), (iii) cost. And NL->ACSL on a held-out 100 ACSLBench tasks (`nl` field) fed to c2sva (ground-truth ACSL available, C replay as oracle). | Shows what the formal side buys; fills the "tool with the help of LLM" gap. | P1 | 3–5 |
| X7 | **Held-out lifting set**: freeze prompt + parser, run S1 on all 109+2 untranslated clauses (R30 rank 1), report first-try / after-repair rates, route (WP unfold vs finite-instantiation), false-accept rate on >= 50 seeded wrong definitions. Log model id, temperature, prompt hash, response hash. | Removes contamination; makes RQ6 numbers quotable. | P1 | 2–3 |
| X8 | **Size-independent N** for at least c07/c09/c16 (quiescent-header designs): array-memory harness (R3.2) or at least N = 2..16 verdict/k_min series. | RQ4 core limitation; deployment gap. | P2 | 5–8 (full), 1 (series only) |
| X9 | **Global variables** (57 untranslated clauses ~ the largest untranslated cause: `c` 26, unnamed 16, `rr` 4...). Model a global as extra state variable / memory with initial value and frame (HLS maps to registers or ROM). | Largest single coverage loss; easy to attack in review. | P2 | 2–4 |
| X10 | **Interface-map sensitivity test** (mutate `locate` and measure how many verdicts flip). | Quantifies the trusted-base T1; cheap. | P2 | 1 |
| X11 | **Optimisations that change semantics-relevant structure**: dataflow/stream, loop_flatten/merge, bind_storage latency 2/3, function_instantiate, inline off, config_compile; plus a negative-control directive that breaks semantics. | RQ5 "optimisations" beyond layout. | P2 | 2–3 |
| X12 | **Full ACSLBench under the config matrix** (currently only base) or a stratified 100-design sample x 6 configs. | RQ5 evidence only on 82 designs. | P2 | 1 + compute |
| X13 | **Unproved ACSLBench tasks** (179): run original-only translation + check; the proof is not needed for the RTL side. Reduces selection bias; shows whether failures (true contract vs C bugs) appear. | Real-bug chance: contracts *not provable* may be *wrong*; RTL failures there would be classified by replay. | P2 | 1–2 |
| X14 | **Contract-consistency and vacuity checks** as first-class tool outputs (contract satisfiable? precondition non-empty? axioms consistent? per-clause antecedent covers). | Found 3 defective ACSLBench designs by accident; make systematic. | P2 | 1–2 |
| X15 | **Bounded vs timeout sensitivity** and solver variance: rerun 30 `bounded(k)` designs with 2x/4x timeout, other engine seeds; report verdict flips and time-to-verdict curves. | Statistical rigor. | P2 | 1 |
| X16 | **Complete the A-QED baseline** (`qed` log has 24 lines, many timeouts) plus AutoINV-like proof baseline on E3; report both honestly as "different question" baselines (R7). | Sets A1/contract value vs spec-free checks. | P2 | 1–2 |
| X17 | **LLM-rewritten HLS-C against original contract** (R30 S3), 15 designs x scripted + LLM rewrites + seeded faults. | Shows contract preservation as the stable verification asset; strongest RQ5/RQ6 story. | P2 (high payoff) | 4–6 |
| X18 | **Other mutation operators and larger sample**: statement deletion, unary-operator insertion, array-index off-by-one, loop-bound shifts; 5–10 mutants per design on 40 designs, exhaustive on 10 to estimate sampling error. | 2–3 round-robin first-order mutants/design is thin. | P3 | 1–2 |
| X19 | **Fixed-point / floating-point** (`ap_fixed`, float) — 1 ACSLBench design rejected for floats; HLS kernels use them. | Realism. | P3 | 3–5 |

---------------------------------------------------------------------------------------------------

## 3. Threats to validity not yet stated

(The paper_draft §8 lists bounded verification, shared parser, first-order mutants, WP incompleteness, one tool. The following are additional.)

**Construct validity**
- **T-mut: Mutant kills are C faults.** Mutants are created in C and synthesised; any faithful HLS makes the RTL violate the contract, so "C-confirmed" does not validate the RTL-side
  checker, only that the contract is strong. Present the mutation score as *specification strength*, not as RTL bug-finding power.
- **T-mut-denominator.** "Non-equivalent" means "WP could not prove the mutant" (not "a violating execution exists"). 76 ACSLBench survivors: 24 no-violation (probably equivalent), 45 memory-safety-only,
  2 untranslated clause, 5 not analysed. The 79.6/80.1 % score therefore mixes unknown-equivalent mutants (lower bound) with kills of runtime-error type (52 `c-runtime-error`, 1 non-termination
  out of 305, i.e. 17 % of kills are not contract violations).
- **T-ms-safety.** 45 survivors are memory-safety-only violations in C that the RTL check did *not* flag (OOB inside the physical N-array or speculative aliasing). It is the project's own
  memory-safety story; yet the survivor classification scores them as "not missed bugs".
- **T-spec: speculative/out-of-memory is defined relative to the N-sized physical array.** Out-of-memory under `bram` appears because the harness array has exactly N cells. With a larger physical
  array the same access is in range. State the finding as "reads beyond the `\valid` range, observable on the address bus", and show the hazard on a setting where it is real (m_axi, partial-mapped
  BRAM, ECC memory).
- **T-agree.** "Agreement with the C proof 332/332" is trivially a restatement of "no RTL failure" (cannot be anything else when no failures occur). Do not present it as an independent metric.
- **T-clause-rate.** Clause counts are dominated by designs with many clauses; also counts "discharged by interface" clauses.

**Internal validity**
- **Code frozen vs results mixed.** E1–E3 earlier code; E4 re-run partially (merges with `merge_runs.py`); E5 before translator fixes; E6/E7 after. Results are valid by argument ("fixes only removed false failures"), not
  by re-execution. Re-run the complete matrix once on the tagged commit for the paper.
- **Tool and tests co-evolved with the benchmarks** (conformance designs, rules and fixes derived from failures on ABE/ACSLBench): ACSLBench/ABE are training set as well as test set.
  There is no held-out benchmark. MachSuite/CHStone (X3) would be the held-out set.
- **Stale or inconsistent reports:** see section 4 (numbers differ between paper_tables, summary.md, prove_summary.md, plan.md).
- **Solver/timeouts:** 600 s per task, Yices->Bitwuzla portfolio; verdicts `bounded(k)` depend on the machine. No repeat runs, no seeds, no CIs.

**External validity**
- **One tool version, one device, one clock (10 ns, Artix UltraScale+),** Vitis 2026.1 report format; the interface model is read from that report.
- **Benchmarks are textbook algorithms,** small, mostly integer, single function (+ callees), mostly arrays of int; no real HLS idioms (streams, burst, ap_fixed, dataflow).
- **ACSLBench provenance:** LLM-generated tasks/contracts (unvalidated for meaning; 3 defective by inspection); "proved" is not "meaningful".
- **Environment assumptions (T2) are stronger than real systems:** no one else writes the memory during a call; `ap_continue` held high; inputs stable; reset only first cycle; fixed read latency.
  A real SoC (DMA, AXI interconnect) violates them. The theorem's "every caller that respects the block protocol" ignores memory sharing.
- **Reset/initial-state realism:** datapath regs unconstrained at start (over-approximation: sound) but verdicts like `pass` for *call sequences* assume the environment can leave any state; real devices
  initialise from bitstream (some counterexamples would be unreachable).

**Conclusion validity**
- Post-hoc predictors (II,D,δ) on 54 correlated designs; p-values reported without clustering or multiple-comparison correction.
- k_min measured on a coarse ladder (2,4,6,8,12,16,24,32) with censoring at 32; ratios like "0.89" come from a single design at N=4.
- "0 failures" cannot be interpreted as a precision estimate; with 0/1,500 the rule-of-three upper bound on false-alarm rate is about 0.2 % (state it) — but also on *miss rate* nothing is measured.

---------------------------------------------------------------------------------------------------

## 4. Claim hygiene — statements that over-claim or are stale

| Where | Statement | Problem | Fix |
|---|---|---|---|
| paper_draft Abstract | "tool-agnostic procedure" | Only Vitis evaluated; Bambu only 3 demo designs on an early pipeline (runs/demo_t1); XLS none. | "designed to depend only on the HLS report; evaluated with Vitis HLS" or run X5. |
| paper_draft §8 | "a second back end (Bambu) exists in the code base" | `hls.TOOLS` has a Bambu entry but no run since the pipeline changed (struct/overlap/layout model, replay, A1). | State "prototype; last exercised on 3 designs". |
| paper_draft Abstract | "proves the contracts on y % of the checked designs" | `pass` is *bounded* (N=4 cells, depth D) for the main campaign; only A1 gives unbounded-time results, still N=4. | Use "verified up to bound for N cells"; reserve "proved for all executions" for A1 and add "(N = 4 cells)". |
| paper_draft §7, T10 headline | "x/y designs proved for all executions" | Most proofs have k >= L+1 (window-covering): BMC to the call diameter; "unbounded" adds little over bounded for them. | Always report rho and the split inductive vs window-covering (E3: 18 vs 23 of 41). |
| paper_draft Abstract | "kills z % of non-equivalent mutants with every kill confirmed at the C level" | Mutants are C faults; confirmation is near-tautological; "non-equivalent" = WP-unprovable; 17 % of kills are runtime-error/non-termination, not contract violations. | Say "specification strength against C-level mutants"; report contract kills (252/305) separately. |
| paper_draft Abstract | "reveals that Vitis issues speculative memory reads beyond the ranges the contracts allow — benign with ap_memory but true out-of-bounds accesses with BRAM" | Relative to an N-cell physical array; "true OOB" is a modelling consequence; benign-ness proved only under the harness memory model (Prop. 4). | Reformulate as in T-spec; add the m_axi/larger-array experiment. |
| paper_draft §7 | "T4/T5: the same properties hold under 11 configurations" | ABE has 6 configs (E5, pre-fix code); ACSLBench 0; 11 only for 17 conformance designs. | Table caption per suite; re-run E5 on frozen code. |
| paper_draft §7 | "T2: ... agreement with C-level proofs; every RTL failure on a proven original is replayed (none expected...)" | "Agreement" is vacuous (no failures). For ABE only 47/82 designs are fully WP-proved (rest: Coq-only lemmas), yet the table says "agree w/ C proof" for 56. | Report agreement only for fully-proved designs; call it "no disagreement". |
| methodology §1 | "The contract is assumed to hold at the C level (benchmarks are proved with Frama-C/WP)" | True for 47/82 ABE; 3 ACSLBench "proved" tasks are vacuous (inconsistent axioms); proofs of others rely on `-wp-rte` settings and 10 s provers. | Add the defect table (R29-F) and restate. |
| methodology §9 | "Interface protocols m_axi/s_axilite..., structs, global state and floating point are outside the model" | **Stale**: structs are supported (c15–c17, ABE structs). Also "axiomatic/inductive logic is not translated" is stale after S1 lifts. | Update. |
| README | "validates every violation at the C level" / theory §6 "every kill ... confirmed" | Replay uses the same parser/evaluator plus E-ACSL; E-ACSL crashes -> ASan fallback; 'undetermined' classes exist. | Report class distribution; say "classified" not "validated". |
| theory §1 | "Env ... exactly the obligations Vitis' block-level protocol places on a caller" | Memory is also assumed quiescent during a call and read latency fixed; those are not block-protocol obligations. | List as T2 items explicitly. |
| theory §3 | "A PASS therefore covers call sequences" | Only in sequential mode up to bound D; for `bounded(k)` only k cycles; and cover shows only one call completes. | Qualify. |
| theory Prop. 4 | "values never influence a translated postcondition" | Holds for *translated* clauses only; with `pass-partial` untranslated clauses may depend on them. | Qualify with clause coverage. |
| paper_draft Abstract | "c2sva translates x % of all contract clauses" | Includes "discharged by interface" and excludes 9 Stack designs, 39 unsupported, 28 errors at design level. | Add design-level and all-598 funnel next to the clause rate. |
| plan §14 / research_log | "RQ3: latency bound proved for all executions 16/17" | On N=4 with call-age (16-bit) counters; wrapping artefacts were found (c08/c14 CTIs). | Add "for N=4, sequential harness". |
| E1 prove_summary.md | "unbounded: 15" and "RQ3 latency 0/0", k_min c05/c08 = 16 | plan.md says 16/17 and c05/c08 k_min 8–12; summary is **stale**. | `c2sva prove-report` after the last merge; add the generation timestamp + code hash to every generated table. |
| runs/paper_tables.md vs E4 summary | ACSLBench Checked 352 vs 333; mutation score 79.6 vs 80.1 %; ABE 92.5 vs 91.8 % | Two aggregators disagree (definition of "checked", re-aggregation after merges). | One aggregator, one definition, regenerate from run.json. |
| research_log R27 (planned abstract block) | "kills ... 305/305 ... confirmed" etc. | Fine, but the block does not mention that every original passes only after ~10 translator fixes driven by the same benchmarks (see threats). | Add a development-process paragraph. |

---------------------------------------------------------------------------------------------------

## 5. Cheap improvements to usability and reporting

1. **One-command front door.** `c2sva check file.c --top f` that auto-selects tool/config, writes a one-page *assurance card*: contract clauses translated / dropped (with reason and
   suggested fix), N, BMC depth, bounded vs unbounded (with rho), latency bound, access class (`ok/speculative/out-of-memory`), covers (per-clause), trusted assumptions (T1–T4), tool
   versions and code hash. Exit codes for CI (0 pass, 1 fail, 2 inconclusive). (1 pd)
2. **Per-clause provenance.** Every `props.sva` property carries the ACSL clause id and source line; failures print "ensures at foo.c:12 violated at step 31"; a VCD with C-variable
   names (from `interface.json`) rather than RTL names. (1 pd)
3. **Standalone checker export.** Emit a `bind`-able SVA checker module (plus a small C-to-RTL variable map) that runs in Questa/VCS/Xcelium/Verilator and in cosim testbenches; today the properties live in
   SBY harnesses. This is also the natural bridge to the co-sim baseline (X2) and to hardware-in-the-loop monitors; the problem statement title says SVA/LTL. (2 pd)
4. **Failure-to-test.** For any RTL counterexample, emit the C test case (inputs, expected vs observed outputs) and a Vitis cosim testbench that reproduces it. Replay already produces the witness. (1 pd)
5. **Contract hygiene checks** before HLS: precondition satisfiable, contract consistent (some post-state), axioms consistent (automate the `lemma \false` probe), `reads` clauses present on logic functions
   (Frama-C warns), loop assigns present. Surfaced three ACSLBench defects by hand. (1–2 pd)
6. **Vacuity report.** Antecedent covers for every implication, behavior and quantifier range; report "vacuous clauses". Cheap in Yosys (additional `cover` tasks). (1 pd)
7. **Reproducibility.** `run.json` with versions (Vitis/Frama-C/yosys/solvers), git hash, config hash; cache HLS output by source+directives hash; `c2sva doctor`; one aggregator for all tables
   (kill the paper_tables vs summary discrepancy); every table footer shows run ids and timestamp. (1 pd)
8. **Result schema.** Stable JSON/SARIF so results can be consumed by dashboards; JUnit XML for CI. (0.5 pd)
9. **Config-diff report.** `c2sva diff run1 run2` listing verdict changes per design/config (will make RQ5 and regression testing trivial). (0.5 pd)
10. **LLM provenance.** `llm.py`: log model id, temperature, prompt hash, response hash, retries; offline cache keyed by prompt hash so reviewers can replay without a key; separate
    "LLM-proposed" from "validated" artefacts in the report. (0.5–1 pd)
11. **Time-budget flag.** `--budget` with automatic escalation (BMC -> k-induction with certified invariants -> latency proof) and an honest `bounded(k, Tsec)` report; plus progress logging. (1 pd)
12. **Directive hints.** When `out-of-memory`/`speculative` is reported, print a one-line remedy (array size, `bind_storage`, interface choice). (0.5 pd)

---------------------------------------------------------------------------------------------------

## 6. Prioritised TODO list

| Pri | Task | Effort | Output |
|---|---|---|---|
| P1 | **Reconcile numbers and docs** (section 4): single aggregator, regenerate prove_summary and paper_tables, fix stale limitations (structs, axiomatic), adjust abstract wording, add defect table for ACSLBench | 0.5–1 d | Consistent paper_tables, honest abstract |
| P1 | **X1 + X2**: injected pragma/RTL faults + co-sim/random-testing baseline, paired comparison (McNemar) | 6–9 d | Table "what c2sva finds that testing and C proof do not" |
| P1 | **X3**: 8–12 realistic kernels (MachSuite/CHStone) with LLM-written WP-validated contracts, m_axi interface included | 8–12 d | Held-out evaluation; real-size runtime table |
| P1 | **X6 + X7**: LLM-only SVA baseline, held-out lifting set with frozen prompt, NL->ACSL on ACSLBench `nl`; logging for reproducibility | 5–8 d | RQ6 table with false-accept rates |
| P1 | Freeze a tagged commit and **re-run E1–E6 end to end** (including E5 on new code, ACSLBench configs sample X12) | compute 1–2 d + 1 d | Single provenance for all numbers |
| P2 | **X4** m_axi + wait-state memory + back-pressure (also tests the speculative-read hazard where it is real) | 5–8 d | New RQ3/RQ4 evidence, T-spec resolution |
| P2 | **X5** Bambu (or XLS) on conformance + 20 ABE designs; otherwise remove tool-agnostic claim | 3–5 d | Second-tool table |
| P2 | **X9** global variables model | 2–4 d | Coverage +~57 clauses |
| P2 | **X10, X14, X15** (interface-map sensitivity, contract-consistency/vacuity, timeout/seed variance) | 3–4 d | Trusted-base and statistics sections |
| P2 | **X8** series N = 2..16 for the proof-depth claims (full array-memory harness optional) | 1 d (series) / 5–8 d | Decoupling claim tested |
| P2 | **X17** LLM-rewritten HLS-C vs original contract | 4–6 d | Strongest RQ5+RQ6 story |
| P2 | Statistics: Wilson CIs on all rates, per-operator mutation breakdown, clustered permutation test for the (II,D) predictor, rule-of-three on false alarms, cost-vs-timeout curves | 1–2 d | Section "Statistical treatment" |
| P2 | Usability items 1–4, 7 (check front door, provenance, checker export, failure-to-test, reproducibility) | 4–6 d | Tool artefact; artifact-evaluation ready |
| P3 | X11, X12, X16, X18, X19 (more directives, full ACSLBench configs, finish QED/AutoINV baselines, more mutation operators, fixed-point) | 8–15 d | Breadth |
| P3 | Mechanise Lemma 1 (Coq/Isabelle) or at least expand the proof per construct | 5–10 d | FMSD-level theory |

**Suggested journal narrative after the P1 items:** "Contracts survive synthesis" -> (1) finite-state transaction semantics + polarity-sound translation, no false alarms on N benchmarks;
(2) contracts catch HLS-introduced faults that co-simulation with equal budget misses (X1/X2); (3) unbounded proofs and the window/inductive classification as a *measured* statement;
(4) formal validation is what makes LLM assistance safe (X6/X7). Without (2) the paper reads as a well-engineered translator with no demonstrated use; with it, the evaluation answers the reviewer's
first question.
