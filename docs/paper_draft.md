# Contracts Survive Synthesis? Checking ACSL Function Contracts on HLS-Generated RTL

*Draft — numbers marked ⟨…⟩ are filled from `runs/paper_tables.md` after the evaluation campaign.*

## Abstract
High-level synthesis (HLS) turns C functions into hardware, but the formal contracts that many C
code bases carry (preconditions, postconditions and frame conditions, written in ACSL and proved
with Frama-C) are not checked against the generated RTL. We present c2sva, a procedure that
translates ACSL function contracts into transaction-level properties of HLS-generated RTL and
checks them formally. It depends only on the HLS tool's interface report and is evaluated with
AMD Vitis HLS.

* **Interface model.** C objects are related to RTL ports through a model read from the tool's
  report. It covers partitioned, reshaped, byte-addressed and per-element memory layouts,
  pipelined and overlapping calls, and structs.
* **Translation.** Quantifiers, logic definitions, behaviors and frame conditions are translated
  under an explicit soundness discipline: assumptions only get weaker, assertions only stronger.
* **Classification of violations.** Every reported violation is classified at the C level by
  replaying the counterexample with E-ACSL, sanitizers and a concrete contract evaluator.
* **Unbounded proofs.** The loop invariants of the C proof are carried to the RTL as candidate
  invariants, certified by Houdini, and used to prove contracts and latency bounds for all
  executions (arrays of N cells). Each proof is reported with its minimal induction depth
  relative to the call latency, so proofs with inductive content are separated from those that
  amount to checking a whole call.
* **LLM assistance.** An LLM lifts axiomatic definitions the rules cannot translate. A lifted
  definition is admitted only when Frama-C proves an induction-free unfolding lemma from the
  original axioms. None of 11 seeded wrong definitions was admitted.

**Results.** On ⟨N⟩ functions from ACSL by Example and ACSLBench:

* c2sva translates ⟨x⟩ % of the contract clauses (⟨d⟩ % of the designs fully);
* it verifies ⟨y⟩ % of the checked designs up to the BMC bound (N = 4 cells), and ⟨u⟩ % for
  all executions, ⟨i⟩ of them with inductive content;
* it kills ⟨z⟩ % of the non-equivalent C-level mutants, with each kill classified at C level;
* with deliberately unsafe dependence pragmas, it detects ⟨f⟩ hardware faults that the C-level
  proof cannot see, while a specification-free consistency check (A-QED style) detects none of
  the functional faults.

The checks also exposed three ACSLBench tasks whose "proofs" are vacuous because of inconsistent
or memory-independent axiomatisations.

## 1 Introduction
* HLS is now mainstream; verification of HLS output relies on C/RTL co-simulation and, in
  industry, sequential equivalence checking against the C model.
* Much C code that is synthesised is *specified*: safety- and security-relevant libraries carry
  ACSL contracts proved with Frama-C. These specifications are weaker than the code (they state
  what must hold, not how), which makes them natural verification targets for the generated
  hardware — but they are lost at the C/RTL boundary.
* Translating them is not a syntactic exercise (RQ1–RQ5): an untimed call becomes a handshake
  transaction of variable latency, possibly overlapping with other calls; C variables become
  ports whose names, widths and layouts depend on optimisation directives; arrays live outside
  the generated module behind memory ports; quantifiers, logic functions and frame conditions
  have no direct RTL counterpart.

**Contributions.**
1. A transaction semantics for ACSL contracts over HLS block-level protocols, including
   overlapping calls of pipelined blocks (§3).
2. A layout-generic interface model that relates every C object to RTL ports using only the
   HLS report, covering partition, reshape, complete partition, BRAM and struct packing (§4).
3. Translation rules for the ACSL fragment used in practice — quantifiers, aggregates,
   overloaded and recursive logic, labels, behaviors, exact frame conditions — with a polarity
   discipline that keeps every approximation sound (§5).
4. Independent C-level validation of every violation by counterexample replay (E-ACSL + a
   concrete evaluator) (§6).
5. An evaluation on ⟨N⟩ benchmark functions, 11 Vitis optimisation configurations and ⟨M⟩
   automatic mutants, and the observation that Vitis' speculative reads violate memory-safety
   contracts in a way that depends on the interface choice (§7).

## 2 Background
ACSL contracts; Frama-C/WP; E-ACSL; Vitis HLS block protocols (ap_ctrl_hs/chain), interface
modes (ap_memory, bram, ap_none/vld/ovld), array directives; SymbiYosys BMC.

