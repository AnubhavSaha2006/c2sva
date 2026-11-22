# c2sva — semantics and soundness (A2)

This note states what c2sva's results mean and why they are sound. It follows the code
(`translate.py`, `harness.py`, `formal.py`, `prove.py`); section numbers in brackets refer to
`methodology.md`.

## 1. Objects

**C side.** A function `f` with ACSL contract `𝒞 = (Pre, Post, Asg)` (behaviors are desugared:
`assumes A; requires R; ensures Q; assigns L` becomes `A ⇒ R`, `\old(A) ⇒ Q`, and a frame per
behavior under `\old(A)`). Pointer parameters given as ranges are arrays of `N` cells (the
HLS input is rewritten to `T p[N]`, [3]). A *C state* `σ` maps formals to values and array
cells to values. The *bounded C semantics* `⟦f⟧_N ⊆ Σ × Σ` relates a pre-state to the post-state
of a terminating execution. The contract is *valid for N* if

    ∀ (σ, σ') ∈ ⟦f⟧_N .  Pre(σ) ⇒ Post(σ, σ') ∧ Frame_Asg(σ, σ')                    (C)

where `Frame_Asg` says every cell outside the `assigns` locations (evaluated in `σ`) is unchanged.
Frama-C/WP establishes (C) for unbounded arrays; the RTL check needs it only for `N` cells.

**RTL side.** Vitis produces a synchronous circuit `M` with ports `ap_start, ap_done, ap_ready, …`,
scalar ports and memory interfaces. The *interface map* `ι` (from the csynth report and the
layout function `locate`, [4]) sends every C formal to a port and every array cell to a
(bank, word, slot) of a memory interface.

**Harness.** `H = M ∥ Env ∥ Mem ∥ Obs` is a finite transition system:

* `Env` (assumptions): reset in the first cycle only; `ap_start` held until `ap_ready`; scalar
  inputs stable while a call is in flight; sequential mode: no new `ap_start` before `ap_done`;
  `ap_continue` high. DUT registers that the RTL neither resets nor initialises start at 0, as
  on an FPGA (configuration INIT values) and in Vitis co-simulation; `--power-on arbitrary` lets
  them start anywhere (ASIC-style, stricter). With arbitrary values, a Vitis design (ABE
  `shuffle`, through its unreset divider pipeline) shows RTL/C differences that co-simulation and
  the zero-initialised model do not. These are the obligations Vitis' block-level protocol places on a caller
  (UG1399, `ap_ctrl_hs`/`ap_ctrl_chain`). In addition (part of T2 below), memory is quiescent
  during a call (no other agent writes it) and RAM reads have the fixed latency of the
  `ap_memory`/`bram` protocol.
* `Mem`: one register per C cell, read/written through the protocol of the interface
  (`ap_memory`: registered read, word write; `bram`: byte enables; cells/`ap_vld`/`ap_ovld`).
  Initial contents are unconstrained (`anyinit`).
* `Obs`: call tracking (`h_start_evt`, `h_busy`, `h_done_d`), snapshots `o_a_i` at acceptance,
  captured inputs `x_cap` and result `h_res_cap`.

**Transactions.** In a run `ρ = s_0 s_1 …` of `H`, call `k` is accepted at the `k`-th cycle with
`h_start_evt` and completes at the `k`-th `ap_done` (sequential mode; in overlap mode the `k`-th
`ap_done` is paired with the `k`-th acceptance, which Vitis guarantees for in-order pipelines and
the harness asserts as `PROTO`). Via `ι`, the state at acceptance gives a C pre-state `σ_k` and
the state one cycle after `ap_done` gives a post-state `σ'_k` (memory writes issued in the
`ap_done` cycle have landed).

**Contract preservation.** `M` *preserves* `𝒞` (for `N`) if for every run of `H` and every
completed transaction `k`: `Pre(σ_k) ⇒ Post(σ_k, σ'_k) ∧ Frame_Asg(σ_k, σ'_k)`.

This is weaker than equivalence with `f`: the RTL may compute any result the contract allows.
That is the point. The C code may be rewritten for HLS (by hand or by an LLM) and still be checked
against the original verified contract.

## 2. Translation and the polarity lemma

Each clause `φ` becomes a Verilog condition `t(φ)` over harness signals (pre view = ports and
memory at acceptance, post view = memory one cycle after `ap_done`, `\old` = snapshots). Free
symbols of `t(φ)` are *solver-chosen constants* (`anyconst`): their value is fixed for a run and
chosen by the solver, so an assertion must hold for **every** choice, and an assumption
constrains **some** choice.

**Lemma 1 (polarity).** For every translated clause, over every harness state that encodes
`(σ, σ')` through `ι`:

* assertion position: `(∀ symbols. t(φ)) ⇒ φ(σ, σ')`, i.e. the assertion is exact or stronger;
* assumption position: `Pre(σ) ⇒ ∃ symbols. t(φ)`, i.e. the assumption is exact or weaker.

*Proof sketch* (structural induction, tracking polarity, which flips under `!` and on the left
of `⇒`):

* Connectives, arithmetic in `W` bits ([5.6]: `W` is large enough that no `integer` term
  overflows), casts (exact C conversions): exact.
* A quantifier that is universal in effect in an assertion becomes a fresh symbol. Since the
  assertion must hold for every value of the symbol, this is exact.
* A quantifier that is existential in effect in an assertion is enumerated over `0..N`: the
  disjunction implies the existential, so the assertion is stronger.
* A quantifier that is universal in effect in an assumption is enumerated: the conjunction is
  implied by the universal, so the assumption is weaker. An existential in effect in an
  assumption is not translated: the clause is dropped, which is weaker.
* `\sum`/`\numof` are enumerated with the side condition "range within `0..N-1`": conjoined in
  assertions (stronger), a guard in assumptions (weaker).
* A cell outside `0..N-1` is a solver-chosen symbol, shared between pre- and post-state. In an
  assertion it must hold for every value (stronger than any particular C value). In an
  assumption it is satisfiable by the C value (weaker). Sharing is faithful: the RTL has no
  storage for such a cell and cannot change it, and an out-of-range write fails the access check.
* `\valid(a + (lo..hi))` nested in an expression is the exact `N`-cell meaning
  `hi < lo ∨ (0 ≤ lo ∧ hi < N)`.
* Recursive logic is unrolled to depth `N+2`. A deeper call is a fresh symbol: stronger in
  assertions, weaker in assumptions. The libc `strlen` cut-off value is exact for the `N`-cell
  model ([5.5]).
* Behaviors are desugared as in §1, with the `assumes` negated inside an assertion, so it has
  negative polarity.

Clauses that are not translated are reported. A dropped precondition makes a FAIL inconclusive
(`fail-inconclusive`), and a dropped postcondition makes a PASS partial.

## 3. Soundness of the bounded check

**Assumptions (trusted base).** (T1) The interface map `ι` and the memory models agree with
the RTL interfaces. These come from Vitis' report and the documented protocols; a mismatch is
caught in practice by the cover task and by C replay. (T2) `Env` admits every behaviour of a
caller that respects the block protocol. (T3) Yosys, SymbiYosys and the SMT solvers are correct.

**Theorem 1 (bounded).** If BMC of `H` passes to depth `D`, then for every run of length at most
`D` and every transaction completed in it, contract preservation holds for the translated
clauses (for `N` cells).

*Proof.* Let `k` complete within the bound and let `Pre(σ_k)` hold. By Lemma 1 (assumption side),
the harness assumptions admit this run, for some choice of the symbols. The BMC verdict then
gives the assertions at the cycle after `ap_done` for every choice of the symbols. By Lemma 1
(assertion side), `Post` and the frame hold for `(σ_k, σ'_k)`. ∎

*Coverage.* The harness admits arbitrarily many sequential calls, with arbitrary inputs and
whatever state earlier calls left in the circuit. A bounded PASS covers every call sequence that
fits in D cycles. With D = L + 4 that is about one call from reset plus the state it leaves; a
`bounded(k)` result covers only k cycles. Unbounded call sequences are covered by Theorem 2, in
sequential mode.
The `cover` task shows that at least one call completes under the assumptions (non-vacuity).

## 4. Unbounded proofs (A1)

**Theorem 2.** Let `I` be a set of state predicates of `H` such that SymbiYosys' k-induction proves
`I` (base case to depth `k` from the initial states, step case of length `k`). Let the contract
assertions `P` be k-inductive relative to `I` (proved with `I` assumed). Then `P` holds in every
reachable state of `H`, so contract preservation holds for every transaction of every run, with
no bound on run length, number of calls or latency (for `N` cells).

*Proof.* k-induction is sound for finite transition systems, so `I` holds in all reachable
states. Assuming `I` therefore removes only unreachable states, and the k-induction proof of `P`
over that restricted system carries over to the reachable states. Theorem 1's argument, now
for all depths, gives contract preservation. ∎

**Corollary (candidate generation is outside the trusted base).** Soundness does not depend on how
the candidates are produced: loop invariants mapped by register name, HLS-structural guesses,
callee-loop guesses, or an LLM. A wrong candidate is dropped by Houdini, and only the final
k-induction verdict counts. Houdini returns the largest subset of the candidates that is
k-inductive (Flanagan–Leino: dropping candidates violated at the last step of a counterexample
to induction never removes a member of any inductive subset). It only affects completeness.

**Theorem 3 (latency, RQ3).** If `h_busy ⇒ h_cyc ≤ B` is proved as in Theorem 2, then every accepted call
completes within `B` cycles in every run (under `Env`, with `ap_continue` held high). Together
with Theorem 2 this is total correctness of the hardware transaction for `N`-cell arrays.

**Proposition 3' (lifted definitions).** Let `f` be axiomatic and `g(x) = DOM(x) ? BODY_g(x) : ⊥`
the lifted definition (`⊥` a fresh solver-chosen value), with the unfolding lemma
`∀x. DOM(x) ⇒ f(x) = BODY_g[g := f](x)` proved from the axioms. Then for every unrolling depth `d` and
every argument, some choice of the solver-chosen values makes the depth-`d` unrolling of `g`
equal `f`. *Proof:* by induction on `d`. At a cut-off or outside `DOM`, choose `⊥ = f(x)`. Inside
`DOM`, the unfolding lemma rewrites `f(x)` into `BODY` over recursive calls, which equal `f` by the
hypothesis. ∎ By Lemma 1, the translation stays exact or stronger in assertions and exact or
weaker in assumptions. If the axioms are inconsistent, every lemma is provable, so the
consistency probe is required.

## 5. Memory safety and speculation

**Proposition 4.** Suppose the functional check passes (bounded or unbounded) and the access check
classifies the design as `speculative`. Then the out-of-range accesses read only existing memory
cells, and their values never influence a **translated** postcondition or frame condition (for
`pass-partial` designs, untranslated clauses are not covered). "Existing" is relative to the
modelled N-cell memory; a deployment with a larger physical array changes which reads are in
bounds.

*Proof.* Cell contents outside the `\valid` range are unconstrained by the assumptions (`anyinit`,
and `Pre` does not mention them). If such a value influenced an asserted output, some choice of
the value would violate the assertion, contradicting the PASS. ∎

`out-of-memory` means an address beyond the physical memory was issued. That is a real
interface-level hazard even if the data is discarded, because a `bram` interface connected to
a smaller BRAM would fault.

## 6. What validation adds (not needed for soundness)

C replay (E-ACSL, ASan/UBSan, the auto-init differential and a concrete evaluator) classifies
every RTL counterexample: `c-contract-violation`, `c-runtime-error` or `c-nontermination` (the C
code is wrong too), `invalid-witness` (the harness admitted an input that `Pre` excludes; by
Lemma 1 this can only come from a weakened assumption), `rtl-c-divergence` (the hardware departs
from the C semantics), or `translation-error`/`undetermined`. On the benchmarks every kill of a
non-equivalent mutant has been confirmed at the C level. The `invalid-witness` cases found
during development led to the exact nested `\valid` rule and the shared out-of-model cells.

## 7. Limits of the statements

* Everything is relative to `N` (array cells). The bound becomes size-independent only with RTL
  whose memory interface does not depend on `N` (future work).
* Sequential-call semantics for designs that write memory. Overlap mode covers read-only designs,
  with in-order completion asserted.
* Trusted: the interface map (T1), the environment (T2) and the tool chain (T3). The shared parser
  of the translator and the concrete evaluator is a common-mode risk; E-ACSL is independent.
