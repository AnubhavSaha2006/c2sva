# c2sva — the ten strongest novel ideas (consolidated from research_log R1–R32)

*Compiled 2026-11-21 by the background research agent. Sources: docs/research_log.md (Rounds 1–32), plan.md tail (status), runs/** (read-only). Citations marked [v] were confirmed by search in this project; [unv] not confirmed. Status = state in c2sva as of plan.md "2026-11-21 midday". Scores: N = novelty, I = impact, F = feasibility (1–5); rank = N·I·F-weighted judgement, not a formula.*

| Rank | Idea | N | I | F | Status |
|---|---|---|---|---|---|
| 1 | Quiescence rule for source invariants in modulo-scheduled pipelines (δ, commit count, phases, coupling) | 5 | 5 | 5 | done on E1; E3 rerun pending |
| 2 | Window theorem and the k_min / ρ / F methodology ("what an unbounded proof really means") | 4 | 5 | 5 | measured; theorem text only in log |
| 3 | Certificate repair: incremental re-verification and portability across HLS configs, plus a proof-carrying RTL bundle | 4 | 5 | 4 | not started (data ready) |
| 4 | Contract-oracle bug finding in HLS flows: pragma audit, config/tool/version differential, report validation | 4 | 5 | 4 | partly (E2/E5 differential; QED-FC running) |
| 5 | Certificate reuse across source rewrites, incl. LLM rewrites (S3) with faults vs testing/snapshot baselines | 4 | 5 | 3 | not started |
| 6 | Size-independent verification: array-memory harness, Skolem tracked cells, deployment-specialised contracts, aliasing modulo 2^w | 4 | 4 | 3 | not started |
| 7 | LLM lifting under formal guardrails (S1) with bounded-model validation; benchmark-defect detection | 3 | 4 | 5 | done (WP route), BME/FIV route not done |
| 8 | Source proof as an untrusted partial witness: structural vs semantic split of a certificate | 4 | 4 | 5 | partly (binding aliases, ablation) |
| 9 | Contract-guided HLS: verified facts → pragma/optimisation hints, with the check closing the loop | 5 | 4 | 2 | not started |
| 10 | CTI-guided LLM lemma proposer (S2), evaluated retrospectively | 3 | 3 | 4 | not started |

Ordering rationale: 1–2 are results already in hand that need only writing/rerun and are the paper's core; 3–5 are cheap-to-medium experiments on existing infrastructure with strong narratives; 6 is the main answer to the "N=4" criticism; 7–8 are completed or framing contributions; 9 is the most original but needs new engineering; 10 is optional RQ6 evidence.

---

## 1. Quiescence rule for source loop invariants in pipelined RTL
**What.** A source (ACSL) loop invariant holds verbatim on RTL only if every variable and memory it mentions is *quiescent* at the loop-header cycle. For a variable defined in pipeline state s_def, the number of earlier iterations whose update is still in flight is δ(v)=⌊(s_def−1)/II⌋ (read from `Pipeline-n : II, D` and the `ST_s` of the alloca/array `store` in `*.verbose.sched.rpt`). The exact, phase-independent form is a *dynamic commit count* c_eff = i − Σ_{j=1..δ} v_{j·II} (v_s = `ap_enable_reg_ppX_iter<s>`), instantiated as I[i ↦ c_eff]; the epilogue is handled by an exit ghost (`ap_condition_exit_*`/`ap_ready`), exit and post-persistence facts on frozen register taps, write-port instances, and a harness–RTL coupling family (`h_busy ⇒ ¬ap_idle`, `v_s ⇒ h_busy`). Together they make source invariants inductive in overlapped (II=1, D≥2) pipelines.
**Why novel.** Puri–Ray–Hao–Xie (ACL2 2014, EPTCS 152, pp. 111–128) [v] prove sequential↔pipelined *equivalence* with a hand-built correspondence invariant (k iterations plus a staircase of partial ones); Tristan–Leroy (POPL 2010) [v] validate software pipelining by symbolic evaluation; Namjoshi–Zuck (SAS 2013, pp. 304–323) [v] transport invariants through transformations with a witness. None derives per-variable lags from a *black-box* commercial schedule and uses them to instantiate *source contracts' loop invariants* for a model checker, nor reports a statistical predictor (II,D,δ) of survival.
**Status.** Done for E1: lag candidates, `loop-invariant-store`, `pipeline-copy`, R-phase, coupling, call-age facts; c08, c05, c14 inductive at k_min 8 (L+1=9); c07 control 2. E3 (54 loop designs): correlation II=1,D≥2 ⇒ 0/20 inductive vs 18/21 otherwise (Fisher p≈7.5·10⁻⁹) *before* these fixes.
**Impact.** The paper's mechanism result: explains when source proofs survive HLS and fixes it by a schedule-derived rule; answers RQ2/RQ5.
**Effort.** S for the rerun; M for the write-up and the dynamic c_eff on non-function-level loops (loops in longer FSMs, callee sub-modules).
**Experiment.** Rerun E3 with all families: predict inductive ≥30/50 (E1+E3) vs 21/50 before; per-design table (II, D, δ_max, kept lag vs δ from report ≥90 % agreement); `nopipe`/`unroll` config pairs as the controlled intervention (δ=0 ⇒ verbatim pool works); negative control lower_bound/upper_bound/binary_search (relative lag 0 ⇒ no gain from lags).

## 2. Window theorem and the k_min/ρ/F methodology
**What.** If the structural strengthening gives window-locality (every violation is witnessed by a call that started within L steps) and idle bisimilarity after F steps, then k-induction at k ≥ F+L+1 succeeds iff the property holds, and its base case is BMC to depth k (the property is then settled by BMC to depth d+L, d≤1 when reset is bisimilar to every accepting state): such proofs are *BMC to the call diameter*, valid for the synthesised N only. Hence report k_min per candidate pool, ρ = k_min/(L+1) and F = k_min−L−1; only ρ<1 certificates carry inductive content. Measured: plain k-induction fails to k=32 on all L≥8 loop designs; structural proofs k_min = L+F+1 with F≈4 constant (c03 13, c07 13).
**Why novel.** Completeness thresholds/recurrence diameters (Kroening–Strichman VMCAI 2003; Clarke et al. VMCAI 2004; Kroening et al. CAV 2011) [v] exist for general BMC; applying them to *explain and honestly measure* k-induction with HLS-structural invariants (AutoINV-style helpers, arXiv 2604.22285 [v] report speed-ups only) and to separate "unbounded in calls" from "size/latency-independent" is new. It also protects the paper against the standard reviewer objection.
**Status.** k_min strategy, fine ladder, E1 table done; E7/E6 N-scaling running; theorem text exists in research_log R10/R18 (verify whether docs/theory.md has it — I did not edit it).
**Impact.** Methodological credibility; an unusual, falsifiable claim (F constant, slope 1 in L).
**Effort.** S (write-up, one figure: k_min vs L with the y=L+F+1 line).
**Experiment.** k_min(N) for N=2,4,8,16 on 16 loop designs; pre-registered decision rules in R28 (P-N1..P-N9); idle-miter check of the bisimilarity hypotheses on 3 designs; adding response-bound lemma to the structural pool should give F≤1.

## 3. Certificate repair: incremental re-verification and a proof-carrying RTL bundle
**What.** After any change (HLS directive, source edit, tool version), reuse the previous certificate J: map it through the binding map, check J∧P at the previous k in one k-induction run, *repair* (drop failing, add δ/phase variants, rerun Houdini only on the delta). Package contract + RTL hash + J + k + a 30-line checker script as a *proof-carrying RTL bundle* a consumer re-checks in seconds with a different solver, without Vitis/Frama-C.
**Why novel.** Proof-carrying hardware with inductive invariants exists (Drzevitzky–Kastens–Platzner ReConFig 2009 [v]; "Proof-Carrying Hardware via Inductive Invariants", ACM 10.1145/3054743 [v, authors not read]), and Kairos does incremental *equivalence* across HLS variants (FMCAD 2019) [v]. New here: the certificate is seeded by a *source* contract proof, its reuse across HLS configurations is governed by the (II,D,δ) rule, and the quantity measured is *certificate portability* (fraction of J reused, speed-up vs from scratch).
**Status.** Not started; all raw data exist (E2 11 configs × 17 designs; E5 6 configs × 72 designs).
**Impact.** Turns the framework from "one-shot checker" into a regression tool for design-space exploration (the HLS use case: dozens of directive variants).
**Effort.** S–M (a `c2sva reprove --from CASE_DIR` using binding map aliases).
**Experiment.** For each design × config pair: time and #SAT calls to certify from scratch vs from the base-config certificate; fraction of J reused verbatim / after lag-phase variants; bundle re-check time by an independent solver; failure analysis by (II,D,δ). Expected: ≥60 % reuse where the pipeline class is unchanged, 3–10× speed-up.

## 4. Contract-oracle bug finding in HLS flows
**What.** Use the contract as the spec for *bug finding in the HLS flow itself*: (a) **pragma audit** — seed unsafe pragmas (UG1399: a false `dependence … inter false` "can result in incorrect hardware" [v]; wrong `array_partition`/unroll with races; undersized `stream` depth in dataflow) and check whether c2sva/QED-FC detect the resulting RTL misbehaviour where C/RTL co-simulation on a few vectors does not; (b) **config differential** — same contract across 11 (+6) directive configs and Vitis versions: any verdict change is a bug or an interface-semantics difference (already: speculative reads become OOB only with `bram`); (c) **tool differential** — the problem statement lists Bambu and XLS; with a Bambu interface model, the same contracts expose disagreements between tools; (d) **report validation** — treat csynth latency/II/depth claims as properties and prove or refute them on the RTL (we already prove response bounds: 16/17).
**Why novel.** Herklotz–Wickerson (Verismith, FPGA 2020) [v] fuzz FPGA *logic* synthesis tools with random Verilog and equivalence checking; A-QED/G-QED find bugs in accelerators without spec [v]. No work uses *source-level contracts* as an oracle to fuzz HLS tools and their directives; contracts also catch bugs that preserve C-RTL equivalence of the sampled vectors.
**Status.** Partly: E2/E5 give 0 failing originals in 432+ checks, the speculation finding, QED-FC baseline `c2sva qed` running (E1 originals+mutants). Pragma seeding and Bambu not started.
**Impact.** The most "practical" claim; yields concrete bug reports and a safe-directive checker.
**Effort.** M for (a)+(b) (generator of directive variants + the existing pipeline), L for (c).
**Experiment.** 20 designs × 8 seeded unsafe directives (e.g. `dependence inter false` on copy_backward/remove loops with true dependences): detection rate of c2sva vs co-simulation with 10 random vectors vs QED-FC; classification of detections by C replay; report any Vitis behaviours that violate documented semantics.

## 5. Certificate reuse across source rewrites, including LLM rewrites (S3)
**What.** Rewrites for HLS (unroll, tile, pointer-walk, local buffer, LLM "optimise for II=1") keep the *contract*; the C proof's abstract-state invariants (prefix predicates over a progress measure) are the most stable part. Check the rewritten function's RTL against the *original* contract and certificate pool (name-free binding + trace-fitted affine remap), with no re-annotation or WP rerun; compare fault detection on seeded rewrite faults against C2HLSC-style testing and an SVApshot-style snapshot of the original RTL.
**Why novel.** Pouchet et al. (FPGA 2024) verify C→HLS-C equivalence at source; Kairos (FMCAD 2019) checks RTL↔RTL equivalence vs a trusted reference; C2HLSC/Shift-Left agents validate LLM rewrites by testing; SVApshot (arXiv 2611.12712) [v] snapshots existing RTL. None combines a *source contract proof*, unbounded certificates and LLM-written HLS-C.
**Status.** Not started. Protocol in research_log R5/R30 (≥15 designs, R1–R7 scripted, 3 LLM samples, seeded faults, bounded-exhaustive C oracle).
**Impact.** Strongest RQ5+RQ6 story ("a formal safety net for LLM-written HLS-C"); also tests certificate portability.
**Effort.** M–L (rewrite scripts + LLM harness + agreement matrix).
**Experiment.** Agreement matrix RTL verdict × C-oracle label; detection of seeded faults at equal wall time vs random co-simulation; certificate-reuse ratio per rewrite class; RTL-identical fraction.

## 6. Size-independent verification
**What.** (i) *Deployment-specialised contract*: state `Pre_D = Pre ∧ ranges ⊆ [0,D)` so the claim is about the circuit Vitis builds for depth D (N=4 is a different circuit: address width 2); (ii) *array-memory harness*: a Verilog `mem[0:D-1]` (SMT array) plus tracked Skolem cells for the quantified indices and per-index snapshots instead of N cell registers — exact for clauses universal in assertions / existential in assumptions (c2sva's polarity lemma already turns these into symbolic indices); lazy instantiation of ∀-assumptions (sortedness) on addresses the RTL reads; (iii) *aliasing modulo 2^w*: speculative reads wrap benignly for power-of-two D and hit real out-of-range cells otherwise — a size-dependent semantic effect any cutoff argument must carry; (iv) *family certificate* with a structural-uniformity witness and an explicit trusted assumption that Vitis is uniform in D.
**Why novel.** Array abstraction by cell/Skolem tracking is known (Monniaux–Alberti SAS 2015 [v]; Mann et al. TACAS 2021 [unv]; Conchon–Goel FMCAD 2013 invariants-for-finite-instances [v]); here the array lives *behind the HLS interface* so the abstraction is exact without touching the RTL, and the object is the *deployed* circuit.
**Status.** Not started (E6/E7 run concrete N=2,8,16).
**Impact.** Removes the main generalisation objection; unbounded-N proofs for the ∀-fragment.
**Effort.** M–L (harness + translate index bookkeeping).
**Experiment.** Time vs D ∈ {4, 64, 1024, 65536} flat vs exponential today; verdict table at deployed D; access-outcome table vs D mod 2^w (D=5,6,1000,1024) for `is_sorted`-like designs.

## 7. LLM lifting under formal guardrails (S1) and benchmark-defect detection
**What.** An LLM lifts axiomatic/inductive definitions the rules reject into explicit definitions; acceptance only after validation (WP unfolding lemma + consistency probe; planned: bounded-model enumeration / finite instantiation of the axioms for N cells, complete for the translator's N-cell semantics) with the domain conjoined by the polarity rule. By-product: inconsistent or `reads`-less axiomatics in ACSLBench (factorial 13/216, Ordered, count of task_452 — confirmed with Frama-C) make some "proved" benchmark tasks vacuous.
**Why novel.** LLM→SVA papers (AssertLLM, FVEval, nl2sva, Spec2SVA-Eval) translate natural language without a soundness discipline; here the LLM produces *definitions* validated against the source semantics and the translator's soundness rule, and the machinery exposes benchmark defects.
**Status.** Done (10 names; repair loop; guardrail test 11/12 wrong seeds rejected, 1 equivalent accepted, 0 false accepts); BME/FIV route not implemented; full 111 rejected clauses not yet run.
**Impact.** Answers RQ6 concretely and adds a benchmark-validity finding.
**Effort.** S (BME/FIV) + S (full run).
**Experiment.** Run on all 111 rejected clauses; metrics: recovered clauses, route used, inconsistent/vacuous count, first-try/after-repair syntax rate, false-accept rate on seeded wrong definitions.

## 8. The source proof as an untrusted partial witness: structural vs semantic split
**What.** View Vitis' schedule/binding reports plus RTL names as an untrusted, partial refinement witness (Namjoshi–Zuck [v]): soundness comes from checking the transported facts on the RTL (Houdini + k-induction), so a wrong witness costs only completeness. A certificate splits into a *domain part* (control skeleton, affine register relations: derivable from structure, AutoINV-style) and a *semantic part* (history predicates over memory: only the source proof supplies them). Ablation: plain k-induction fails (>32); structural alone gives window-covering proofs; C artefacts give inductive ones where the header is quiescent.
**Why novel.** Witnesses are emitted by verified compilers; here the witness is *inferred from a black-box commercial tool*, the target property is a contract (weaker than equivalence), and its value is measured (k_min, minimal set).
**Status.** Partly: schedule aliases and per-module placement implemented, ablation by pool done; minimal-set/residue (delta-debugging) and the "post-establishing invariant" test not done.
**Impact.** The paper's framing sentence and the structure of §7.
**Effort.** S.
**Experiment.** Residue ρ_res = |minimal certified set| − |structural-only minimal set|; WP ablation marking post-establishing invariants; correlation with gain (R19 table).

## 9. Contract-guided HLS: verified facts → optimisation hints, with the check closing the loop
**What.** Today HLS optimisation hints are user promises (UG1399 warns that wrong `dependence` can produce incorrect hardware [v]). A proved contract already contains legality facts: `\separated(a,b)` ⇒ `dependence variable=a inter false`; `requires 0<=n<=N` and element ranges ⇒ `ap_int` widths and `loop_tripcount`; `assigns` frames ⇒ safe array partitioning/dataflow splitting; loop invariants ⇒ pipelining legality. Generate the pragmas from the contract and then **verify the optimised RTL against the same contract** (and the k_min/phase machinery for the pipelined loop): the pragma is justified by the source proof, and the RTL check catches tool or translation errors.
**Why novel.** Compiler-based pragma insertion (ScaleHLS, polyhedral tools) trusts its analysis; Pouchet et al. verify source-to-source rewrites; no work derives pragmas from *deductive contract proofs* and re-checks the generated hardware against the contract. Related caution: wrong hints are a known bug source.
**Status.** Not started.
**Impact.** Closes the loop between verification and optimisation; a distinctive "verified-by-construction hints" story; speed-up measured as II reduction.
**Effort.** L (pragma synthesis rules, QoR measurement; start with `dependence` and bit-width narrowing).
**Experiment.** ABE designs with `separated`/range preconditions: auto-insert hints; report II/latency/area gain; c2sva verdict on every optimised variant (must PASS); seeded unsound hints (violating the precondition) must FAIL/c-confirm.

## 10. CTI-guided LLM lemma proposer (S2)
**What.** Feed a k-induction counterexample (cone-restricted signals, schedule facts, candidate grammar) to an LLM; it proposes ≤8 candidate invariants in the restricted IR; guardrail: parse → BMC base filter → Houdini. Retrospective evaluation: replay the CTI history of c08/c05/c14 (phantom busy, drain/post, lag) and measure whether the LLM re-derives the hand-built families.
**Why novel.** ICE learning (Garg et al., CAV 2014 [v]) and data-driven invariants learn from CTIs with synthesis, not LLMs; LLM invariant generation (AutoSpec-style) targets C loop invariants with WP validation; here the target is RTL candidates under k-induction with HLS vocabulary.
**Status.** Not started.
**Impact.** Moderate (RQ6 evidence; may extend to equal_range/unproved E3 designs).
**Effort.** S–M.
**Experiment.** Hit rate vs manual families; unproved E3 designs solved within ≤3 CTI rounds.

---

## Also considered (not in the top ten)
* **Cut-point/Hoare proofs over schedules** (segments = prologue/RUN/DRAIN/POST, callee contracts as summaries; Kairos FMCAD 2019 and Behnam–Alizadeh–Taheri arXiv 1712.09818 are region/cut-point precedents [v]): high novelty, effort L; would solve equal_range; the R-phase machinery is its first instance.
* **Range-certified width reduction and multiplier congruence** for c10 (R4; Cimatti et al. TACAS 2017 / TOCL 2018 [v]; Coward et al. e-graph papers [v]): effort M.
* **Proof-based equivalent-mutant classification:** mutants that A1 can prove contract-preserving are equivalent w.r.t. the contract (instead of "no violation found"); applies directly to the 76 E4 and 8 ABE survivors; effort S; relates to trivial-compiler-equivalence for mutants [unv].
* **Example-pruned cone-sliced worklist Houdini** (H-Houdini, ASPLOS 2025 [v]): engineering scalability (c12-type timeouts).
* **QED-FC arm / T2 as call-level strong FC** (A-QED², G-QED [v]): positioning and a bug-finding comparison.
* **ABE-pipelines dataflow benchmark and channel-content invariants** (FIFO contents ↔ producer prefix) for the AutoINV home ground.
* **Family certificate with uniformity witness** (part of idea 6).

## Dependencies and suggested order
1 → 2 → (3, 4 in parallel; both reuse binding map and E2/E5 configs) → 5 → 6 → 7/8 (writing) → 9, 10 optional. A paper can be built on 1+2+3+4+7+8 with 5 and 6 as the extension that answers the two likeliest reviewer objections (N=4; "LLM role").