## 3–6 Approach
See `docs/methodology.md` (§3 front end, §4 interface model, §4.1 call semantics, §5 rules,
§6 checking, §7 validation, §8 mutation). The paper version condenses it to: Figure 1
(pipeline), Table 1 (clause → property), Table 2 (layouts and `locate`), the polarity rule, the
statement of what a PASS means, and the replay classification.

## 7 Evaluation
**Benchmarks.** 17 conformance designs (one per rule, all WP-proved); ACSL by Example (82
functions, proofs by the authors); ACSLBench (598 tasks, 419 fully proved by Frama-C/WP with
Alt-Ergo, Z3, CVC5). **Setup.** Vitis HLS 2026.1 (Artix UltraScale+, 10 ns), SymbiYosys with
Yices/Bitwuzla, Frama-C 33, N = 4 memory cells, latency cap 128, 600 s per formal task.

* **RQ1/RQ2/RQ4 — how much of a contract can be carried to RTL?** T1 funnel and clause rate;
  T8 reasons (global state, axiomatic logic, unsupported C types).
* **Correctness.** T2: outcomes on originals, agreement with the C-level proofs; every RTL
  failure on a proven original is replayed (none expected; any is a finding).
* **Property strength.** T3: mutation score on non-equivalent mutants; equivalent mutants must
  pass; kills confirmed at the C level.
* **RQ3 — latency.** T6: latency bounds established formally where the report has none;
  overlapping-call designs.
* **RQ5 — robustness across optimisations.** T4/T5: the same properties hold under 11
  configurations; only the interface model changes. Finding: speculative reads.
* **Scalability.** T7 (N = 2/4/8), T9 run time per stage; nonlinear arithmetic (triple products)
  remains bounded.
* **Unbounded proofs from the C proof (A1).** T10: per design, plain k-induction, IC3/PDR, and
  k-induction with the Houdini-certified invariants mapped from the C proof. Headline numbers:
  ⟨unbounded⟩/⟨cases⟩ designs proved for all executions, ⟨only with C artefacts⟩ of them only
  with the C-derived invariants, ⟨bmc-bounded now unbounded⟩ of the ⟨bmc-bounded⟩ designs that
  BMC could only bound. *Survival rate:* ⟨c-kept⟩/⟨c-total⟩ C loop invariants hold verbatim on the
  RTL for some register placement and FSM state: an empirical measure of how far Vitis preserves
  the program's proof structure. *Ablation:* HLS-structural candidates alone / + contract-derived
  / + C proof artefacts.

## 8 Threats to validity
* Bounded verification (N cells, BMC depth) for the main campaign: results are for arrays of at
  most N elements and executions of bounded length, except the designs proved unbounded in
  depth by A1 (still N cells). Mitigation: N-sensitivity study; latency bound established
  formally.
* The translator and the concrete evaluator share the parser and logic inlining; E-ACSL is fully
  independent for the constructs it supports.
* Mutants are first-order; equivalence decided by Frama-C/WP, which is incomplete (a mutant
  whose proof fails may still be equivalent).
* One HLS tool (Vitis). The procedure only depends on the report and port conventions. A Bambu
  back end exists as a prototype, last exercised on 3 demo designs before the current
  interface model.
* Benchmarks double as development set: about ten translator fixes were driven by failures on
  these suites (all are listed in plan.md). A held-out set and realistic kernels (MachSuite /
  CHStone, m_axi) are future work.
* Environment assumptions are stronger than the block protocol alone: memory is quiescent during
  a call, and RAM read latency is fixed (ap_memory). Results hold under these assumptions.
* Mutants are C-level faults. Their kills measure specification strength, not hardware bug
  finding; RTL-level faults are studied separately (unsafe pragmas, E8).

## 9 Related work
See `docs/related_work.md`: in-HLS assertion synthesis (Curreri et al.; Ribon et al.;
Catapult), C/RTL equivalence (SLEC, DEEQ) and verified HLS (Vericert), TLM→RTL assertion reuse
(Bombieri et al.), LLM-based SVA generation (AssertLLM, nl2sva, AssertionForge).

## 10 Conclusion and future work
LLM-assisted lifting of axiomatic definitions with Frama-C-checked equivalence; m_axi/AXI-stream
interfaces; size-independent RTL for unbounded N; global state; LLM strengthening of invariants
from counterexamples to induction (A3).
