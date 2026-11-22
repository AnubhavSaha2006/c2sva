# c2sva — research directions for a journal-grade contribution

*Literature survey and proposal, 2026-11-20. Every reference in §6 was found and checked
(title, authors, venue) by web search during this survey. Items marked **[unverified]** could
not be confirmed in full. Nothing in the code base was changed and no tool was run for this
report, except reading Vitis reports already in `runs/` and Frama-C 33 sources under `~/.opam`.*

---

## 0. Executive summary

c2sva already establishes a new problem (carrying ACSL function contracts across a production HLS
tool and checking them on the RTL) with a careful bounded procedure. Reviewers at TCAD, FMSD or
FMCAD will raise three questions:

1. **Is it more than bounded checking?** Today it covers arrays of 4 cells, BMC depth and
   sequential calls.
2. **What is the formal correctness notion,** and is the translation sound with respect to it?
3. **What is new compared with A-QED, AutoINV and translation validation of HLS?** These are
   close works that `docs/related_work.md` does not yet cite (§1).

**Recommendation.** Combine idea **A1 + A2** (§2) into the journal paper:

* **A1 — Certificate translation through HLS.** Reuse the C-level proof artefacts that already
  exist (ACSL loop invariants, loop assigns, loop variants) as candidate *inductive invariants*
  of the HLS-generated RTL. The mapping comes from the Vitis schedule and binding reports.
  Together with HLS-structural invariants and a Houdini-style fixpoint, this turns bounded
  results into **unbounded proofs**: any number of calls, any latency, and, with a
  size-independent interface, any array size.
* **A2 — Contract preservation as a correctness notion for HLS.** This is the theory that A1
  needs. It gives a transaction semantics, an abstraction function from the interface model,
  soundness of the translation rules (the polarity discipline), and three lifting theorems:
  transaction induction, overlapping calls, and compositional callee contracts. It also opens a
  strong application: formally checking RTL produced from *rewritten* HLS-C (by hand or by an
  LLM) against the *original* verified contract. This is "spec-level translation validation",
  which full equivalence cannot provide.

**A3** (an LLM inside a single soundness discipline) becomes the RQ6 section. The finding about
speculative out-of-bounds accesses (§3.6) can also stand alone as a short DATE/DAC paper.

---

## 1. Closest prior work (new items to add to `docs/related_work.md`)

| Work | What it does | Gap relative to c2sva + A1/A2 |
|---|---|---|
| **A-QED** (Singh et al., DAC 2020); **A-QED²** (Chattopadhyay et al., FMCAD 2021); **G-QED** (Chattopadhyay et al., DAC 2023) | BMC of HLS-generated accelerators for *functional consistency* (same input ⇒ same output) without a specification. A² decomposes the design; G-QED handles interfering (stateful) accelerators. | No user specification: it cannot show that a result is *correct*, only that it is consistent. Its decomposition theorems are reusable for overlapping calls and unbounded call sequences (§3.5, A2). **Must cite.** |
| **AutoINV** (Zhou, Du, Hu, Sinha, Zhang, Zhang, arXiv 2604.22285, 2026) | Helper invariants for IC3 on Vitis-generated RTL, mined from HLS structure (FSM, FIFOs, loop bounds, execution order) and side-loaded into IC3Ref. | Uses no source *proof artefacts* and no contracts. The properties are user SVA. It is the natural baseline for A1. |
| **Xu & Josipović, ICCAD 2023**; PROMISE (Xu, Cortadella, Josipović, ICCAD 2025) | Inductive invariants for k-induction on HLS dataflow circuits, derived from HLS insight. | Targets handshake and optimisation properties, not functional contracts. |
| **Kundu, Lerner, Gupta, CAV 2008**; **Ray, Hao, Chen, Xie, Yang, ATVA 2009**; **Hao, Ray, Xie, DATE 2014** | Translation validation or certification of HLS transformations (scheduling, loop and function pipelining) by equivalence. | They prove code equivalence inside a research HLS tool. c2sva works with a black-box commercial tool and checks a specification that is weaker than the code. |
| Siemens patent US10515168 "Formal verification using microtransactions" (2014) | C-vs-RTL SEC for HLS, decomposed into sub-call "microtransactions". | Equivalence only. Shows that industry needs decomposition below the call level. |
| **ILA** (Huang, Zhang, Subramanyan, Vizel, Gupta, Malik, TODAES 2018/19) | Instruction-level abstraction of accelerators, with refinement maps (start/finish conditions) to RTL. | Hand-written ILA specifications, not C contracts. Its refinement-map formalism is the closest formal precedent for c2sva's transaction model and should be cited in A2. |
| **Certificate translation** (Barthe, Grégoire, Kunz, Rezk, SAS 2006 / TOPLAS 2009); **VOC/TVOC** (Zuck, Pnueli, Goldberg et al.); **Tristan & Leroy, POPL 2010** | Carry proofs or invariants through *software* compilers, including software pipelining. | Never applied across HLS, where the target is a clocked FSM with pipelined iterations. This is the core novelty argument for A1. |
| Pouchet, Tucker, Zhang, Chen, Pal, Rodríguez, Zhang, FPGA 2024 | Formal verification of *source-to-source* transformations for HLS (C to HLS-C equivalence). | Stays at the C level. A2's application covers the C-to-RTL step against the contract. |
| C2HLSC (Collini, Garg, Karri, TODAES 2025); Shift-Left HLS Verification LLM agent (arXiv 2606.17128); Evidence-Driven C-to-synthesizable-C (arXiv 2606.28409) | LLMs rewrite C into HLS-C; consistency is checked by *testing*. | No formal guarantee. A2's application provides one through the contract. |
| CREST (Tiemeyer, Melham, Kroening, O'Leary, arXiv 2019) | C reference *models* used for RTL property checking. | Reference code, not declarative contracts, and no HLS interface model. |

**Gap statement, supported by this search.** No published work (i) translates ACSL contracts
to properties of black-box HLS RTL, (ii) reuses deductive-verification artefacts from the C
proof as inductive invariants of the RTL, or (iii) defines contract preservation as a
correctness criterion for HLS with a soundness proof. Write "to the best of our knowledge"
and re-run the search before submission. The area is moving fast: there are many 2026 arXiv
papers on LLMs for HLS and SVA.

---

## 2. (A) Overall novel approaches

### A1 — Certificate translation through HLS: from C proof artefacts to unbounded RTL proofs  ★ recommended core

**Idea.** The benchmarks ship with WP proofs whose key ingredients are loop invariants,
`loop assigns` and `loop variant`s. These are exactly what an RTL model checker lacks:
k-induction and IC3 fail on c2sva harnesses because the contract assertion at `ap_done` is not
inductive. A1 maps each C invariant to RTL candidate invariants:

* **Loop header.** Loop `L` becomes an FSM-state predicate `H_L`, read from `ap_CS_fsm` and the
  `ap_ST_fsm_*` encodings.
* **Variables.** C variable `v` becomes an RTL expression. Example from
  `runs/E2b_struct_reshape/.../find_tag`: the schedule report attributes `%i = alloca` /
  `store_ln14` to `c16_struct_array.c:14`, and the RTL holds `i` in register `i_fu_56`.
* **Memory.** Memory terms go through c2sva's existing `locate` / pre / post / old views.

The result is a set of *candidate* invariants `H_L ∧ commit_L ⇒ I[e_v/v]`.

Three further ingredients complete the picture:

* **Houdini fixpoint.** A Houdini-style loop (Flanagan & Leino, FME 2001) keeps the largest
  inductive subset of candidates, together with HLS-structural invariants (AutoINV-style: FSM
  one-hot, pipeline valid bits, counter ranges). This subset is then used to prove the contract
  assertions by k-induction or IC3.
* **Pipelined loops.** Several iterations are in flight, so each invariant is instantiated at
  stage offsets `0..D` (D = pipeline depth from the report). This mirrors how
  TVOC/Tristan–Leroy validate software pipelining. Unrolled loops are easy: a C invariant holds
  at *every* iteration, so it holds at the subset of iterations that unrolling exposes. That is
  a clean robustness argument for RQ5.
* **Other loop clauses.** `loop assigns` becomes an RTL *frame invariant*: cells outside the
  loop-assigns set equal their `\old` snapshot throughout the loop. This is crucial for
  inductive reasoning about memory. `loop variant` becomes a *progress invariant* (the variant
  decreases at least every `II × stall-bound` cycles). That gives a formal latency bound for
  symbolic `n` and answers RQ3 without a latency search.

**Why novel.** Certificate translation and TVOC transfer proofs or invariants through *software*
compilers. AutoINV and Xu & Josipović mine invariants from *HLS structure*. Ray, Hao and Xie
certify *transformations* by SEC. No work transfers source-level deductive proof artefacts
through a commercial HLS tool to prove a *specification* on the RTL. Two outcomes are publishable
either way: (a) how many C invariants survive HLS as RTL invariants, under which optimisations;
(b) unbounded proofs where BMC is only bounded today (the 16 `bounded` ABE designs).

**Fit with the pipeline.**

* Front end: extend `acsl.py` to keep loop annotations, which are parsed but discarded today.
* `hls.py`: new correspondence extractor from `.autopilot/db/<top>.verbose.sched.rpt`,
  `<top>.bind.adb.xml` / `verbose.bind.rpt` and the loop table in `csynth.xml` (II, depth, trip
  count).
* `harness.py`: emit candidate invariants as named assertions (and, after proof, assumptions).
* `formal.py`: new `prove` stage (sby `mode prove`, k-induction via smtbmc; `abc pdr`; `pono` and
  `rIC3` are already installed in OSS CAD Suite).
* Replay stays the oracle for any counterexample to induction (CTI) that turns out to be a real
  counterexample.

**Implementation sketch (about 2–3 person-months).**

1. Correspondence map `(loop, var) → (state predicate, register, stage offset)`; validate it on
   the conformance designs by simulation against C traces.
2. Candidate generation and Houdini on k-induction.
3. Contract proof with the surviving invariants as lemmas.
4. Progress invariants from variants.
5. Fallback: CTI → LLM strengthening (A3).
6. Combine with §3.2 (size-independent RTL + tracked-cell memory) for unbounded N.

**Evaluation.**

* **Coverage.** Fraction of designs and clauses proved *unbounded*, per Vitis config (11
  configs), against today's bounded results.
* **Ablation.** HLS-structural invariants only (AutoINV-like baseline) / + C loop invariants /
  + `loop assigns` frames / + LLM.
* **Proof time** against BMC to the latency bound, as N and latency grow (N = 4…1024).
* **Survival rate.** Fraction of C invariants that hold verbatim at RTL, and why the others fail
  (speculation, retiming, register sharing). This is an empirical insight into how Vitis
  transforms programs.

**Risks.**

* Register naming and sharing after binding may break the correspondence. Mitigation: Houdini
  over several candidate placements; use invariants only as *hints* (IC3 lemma side-loading);
  fall back to bounded.
* Pipelined loops with pending memory writes. Mitigation: invariants over the harness memory
  model plus in-flight-write registers; stage offsets.
* Missing loop invariants in some ACSLBench tasks. Mitigation: AutoSpec-style LLM generation,
  checked by WP first.

### A2 — Contract preservation: a refinement theory for HLS with transaction-level lifting theorems

**Idea.** Formalise what c2sva checks, then prove that the procedure is sound and that it scales
up.

1. **Semantics.** The C contract is `C = (Pre, Post, Frame)` over C states. RTL transactions are
   defined by the block protocol (`ap_start`/`ap_ready`/`ap_done`/`ap_continue`). The interface
   model `ι` (`locate`, port map) defines an abstraction `α_ι` from RTL observations to C states,
   in the style of ILA refinement maps and Abadi–Lamport refinement mappings.
2. **Correctness notion.** The RTL *preserves* `C` under `ι` when every transaction accepted in a
   protocol-respecting environment with `Pre(α(pre))` ends with `Post ∧ Frame` on
   `(α(old), α(post))`. Relation to compiler correctness: a semantics-preserving HLS
   (Vericert-style) implies contract preservation, but contract preservation can be checked per
   design. It also applies when the RTL comes from *different* C. That makes it a *robust
   property preservation* notion in the sense of Abate et al. (CSF 2019), specialised to HLS.
3. **Theorems.**
   * **T1 (translation soundness).** The polarity rule, symbolic constants, enumeration, recursion
     cut-offs, free out-of-range cells and integer width W yield a harness whose PASS implies
     preservation for traces of length ≤ k and arrays of ≤ N cells.
   * **T2 (transaction induction).** Single-call correctness from reset, plus an *idle-state
     invariant* (every reachable idle state is contract-equivalent to reset, checked by a two-copy
     miter with k-induction), implies preservation for infinitely many sequential calls. This is
     A-QED's "single-action correctness + functional consistency" argument lifted to contracts.
   * **T3 (overlap).** Sequential preservation plus equivalence between overlapped and
     sequential execution (a self-composition miter, Burch–Dill flushing / G-QED) implies
     preservation for overlapping calls, including ones that *write* memory (§3.5).
   * **T4 (compositionality).** Callee RTL modules that preserve their ACSL contracts can be
     replaced by contract abstractions when checking the caller. This is WP's modular rule lifted
     to RTL and gives scalability (§3.1).

**Why novel.** Vericert proves full semantic preservation of one research compiler. TLM→RTL
property refinement (Bombieri et al.) and path-predicate abstraction (Urdahl, Stoffel, Kunz,
TCAD 2014) relate abstraction levels for hand-designed hardware. A-QED avoids specifications. No
work defines and mechanises preservation of *function contracts* across HLS, with interface
layouts and variable latency.

**Killer application (evaluation).** *Spec-level translation validation of HLS-C rewrites.*
Take WP-proved functions, produce HLS-oriented rewrites (manual, and LLM-generated in the style
of C2HLSC: `ap_int`, restructured loops, streaming), synthesise, and check the **original**
contract on the RTL. Compare with C/RTL co-simulation on contract-respecting random inputs and
with the testing-based LLM-agent flows. Seed rewrite bugs (both LLM-induced and manual) and
measure detection. Full equivalence cannot do this, because rewrites legitimately change
behaviour outside the contract.

**Fit.** Mostly documentation and proofs over what exists (`methodology.md` §4–§6). T2 and T3
need new two-copy harness modes. T4 needs Vitis `INLINE off` and per-module harnesses.

**Effort.** 1–2 months for pen-and-paper proofs. A mechanised core calculus (Lean or Isabelle)
would add 2–4 months; that is optional, but FMSD/FM reviewers value it.

**Risks.** Theory without new capability reads as "formalising a tool". Mitigation: pair it with
A1 (T2 + A1 = unbounded results) and the rewrite application.

### A3 — Neuro-symbolic gap closing under one soundness discipline (RQ6)

**Idea.** The LLM is never trusted. Its output can enter the pipeline in only three ways:

* **(a) Candidate lemmas** (RTL invariants, C loop invariants). Each must be proved by k-induction
  or IC3, or by WP.
* **(b) Definitions** (a recursive or executable lifting of axiomatic or inductive logic). Each
  must come with an equivalence lemma, proved by WP or, for c2sva's bounded semantics,
  *exhaustively checked* on N cells (§3.3).
* **(c) Interface transactors** for undocumented protocols. These are validated by cover traces
  and the protocol assertions.

**Why novel (moderate).** LLM+verifier loops exist for software invariants (Lemur, ICLR 2024;
LaM4Inv, ASE 2024; AutoSpec, CAV 2024) and hardware invariants (Large Lemma Miners, arXiv 2025;
CIll, arXiv 2026). What is new here is the *cross-level* use: C artefacts as LLM context for RTL
invariants, and LLM lifting of ACSL axiomatics guarded by a polarity-aware soundness argument.

**Evaluation.** Ablation (rule-based only / + LLM), success rate per gap category, token cost,
5 runs per configuration with variance reported, and a strict "zero unsound outputs accepted"
claim backed by the guards.

**Ranking.** A1 (highest novelty, measurable gain) > A2 (needed for rigour; strong alone at
FMSD/FM) > A3 (incremental alone, valuable as RQ6). **Recommend A1 + A2 in one TCAD/FMSD
journal paper, with A3 as its RQ6 section.**

---

## 3. (B) The eight smaller problems — concrete techniques

Effort: S ≤ 1 week, M = 2–4 weeks, L = 1–3 months.

### 3.1 Scalability of the formal check

| # | Technique (reference) | How to apply in c2sva | Effort | Benefit |
|---|---|---|---|---|
| 1a | **Unbounded engines + strengthening.** k-induction (Sheeran, Singh, Stålmarck, FMCAD 2000), IC3/PDR (Bradley, VMCAI 2011), word-level IC3: Pono (Mann et al., CAV 2021), AVR (Goel & Sakallah, TACAS 2020), rIC3 (Su et al., arXiv 2025, HWMCC winner) | Add engine choices to the portfolio (`abc pdr`, `pono`, `rIC3`, smtbmc k-induction). Supply the A1 invariants plus a few cheap generic ones (FSM one-hot, idle ⇒ snapshot registers equal memory for untouched cells). | S–M | Proofs independent of depth. Removes "bounded(k)" for deep designs. |
| 1b | **Completeness threshold / transaction induction (T2).** BMC is complete beyond the recurrence diameter (Kroening & Strichman, VMCAI 2003). A-QED's single-action + consistency decomposition (DAC 2020). | Prove "every idle state is contract-equivalent to reset" once with a two-copy miter. Then *one* call from reset to depth `L+4` is a complete proof for all calls. | M | Turns most of today's PASS results into unbounded results at low cost. |
| 1c | **Datapath abstraction / uninterpreted multipliers with CEGAR.** Reveal (Andraus, Liffiton, Sakallah, LPAR 2008); Averroes (Lee & Sakallah, CAV 2014); incremental linearisation (Cimatti, Griggio, Irfan, Roveri, Sebastiani, TOCL 2018 / TACAS 2017); bit-vector under/over-approximation (Bryant et al., TACAS 2007) | For `x*y*z`: (i) **range-aware width**: interval analysis over the `requires` bounds shrinks W (today W covers worst-case nesting); (ii) **modular congruence**: when `Pre` rules out overflow, rewrite `\result == x*y*z` to a check modulo 2^w plus range facts, so both sides use 32-bit products; (iii) **multiplier cut-points**: yosys `cutpoint`/`abstract -value` on `$mul` cells, with operand/result witnesses shared with the property (Ackermann constraints); refine on spurious counterexamples using the existing C replay as oracle. | M | Fixes conformance c10 and arithmetic-heavy ACSLBench tasks. |
| 1d | **Compositional proof with callee contracts (T4)**; functional decomposition (A-QED², FMCAD 2021); Siemens microtransactions | Synthesize callees as separate modules (`INLINE off`). Check each against its own ACSL contract, then check the caller with each callee replaced by its contract (assume `Post` at the callee's `ap_done`, outputs free otherwise). | M | Linear rather than exponential growth for multi-function designs. |
| 1e | **Memory-model reduction** (see 3.2): tracked-cell memory instead of N concrete cells. | — | — | Smaller state in the solver; avoids out-of-memory failures. |

### 3.2 Bounded memory model (N = 4)

Observation first. Rewriting `T *p` to `T p[4]` changes the hardware: address width 2. The
`is_sorted` aliasing of `a[4]` onto `a[0]` is a consequence of that. Results are therefore about
a different circuit than the deployed one.

| # | Technique | Application | Effort | Benefit |
|---|---|---|---|---|
| 2a | **Size-independent RTL + abstract memory with tracked cells.** EMM (Ganai, Gupta, Ashar, CAV 2004); word-level memory abstraction (Bjesse, FMCAD 2008); counterexample-guided prophecy for arrays (Mann, Irfan, Griggio, Padon, Barrett, TACAS 2021 / LMCS 2022) | Synthesize with an interface whose RTL does not depend on n: `bram` (32-bit byte address) or `m_axi`, or the deployment depth. In the harness, replace the N-cell RAM by a memory that *tracks* only the symbolic indices of the property (c2sva's universal-assertion constants) plus a few prophecy indices; other reads return fresh values. This over-approximates, so proofs remain sound; spurious counterexamples are refined by adding tracked indices. | M–L | One proof covers **every array size** representable by the address width: a genuinely unbounded result for RQ4. Pairs with A1 for loop bounds. |
| 2b | **Data independence** (Wolper, POPL 1986; Lazić & Nowak, CONCUR 2000) | Many ABE functions (find, copy, equal, count, sorting by comparison) only copy and compare element values. Detect this syntactically on the C code (no arithmetic on elements) and justify small value domains, which shrinks element widths in the harness. | S–M | Faster BMC. A formal argument about values, though not about sizes. |
| 2c | **Small-model / cut-off arguments by induction over N.** Full-program induction (Chakraborty, Gupta, Unadkat, TACAS 2020; Vajra); cell morphing (Monniaux & Gonnord, SAS 2016); CMP method (Chou, Mannava, Park, FMCAD 2004: k concrete agents + abstract "other") | Use as the *theoretical* justification in the paper: properties with one universally quantified index (most ABE postconditions) are cell-local, so tracking 1–2 cells suffices (cell morphing). Use CMP-style "other cells" abstraction when 2a needs a symmetric environment. | S (argument) | Lets the paper state *when* N = 4 generalises, instead of only showing an N-sensitivity table. |

### 3.3 Axiomatic and inductive ACSL logic

The benchmarks contain `is_gcd`, `isfact`, `same_elements` (swap/trans permutation), `reachable`
and others (12 files with `inductive`, 32 with `axiomatic`).

| # | Technique | Application | Effort | Benefit |
|---|---|---|---|---|
| 3a | **Polarity-aware bounded unrolling of inductive predicates** (least-fixpoint semantics) | A derivation of depth ≤ k gives `P_k ⇒ P`, an *under-approximation*. Sound in positive-assertion and negative-assumption positions; otherwise use a one-step inversion over-approximation (recursive premises replaced by `true`). This extends c2sva's polarity rule exactly. Depth bounds are often exact: Euclid on 32-bit inputs needs ≤ 47 steps; `isfact` without overflow needs n ≤ 12. Reuse **E-ACSL's extraction of Horn-clause inductives into direct recursive definitions** (Frama-C 33 `e-acsl/src/analyses/inductive.ml`, refman "Inductive predicates", which documents its restrictions), then feed the result to c2sva's existing unroller. | M | Covers gcd/fact/reachable-style definitions soundly, without an LLM. |
| 3b | **Axioms as instantiated constraints on uninterpreted symbols** (Ackermann reduction; positive equality: Bryant, German, Velev, CAV 1999 / TOCL 2001) | Treat `logic integer f(...)` defined by axioms as a solver-chosen value per distinct argument term, with functional consistency constraints. Add axiom instances over the terms in the property, up to depth d, as assumptions. Proving `∀f. Ax_inst(f) ⇒ φ(f)` implies `φ` for the intended `f`, so it is **sound in every polarity**. Spurious counterexamples are filtered by the existing C replay (CEGAR: add instances). Check axiom consistency once with WP smoke tests (Blanchard, Correnson, Djoudi, Kosmatov, TAP 2024), otherwise the property may pass vacuously. | M | Translates most `axiomatic` blocks with a clear soundness argument. |
| 3c | **Lifting to executable definitions with a checked equivalence lemma** (LLM-assisted, A3) | For definitions outside 3a/3b (permutation via `trans` needs an existential intermediate memory state), the LLM proposes an executable equivalent, e.g. `∀v. count(pre,v) == count(post,v)` for `same_elements`. A Frama-C lemma states the equivalence. Accept it if WP proves it (or with a Coq/Rocq script), **or**, for c2sva's bounded semantics, if an exhaustive SMT/BMC check over ≤ N cells and data-independent small values confirms it. That suffices because PASS claims are already relative to N. | M | Unlocks sorting and permutation contracts (ABE `*_sort`, ACSLBench permutation tasks). |

### 3.4 Global state, AXI interfaces, dataflow

| # | Technique | Application | Effort | Benefit |
|---|---|---|---|---|
| 4a | **Transactors** (Bombieri, Fummi, Pravadelli, TLM→RTL assertion reuse, DATE 2006/07) + **AXI formal VIP** (YosysHQ SVA AXI VIP, open source; ARM AXI4 protocol assertions as packaged in AMD PG101) | `s_axilite`: the call model becomes "write 1 to bit 0 at 0x00, poll ap_done at bit 1"; scalar arguments sit at offsets from 0x10 (UG1399 register map; read offsets from the generated `x<top>_hw.h` / report). `m_axi`: harness memory slave behind the VIP's assumptions (bursts, outstanding transactions), `locate(i)` = base offset + i·size. Only the event detector and the memory model change; the contract rules stay the same. | M | Covers the default Vitis kernel flow; needed for credibility in RQ5. |
| 4b | **Streams as ghost sequences** (ACSL `\list`, supported by WP's `Vlist` theory) + **Frama-C Aoraï** for call-sequence automata | Model `hls::stream` arguments as logic lists: `ensures out == \concat(\old(out), …)`. At RTL, an AXI-stream FIFO observer records the pushed or popped sequence per call. Aoraï can express multi-call protocols (e.g. init-then-process) and is the natural specification route for global state. | M | Opens dataflow/streaming designs (CHStone, MachSuite-style). |
| 4c | **Dataflow by Kahn determinism + compositional contracts** (Kahn, IFIP 1974; T4) | Vitis dataflow processes are sub-functions. Check each against its own contract (3.1 1d), then compose. Kahn determinism equates the dataflow execution with the sequential C. Check deadlock-freedom separately (a bounded `ap_done` liveness cover, or Vitis cosim deadlock detection, UG1399). | M–L | Dataflow regions become checkable without monolithic BMC. |
| 4d | **Globals as white-box state** | Vitis keeps globals as internal registers or BRAMs that persist across calls. Map them with the same schedule/bind correspondence as A1. Initial values come from C initialisers. Pre-state = state at acceptance via hierarchical references. Requires T2-style reasoning, because state *does* persist across calls. (Older Vivado HLS had `-expose_global`; its availability in Vitis 2026.1 is **[unverified]**.) | M | Removes the "global state" exclusion (a significant share of the ACSLBench clauses not translated today). |

### 3.5 Overlapping calls (pipelined top functions)

| # | Technique | Application | Effort | Benefit |
|---|---|---|---|---|
| 5a | **Sequential-vs-overlapped self-composition (T3)**: Burch & Dill flushing (CAV 1994); WEB refinement (Manolios, FMCAD 2000); G-QED for interfering state (DAC 2023) | Two harness copies receive the same call sequence; copy A is forced sequential, copy B overlaps. Assert equal outputs and equal memory after each call completes in both. Together with the sequential contract check this implies the contract under overlap, *including written memory*: the C semantics of a call sequence is sequential composition, and B must refine it. | M | Lifts the read-only restriction; needs no per-call memory tagging. |
| 5b | **Completion functions / refinement maps** (Hosabettu, Srivas, Gopalakrishnan, CAV 1998; ILA refinement maps) | If 5a is too costly: per in-flight call, define its abstract effect as "its writes if it were flushed". The harness keeps a FIFO of pending write sets (address and data captured at the write port, attributed in order) and checks each call's contract on the reconstructed sequential pre-state. | L | Direct contract check without a second copy. More engineering. |
| 5c | **Scoreboard semantics** (in-order FIFO, already present) | Keep it for read-only memories, and state the theorem (T3) that justifies it. | S | Rigour. |

### 3.6 Speculative versus architectural out-of-range accesses

| # | Technique | Application | Effort | Benefit |
|---|---|---|---|---|
| 6a | **Differential access trace against C** (exact for a given witness) | Replay already runs the C code. Record the C access set (ASan or E-ACSL instrumentation) and compare it with the RTL access set from the trace. An RTL access absent from the C trace is *HLS-introduced* (speculative). If WP with `-wp-rte` proves the C memory-safe, **every** RTL out-of-range access is HLS-introduced (a universal statement). | S | Cleanly separates mutants' real C-level out-of-bounds accesses from Vitis speculation. |
| 6b | **Noninterference by self-composition** (Barthe, D'Argenio, Rezk, CSFW 2004; Terauchi & Aiken, SAS 2005; UPEC, Fadiheh et al., DATE 2019) | Two harness copies that differ *only* in cells outside the `\valid` range. Assert equal outputs and equal in-range post-states. Pass means out-of-range reads are unobservable (benign); a counterexample shows an architectural use of the out-of-range value. | M | A formal benign/harmful verdict rather than a heuristic one. |
| 6c | **Formal information-flow tracking** (GLIFT: Tiwari et al., ASPLOS 2009; RTLIFT: Ardeshiricham, Hu, Marxen, Kastner, DATE 2017; CellIFT: Solt, Gras, Razavi, USENIX Security 2022, a Yosys pass) | Taint the read data whenever the address is out of range; assert that no taint reaches `ap_return`, write data or control. Cheaper than 6b because it uses one copy (over-approximate taint). | M | Scales to large designs. A security angle (cf. "hardware–software contracts for secure speculation", Guarnieri et al., S&P 2021). |

The `bram` finding (speculative read of `a[n]` becomes a real bus read out of bounds) is
publishable as a security result if shown to leak an adjacent buffer (6b counterexample + board
or co-simulation demonstration).

### 3.7 Property strength and coverage beyond mutation score

| # | Technique | Application | Effort | Benefit |
|---|---|---|---|---|
| 7a | **RTL mutation** with MCY (YosysHQ "Mutation Cover with Yosys", installed) | Mutate the Vitis netlist. MCY filters equivalent mutants by formal equivalence with SBY. Report contract kill rate on RTL mutants next to C mutants. RTL faults approximate *HLS tool bugs* (Vitis and other HLS tools were shown buggy by Herklotz et al., FCCM 2021). | S–M | Answers "mutating C is not mutating the hardware". |
| 7b | **Output coverage / determinacy** (Große, Kühne, Drechsler, TCAD 2008; completeness checking, Bormann; Urdahl et al.) | For each output (return, each written cell), replace it with a free value (yosys `abstract -value`). If all postconditions still pass, that output is *not covered* by the contract. This yields a per-design "contract coverage" and separates weak specifications from weak translation. | S–M | Strong new metric that reviewers recognise. |
| 7c | **Antecedent vacuity** (Beer, Ben-David, Eisner, Rodeh, CAV 1997; Kupferman & Vardi) | Today's cover only checks that *a* call completes. Add one cover per behavior `assumes`, per implication antecedent in `ensures`, and per quantifier guard. | S | Catches "passes vacuously for this behavior". |
| 7d | **Coverage metrics for model checking** (Chockler, Kupferman, Vardi, CHARME 2003 / STTT 2006) | Mutation-based state/logic coverage on RTL registers in the cone of influence of the assertions. | M | Optional depth. |

### 3.8 Where an LLM adds value with formal guardrails

| # | Task | Guard | Prior work |
|---|---|---|---|
| 8a | Strengthen RTL invariants from counterexamples to induction (CTIs), with the C invariants and the correspondence map as context | Houdini + k-induction / IC3 | CIll (Su et al., arXiv 2026); Large Lemma Miners (Peled, Kroening, Tautschnig, Vizel, arXiv 2025); AutoINV (helpers) |
| 8b | Write missing C loop invariants and contracts so new benchmarks enter the evaluation (CHStone, MachSuite, Rosetta) | WP proof required; human review of `requires` only | AutoSpec (Wen et al., CAV 2024); LaM4Inv (ASE 2024); Lemur (ICLR 2024) |
| 8c | Lift axiomatic or inductive definitions (3c) | WP lemma or bounded exhaustive equivalence | — (new) |
| 8d | Interface and transactor synthesis for unseen protocols or report formats (4a) | Protocol assertions + cover + replay | — |
| 8e | Counterexample explanation for users (C-level narrative from the witness) | No soundness impact | — |

Avoid making the LLM translate contracts to SVA directly. The rule-based translator is the
system's strength. Instead, use LLM-based NL→SVA tools (AssertLLM, nl2sva) as a *baseline* in
RQ6 to show why a formal source specification beats natural language.

---

## 4. (C) Venues and what reviewers will demand

| Venue | Fit | Notes |
|---|---|---|
| **IEEE TCAD** (journal) | Best overall fit: HLS + formal + EDA tooling. Ray/Hao/Xie and Urdahl et al. published nearby. | Expects scalability on realistic designs and a comparison with an industrial-style flow. |
| **Formal Methods in System Design (FMSD)** | Best fit if A2 theory with proofs is central. | Expects precise semantics and soundness proofs; mechanisation is a plus. |
| **ACM TODAES** | ILA and C2HLSC appeared there; HLS-centric. | Practical emphasis. |
| **FMCAD** (conference) | A-QED² and Bjesse appeared there. Ideal for A1 (unbounded RTL proofs from C artefacts). | 10-page limit; tight evaluation. |
| **DAC / ICCAD / DATE** | Shorter tool or finding papers (speculative out-of-bounds; A1 results). | Industrial relevance, runtime tables. |
| **iFM / FM** | iFM's theme (combining deductive verification and model checking) matches A1 + A2 exactly. | Theory plus a case study. |
| **CAV / TACAS** (tool track) | c2sva as a tool with artifact evaluation. | Artifact badge, reproducibility. |
| **MEMOCODE, FPGA/FCCM** | HLS community; FCCM published the HLS reliability study. | — |

**What reviewers will ask for:**

1. A formal statement of what PASS means, with a soundness proof (A2-T1), and explicit
   assumptions: environment, N, interface model trusted, Vitis report trusted.
2. **Unbounded results** or a convincing generalisation argument (A1, 1b, 2a, 2c).
3. **Baselines:**
   * Vitis C/RTL co-simulation with contract-guided random inputs (same time budget);
   * A-QED-style consistency checking (it finds bugs without specifications: why are contracts
     better? Show bugs that are consistent but wrong);
   * AutoINV-style invariants for the proof stage;
   * LLM NL→SVA for RQ6.
4. **Realistic designs beyond ACSL by Example:** CHStone (Hara et al., ISCAS 2008), MachSuite
   (Reagen et al., IISWC 2014), Rosetta (Zhou et al., FPGA 2018), with contracts written or
   LLM-generated and WP-checked (8b), plus default Vitis kernel interfaces (`m_axi`/`s_axilite`,
   4a).
5. **Real bugs.** The speculative `bram` out-of-bounds access, any Vitis bug found (check older
   Vitis versions with known bugs), and rewrite bugs (A2 application). Mutants alone will be
   called synthetic. Add RTL mutants (7a).
6. **Independence of the oracle.** The concrete evaluator shares the parser with the translator
   (already listed as a threat). Mitigate with E-ACSL agreement statistics and a differential
   test of the evaluator against E-ACSL on random inputs.
7. **Second HLS tool** (the Bambu back end exists) for the "tool-agnostic" claim; at least a
   subset.
8. Artifact with pinned toolchain; report solver versions, timeouts and variance.

---

## 5. Suggested roadmap (≈ 6 months to journal submission)

1. **(S, 2 wk)** Quick wins: 1b transaction induction, 6a differential access trace, 7b output
   coverage, 7c antecedent covers, 1a engine portfolio.
2. **(L, 8–10 wk)** A1: loop-annotation front end → correspondence map → Houdini → unbounded
   proofs; progress invariants for latency.
3. **(M–L, 6 wk)** 2a size-independent interface + tracked-cell memory; 4a `m_axi`/`s_axilite`.
4. **(M, 4 wk)** 3a/3b axiomatic/inductive; 5a overlap with written memory.
5. **(M, 4 wk)** A2 write-up with proofs T1–T4; rewrite case study; A3 ablation.

---

## 6. References (verified by search on 2026-11-20 unless marked)

**HLS verification and closest work**
- E. Singh, F. Lonsing, S. Chattopadhyay, M. Strange, P. Wei, X. Zhang, Y. Zhao, J. Cong, D. Chen, Z. Zhang, P. Raina, C. Barrett, S. Mitra. *A-QED Verification of Hardware Accelerators.* DAC 2020. https://theory.stanford.edu/~barrett/pubs/SLC+20-abstract.html
- S. Chattopadhyay et al. *Scaling Up Hardware Accelerator Verification using A-QED with Functional Decomposition.* FMCAD 2021. https://arxiv.org/abs/2108.06081
- S. Chattopadhyay, K. Devarajegowda, B. Zhao, F. Lonsing, et al., C. Trippel, C. Barrett, S. Mitra. *G-QED: Generalized QED Pre-silicon Verification beyond Non-Interfering Hardware Accelerators.* DAC 2023. https://cs.stanford.edu/~trippel/pubs/chattopadhyay_DAC23.pdf
- X. Zhou, L. Du, G. Hu, S. Sinha, H. Zhang, W. Zhang. *AutoINV: Automated Invariant Generation Framework for Formal Verification on High-Level Synthesis Designs.* arXiv 2604.22285, 2026. https://arxiv.org/abs/2604.22285 (details read from the arXiv HTML: Vitis 2020.2, IC3Ref clause side-loading)
- J. Xu, L. Josipović. *Automatic Inductive Invariant Generation for Scalable Dataflow Circuit Verification.* ICCAD 2023. DOI 10.1109/ICCAD57390.2023.10323796
- J. Xu, J. Cortadella, L. Josipović. *PROMISE: Property Mining for Sequential Synthesis.* ICCAD 2025. https://www.cs.upc.edu/~jordicf/Research/gavina/BIB/files/iccad2025_promise.pdf
- S. Kundu, S. Lerner, R. Gupta. *Validating High-Level Synthesis.* CAV 2008. https://www.cs.cornell.edu/~lerner/papers/hls.html
- S. Ray, K. Hao, Y. Chen, F. Xie, J. Yang. *Formal Verification for High-Assurance Behavioral Synthesis.* ATVA 2009. https://home.ttic.edu/~chenyan/paper/atva09.pdf
- K. Hao, S. Ray, F. Xie. *Equivalence Checking for Function Pipelining in Behavioral Synthesis.* DATE 2014.
- P. Chauhan et al. (Siemens). *Formal verification using microtransactions.* US Patent 10515168 B1 (priority 2014). https://patents.google.com/patent/US10515168
- B.-Y. Huang, H. Zhang, P. Subramanyan, Y. Vizel, A. Gupta, S. Malik. *Instruction-Level Abstraction (ILA): A Uniform Specification for SoC Verification.* ACM TODAES 2018/2019. https://arxiv.org/abs/1801.01114
- L.-N. Pouchet, E. Tucker, N. Zhang, H. Chen, D. Pal, G. Rodríguez, Z. Zhang. *Formal Verification of Source-to-Source Transformations for HLS.* FPGA 2024. https://www.csl.cornell.edu/~zhiruz/pdfs/hls-verify-fpga2024.pdf
- Y. Herklotz, J. D. Pollard, N. Ramanathan, J. Wickerson. *Formal Verification of High-Level Synthesis (Vericert).* OOPSLA 2021. https://www.cl.cam.ac.uk/~jpw48/papers/vericert_oopsla21.pdf
- Y. Herklotz, Z. Du, N. Ramanathan, J. Wickerson. *An Empirical Study of the Reliability of High-Level Synthesis Tools.* FCCM 2021. https://www.cl.cam.ac.uk/~jpw48/papers/fuzzingHLS.pdf
- A. Tiemeyer, T. Melham, D. Kroening, J. O'Leary. *CREST: Hardware Formal Verification with ANSI-C Reference Specifications.* arXiv 1908.01324, 2019.
- L. Collini, S. Garg, R. Karri. *C2HLSC: Leveraging LLMs to Bridge the Software-to-Hardware Design Gap.* ACM TODAES 2025. DOI 10.1145/3734524
- Z. Xiao et al. *Shift-Left High-Level Synthesis Verification via Knowledge-Augmented LLM Agent.* arXiv 2606.17128, 2026. Z. Zhao et al. *Evidence-Driven LLM Agent for C-to-Synthesizable-C Conversion and Verification.* arXiv 2606.28409, 2026.
- N. Calagar, S. D. Brown, J. H. Anderson. *Source-Level Debugging for FPGA High-Level Synthesis.* FPL 2014 (C-variable ↔ RTL-signal mapping precedent for A1).

**Proof and invariant transfer, translation validation, refinement**
- G. Barthe, B. Grégoire, C. Kunz, T. Rezk. *Certificate Translation for Optimizing Compilers.* SAS 2006; ACM TOPLAS 2009. https://software.imdea.org/~ckunz/publications/certtrans-SAS06.pdf
- L. Zuck, A. Pnueli, B. Goldberg, et al. *VOC: A Methodology for the Translation Validation of Optimizing Compilers.* J. UCS 9(3), 2003. B. Goldberg, *Translation Validation of Loop Optimizations and Software Pipelining in the TVOC Framework* (2010). https://cs.nyu.edu/~goldberg/pubs/bg_pipelining10.pdf
- J.-B. Tristan, X. Leroy. *A Simple, Verified Validator for Software Pipelining.* POPL 2010.
- C. Flanagan, K. R. M. Leino. *Houdini, an Annotation Assistant for ESC/Java.* FME 2001.
- D. Beyer, M. Dangl, D. Dietsch, M. Heizmann. *Correctness Witnesses: Exchanging Verification Results between Verifiers.* FSE 2016.
- J. R. Burch, D. L. Dill. *Automatic Verification of Pipelined Microprocessor Control.* CAV 1994.
- P. Manolios. *Correctness of Pipelined Machines.* FMCAD 2000.
- R. Hosabettu, M. K. Srivas, G. Gopalakrishnan. *Decomposing the Proof of Correctness of Pipelined Microprocessors.* CAV 1998.
- C. Abate, R. Blanco, D. Garg, C. Hritcu, M. Patrignani, J. Thibault. *Journey Beyond Full Abstraction: Exploring Robust Property Preservation for Secure Compilation.* CSF 2019. https://arxiv.org/abs/1807.04603
- N. Bombieri, F. Fummi, G. Pravadelli. TLM→RTL assertion reuse via transactors, DATE 2006/2007 (already cited in related_work.md).
- J. Urdahl, D. Stoffel, W. Kunz. *Path Predicate Abstraction for Sound System-Level Models of RT-Level Circuit Designs.* IEEE TCAD 33(2), 2014.
- A. Benveniste, B. Caillaud, D. Nickovic, R. Passerone, J.-B. Raclet, et al. *Contracts for System Design.* Foundations and Trends in EDA 12(2–3), 2018.

**Model checking, abstraction, arithmetic**
- M. Sheeran, S. Singh, G. Stålmarck. *Checking Safety Properties Using Induction and a SAT-Solver.* FMCAD 2000.
- A. R. Bradley. *SAT-Based Model Checking without Unrolling.* VMCAI 2011.
- M. Mann, A. Irfan, F. Lonsing, Y. Yang, H. Zhang, K. Brown, A. Gupta, C. Barrett. *Pono: A Flexible and Extensible SMT-Based Model Checker.* CAV 2021.
- A. Goel, K. Sakallah. *AVR: Abstractly Verifying Reachability.* TACAS 2020.
- Y. Su, Q. Yang, Y. Ci, T. Bu, Z. Huang. *The rIC3 Hardware Model Checker.* arXiv 2502.13605, 2025.
- A. Cimatti, A. Griggio, S. Mover, S. Tonetta. *IC3 Modulo Theories via Implicit Predicate Abstraction.* TACAS 2014.
- A. Cimatti, A. Griggio, A. Irfan, M. Roveri, R. Sebastiani. *Incremental Linearization for Satisfiability and Verification Modulo Nonlinear Arithmetic and Transcendental Functions.* ACM TOCL 2018 (and TACAS 2017 / arXiv 1801.08718).
- Z. Andraus, M. Liffiton, K. Sakallah. *Reveal: A Formal Verification Tool for Verilog Designs.* LPAR 2008.
- S. Lee, K. Sakallah. *Unbounded Scalable Verification Based on Approximate Property-Directed Reachability and Datapath Abstraction.* CAV 2014.
- R. E. Bryant, D. Kroening, J. Ouaknine, S. A. Seshia, O. Strichman, B. Brady. *Deciding Bit-Vector Arithmetic with Abstraction.* TACAS 2007.
- R. E. Bryant, S. German, M. N. Velev. *Exploiting Positive Equality in a Logic of Equality with Uninterpreted Functions.* CAV 1999 (from memory; the search did not specifically confirm it).
- D. Kroening, O. Strichman. *Efficient Computation of Recurrence Diameters.* VMCAI 2003.
- M. K. Ganai, A. Gupta, P. Ashar. *Efficient Modeling of Embedded Memories in Bounded Model Checking.* CAV 2004.
- P. Bjesse. *Word-Level Sequential Memory Abstraction for Model Checking.* FMCAD 2008.
- M. Mann, A. Irfan, A. Griggio, O. Padon, C. Barrett. *Counterexample-Guided Prophecy for Model Checking Modulo the Theory of Arrays.* TACAS 2021; LMCS 18(3), 2022.
- D. Monniaux, L. Gonnord. *Cell Morphing: From Array Programs to Array-Free Horn Clauses.* SAS 2016.
- S. Chakraborty, A. Gupta, D. Unadkat. *Verifying Array Manipulating Programs with Full-Program Induction.* TACAS 2020.
- C.-T. Chou, P. K. Mannava, S. Park. *A Simple Method for Parameterized Verification of Cache Coherence Protocols.* FMCAD 2004.
- R. Lazić, D. Nowak. *A Unifying Approach to Data-Independence.* CONCUR 2000. P. Wolper, POPL 1986 (data independence; from memory).

**Security, information flow, speculation**
- G. Barthe, P. D'Argenio, T. Rezk. *Secure Information Flow by Self-Composition.* CSFW 2004. T. Terauchi, A. Aiken. *Secure Information Flow as a Safety Problem.* SAS 2005.
- M. R. Fadiheh, D. Stoffel, C. Barrett, S. Mitra, W. Kunz. *Processor Hardware Security Vulnerabilities and their Detection by Unique Program Execution Checking.* DATE 2019.
- A. Ardeshiricham, W. Hu, J. Marxen, R. Kastner. *Register Transfer Level Information Flow Tracking for Provably Secure Hardware Design.* DATE 2017.
- F. Solt, B. Gras, K. Razavi. *CellIFT: Leveraging Cells for Scalable and Precise Dynamic Information Flow Tracking in RTL.* USENIX Security 2022.
- M. Tiwari et al. *Complete Information Flow Tracking from the Gates Up (GLIFT).* ASPLOS 2009 (from memory).
- M. Guarnieri, B. Köpf, J. Reineke, P. Vila. *Hardware-Software Contracts for Secure Speculation.* IEEE S&P 2021.

**Coverage and vacuity**
- H. Chockler, O. Kupferman, M. Y. Vardi. *Coverage Metrics for Formal Verification.* CHARME 2003; STTT 2006.
- I. Beer, S. Ben-David, C. Eisner, Y. Rodeh. *Efficient Detection of Vacuity in ACTL Formulas.* CAV 1997.
- D. Große, U. Kühne, R. Drechsler. *Analyzing Functional Coverage in Bounded Model Checking.* IEEE TCAD 27(7), 2008.
- J. Bormann. *Complete Functional Verification* (PhD thesis, TU Kaiserslautern); OneSpin "gap-free verification".
- YosysHQ MCY (Mutation Cover with Yosys), https://mcy.readthedocs.io/; YosysHQ SVA AXI VIP, https://yosyshq.readthedocs.io/projects/ap320/

**Frama-C ecosystem**
- E-ACSL reference manual, Frama-C 33: "Inductive predicates" (experimental; generalised Horn-clause subset) and "Axiomatic definitions" (experimental). Source: `src/plugins/e-acsl/src/analyses/inductive.ml`, `doc/refman/speclang_modern.tex`.
- A. Blanchard, L. Correnson, A. Djoudi, N. Kosmatov. *No Smoke without Fire: Detecting Specification Inconsistencies with Frama-C/WP.* TAP 2024.
- Frama-C Aoraï plug-in manual (33.0), https://frama-c.com/fc-plugins/aorai.html. WP `\list` support: `src/plugins/wp/Vlist.ml`.
- AMD UG1399 (Vitis HLS user guide): S_AXILITE control register map; verification of DATAFLOW.

**LLM + verification**
- H. Wu, C. Barrett, N. Narodytska. *Lemur: Integrating LLMs in Automated Program Verification.* ICLR 2024.
- C. Wen et al. *Enchanting Program Specification Synthesis by LLMs using Static Analysis and Program Verification (AutoSpec).* CAV 2024.
- G. Wu et al. *LLM Meets Bounded Model Checking: Neuro-symbolic Loop Invariant Inference (LaM4Inv).* ASE 2024 (author list not checked).
- R. Peled, D. Kroening, M. Tautschnig, Y. Vizel. *Large Lemma Miners: Can LLMs do Induction Proofs for Hardware?* arXiv 2511.02521 (venue not confirmed).
- Y. Su, T. Bu, Q. Yang, Y. Ci, E. Tian. *CIll: CTI-Guided Invariant Generation via LLMs for Model Checking.* arXiv 2602.23389, 2026.

**Benchmarks suggested** (from memory, standard references): CHStone (Hara et al., ISCAS 2008), MachSuite (Reagen et al., IISWC 2014), Rosetta (Zhou et al., FPGA 2018).

### Items to double-check before citing
Vitis 2026.1 behaviour for globals (`expose_global` equivalent); whether SBY exposes rIC3 as an
engine directly (the rIC3 paper says it can serve as a SymbiYosys backend); the Frama-C/WP
interactive `Induction` tactic for proving lifting lemmas; final venues of the arXiv-only works
(AutoINV, CIll, Large Lemma Miners).
