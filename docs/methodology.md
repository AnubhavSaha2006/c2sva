# c2sva — From ACSL function contracts to RTL properties of HLS designs

This document specifies the procedure implemented by `c2sva` precisely enough to be
re-implemented and to support the soundness claims made in the evaluation. Everything is
design-independent: no rule refers to a particular benchmark, and every HLS-specific fact is read
from the HLS tool's own report.

## 1. Setting

Input: a C function `f` annotated with an ACSL contract (requires / ensures / assigns / behaviors,
logic definitions), the RTL module produced for `f` by an HLS tool (Vitis HLS), and the tool's
synthesis report. Output: a set of temporal properties over the RTL ports whose violation
witnesses a call of the hardware block that does not satisfy the contract, together with a
verdict for each property (proved up to a bound, violated with a counterexample, or not
translated with a reason), and an independent C-level validation of every violation.

The contract is assumed to hold at the C level (the benchmarks are proved with Frama-C/WP). The
question answered is therefore *whether the contract survives synthesis*, and, under
mutation, *whether the properties are strong enough to expose implementations that break it*.

## 2. Pipeline

```
C + ACSL ──► Frama-C normalisation ──► contract trees, logic definitions, signature
   │                                          │
   ├──► memory normalisation (pointer → N-cell array) ──► HLS (Vitis) ──► RTL + report
   │                                                               │
   │                                         interface model  ◄────┘
   │                                                │
   └──────────────► translation (rules §5) ──► property IR ──► harness + SVA view
                                                    │
        latency search ──► BMC (functional) ──► BMC (memory access) ──► cover (non-vacuity)
                                                    │ violation
                                  counterexample replay on C (E-ACSL + concrete evaluator)
```

## 3. Front end

* **Canonical contract.** The contract is taken from Frama-C's normalised print of the program
  (`frama-c -print`): headers are merged, macros expanded, implicit `\old` on formals in
  postconditions made explicit, overloads and labels printed uniformly. Parsing the normalised
  form rather than the raw source removes a whole class of benchmark-specific syntax handling.
* **Top function.** Given explicitly, or the root of the call graph among functions that carry
  a contract.
* **Bounded memory model.** Every pointer parameter `T *p` is realised as an array of `N` cells
  (default `N = 4`) if the contract requires a valid *range* on it, and as a single cell if it
  requires a single valid object. The HLS source is rewritten `T *p` → `T p[N]` (C semantics
  unchanged: array parameters decay). The requirement that each valid range fits the `N` cells
  becomes an assumption (§5.4), so all results are *for arrays of at most N elements*.
* **Linking.** Functions called by the top function and defined in other files of the benchmark
  corpus are passed to the HLS tool together with the top file.
* **Header resolution.** Include directories are passed with `-iquote` before `-I`, so benchmark
  headers win over same-named system/libc headers (e.g. `search.h`).

## 4. Interface model

For every C object of the top function the model records the RTL ports and how C-level state
maps onto them. It is read from the Vitis report (`InterfaceSummary`: port → C object,
protocol, direction, width); layouts come from the directives the framework applied.

| C object | Vitis realisation | Model |
|---|---|---|
| block protocol | `ap_ctrl_hs` / `ap_ctrl_chain` (`ap_start, ap_done, ap_idle, ap_ready[, ap_continue]`) | call acceptance / completion (§4.1) |
| return value | `ap_return` | captured at `ap_done` |
| scalar | `ap_none` input | captured at call acceptance |
| single object `*p` | `ap_none` / `ap_vld` / `ap_ovld` ports | one cell: pre-state = input port, post-state = last valid write |
| array | `ap_memory` (`address/ce/we/d/q`, two ports) | RAM model, 1-cycle registered read |
| array, partition cyclic/block *B* | *B* banks `a_0_* … a_{B-1}_*` | `locate(i) = (i mod B, i div B, 0)` / `(i div C, i mod C, 0)` |
| array, reshape cyclic/block *k* | one RAM, words of *k* elements | `locate(i) = (0, i div k, i mod k)` / `(0, i mod C, i div C)` |
| array, partition complete | one `ap_ovld` triple per element | one cell per element |
| array, `bram` mode | byte address `Addr`, byte enables `WEN`, `Din/Dout` | word = byte address ≫ log2(bytes per word) |
| names that are Verilog keywords | renamed `x → x_r` by Vitis | matched through the report's C-object field |

Every later stage uses only `locate` (element → bank, word, slot) and its inverse
(`index_expr`), whose round-trip is unit-tested for every layout.

### 4.1 Calls and environment

A *call* is accepted when `ap_start` is high and the block can take it (idle in sequential mode;
`ap_ready` in overlap mode); it completes at `ap_done`. The environment of the harness:

* resets the block in the first cycle only (with `ap_start` low) and holds `ap_continue` high;
* keeps `ap_start` high until `ap_ready`, and keeps the inputs stable while the call reads them
  (both as combinational assumptions on the current cycle; a clocked assumption would constrain
  the previous cycle only);
* **sequential mode**: a new call is accepted only after the previous `ap_done`;
* **overlap mode** (top function pipelined or its loop auto-rewinds, from the report's
  `PipelineType`; only when no memory is written): calls may be accepted on every `ap_ready`.
  Calls complete in order, so a FIFO of captured inputs is popped at each `ap_done`. A protocol
  assertion checks that `ap_done` never occurs without a call in flight.

The memory-access checks (§5.4) always run on a sequential-mode harness.

## 5. Translation rules

Notation: `pre` = state at call acceptance, `post` = state one cycle after `ap_done` (all memory
writes of the call have landed), `old` = snapshot taken at acceptance. Integer terms are
evaluated in `W`-bit two's complement with `W` chosen so that no ACSL `integer` term can
overflow (§5.6).

### 5.1 Clauses

| ACSL | RTL property | Placement |
|---|---|---|
| `requires P` | assumption `P[pre]` | at call acceptance |
| `ensures Q` | assertion `Q[post, old]` | one cycle after `ap_done` |
| `behavior B: assumes A; requires R; ensures Q` | `A ⇒ R` assumed; `old(A) ⇒ Q` asserted | as above |
| `assigns L` (per behavior) | frame: every cell *not* in `L[old]` has `post = old` | one cycle after `ap_done` |
| `\valid(p + (lo..hi))` | assumption `hi < lo ∨ (0 ≤ lo ∧ hi < N)`; access check: every enabled memory access targets a cell in `[lo, hi]` | acceptance / during the call |
| `\separated`, `\initialized` | hold by construction (distinct RTL memories / registers) | — |
| termination | established by the latency search (bounded), never a timed property | — |

`assigns` is implemented with its exact ACSL meaning, a post-state frame condition, not as a
ban on writes, so transient writes that restore a value are allowed and the rule is
independent of the memory layout. When an object cannot be written by the RTL at all (no
write port / valid signal), its frame condition is *discharged by construction*.

### 5.2 Terms

* variables: formal parameters are read at acceptance (`\old(x) = x` for by-value formals);
  `\result` = return port captured at `ap_done`;
* memory `a[e]`, `*(a+e)`, `*p`: the cell `e` in the requested state (pre, post, old); a symbolic
  index becomes a multiplexer over the `N` cells; a cell outside `0..N-1` is a **solver-chosen
  value** (never 0), which is sound for assumptions (weaker) and assertions (stronger), and
  the same in the pre- and post-state (the RTL has no storage for it and cannot change it);
* `\valid(a + (lo..hi))` nested in an expression (e.g. under a disjunction in a predicate)
  means exactly `hi < lo ∨ (0 ≤ lo ∧ hi < N)`;
* arithmetic `+ - * / %` (C truncation semantics), bitwise ops, shifts (`>>` arithmetic on
  integers), comparisons including chains `a ≤ b < c`, `? :`, `\abs`, `\min`, `\max`;
* casts `(T)e`: reduce modulo `2^w` and sign-extend if `T` is signed (exact C conversion);
* `\let`, `\at(e, Pre|Old|Here|Post)`, user labels mapped through predicate calls.

### 5.3 Quantifiers and aggregates — the polarity rule

Let a quantifier be *universal in effect* if it is `\forall` under positive polarity or
`\exists` under negative polarity (polarity flips under `!` and on the left of `⇒`).
An assumption may only be made **weaker**, an assertion only **stronger** (or kept exact):

| Position | Universal in effect | Existential in effect |
|---|---|---|
| assertion | fresh **symbolic constant** chosen by the solver (exact) | enumerate `0..N` (stronger) |
| assumption | enumerate `0..N` (weaker) | **not translated** (would strengthen) |

`\sum(lo, hi, λk. e)` and `\numof` are enumerated over `0..N-1` with the side condition
`hi < lo ∨ (0 ≤ lo ∧ hi ≤ N-1)`, which is conjoined to an assertion and used as a guard of an
assumption. Quantified variables range over array indices in all benchmark contracts; the
enumeration bound includes `N` because ranges are routinely written `0 ≤ i ≤ n` with `n ≤ N`.

### 5.4 Memory safety

`\valid` ranges give two checks, run separately from the functional check (Vitis legitimately
issues speculative reads):

1. **access**: every enabled access (read or write) targets a cell inside the `\valid` range of
   the current call;
2. **physical** (only if 1 fails): every access targets a cell that exists in the memory.

Outcome: `ok`, `speculative` (1 fails, 2 holds: an access beyond the range but inside the
memory), or `out-of-memory` (2 fails). Because cell contents are symbolic, a functional PASS
together with `speculative` proves that the speculatively read values never influence a
contract-relevant output.

### 5.5 Logic definitions

* Predicates and logic functions are inlined. Overloads are resolved as in ACSL: by arity, by
  parameter kind (pointer vs. value) and, among the remaining candidates, by exact C type of
  the arguments (a `value_type v` parameter beats a logic `integer n`).
* Pointer arguments are *pointer terms* (base array, offset): passing `\old(a) + k` for a pointer
  parameter `x` turns `x[i]` in the body into `a[k + i]`.
* Memory labels are mapped from the call site (`P{Old,Here}(…)`), and a single-label predicate
  called at `Pre`/`Old` reads the pre-state.
* Bound variables are renamed on every inlining (no capture).
* Recursive definitions are unrolled up to `N + 2` nested calls; deeper calls become
  solver-chosen values (sound in both polarities, as above). For arrays of at most `N` cells the
  base case is reached before the cut-off for every recursion over an index or length.
* Bounded definitions of Frama-C libc axiomatics are provided for `strlen`, `valid_string`,
  `valid_read_string` (`valid_read_string(s) = \valid_read(s + (0 .. strlen(s)))`, as in the
  libc specification). At the unrolling cut-off `strlen` takes an "infinite" value instead of a
  free one: a string that does not terminate within the unrolled cells cannot satisfy
  `\valid(s + (0 .. strlen(s)))` in an N-cell memory, so this is exact for the bounded model and
  removes inputs that are not representable in it.
* `\valid` ranges are collected *after* inlining, so ranges stated inside predicates
  (e.g. `inv_matrix(a, rows, cols)`) give the same range assumptions and access checks.
* Axiomatic and inductive definitions, global variables, structs, floating point and pointer
  comparisons are **not translated**; such clauses are reported with their reason (§7).

### 5.6 Integer width

`W = max(64, w_ret + 8, w_p·(m+1) + 16)` where `w_p` is the widest parameter, `w_ret` the
return width and `m` the deepest nesting of multiplications/left shifts in the expanded
contract. All terms are sign/zero-extended to `W` bits according to their C type.

## 6. Checking

* **Latency search.** On a sequential-mode harness containing only the termination check, BMC
  with bound `L` starting from the report's worst-case latency (capped) or 8, doubling until no
  call can exceed `L` cycles within the bound, up to a cap (default 128). `L` is used only as the
  BMC depth `L+4` (plus room for three overlapping calls in overlap mode).
* **Functional BMC** of all assumptions, postconditions and frame conditions; solver portfolio
  (Yices, then Bitwuzla on timeout); on timeout the result is reported as *bounded to k steps*.
* **Cover** (non-vacuity): a call completes under all assumptions. A PASS without cover is
  reported as vacuous.

What a PASS means: for every execution from reset of length at most the BMC depth, in which the
environment respects the protocol and every accepted call satisfies the translated
preconditions, with arrays of at most `N` elements, every completed call satisfies every
translated postcondition and frame condition. Untranslated clauses are listed and reported as
`pass-partial`; a failing check with an untranslated precondition is `fail-inconclusive`.

### 6.1 Unbounded proof: carrying the C proof through HLS (A1, `c2sva prove`)

BMC proves the contract only up to a depth. To prove it for every execution from reset, the
contract assertion must be made inductive, and it rarely is on its own: an induction step
may start in an unreachable state (stale loop registers, a loop-init flag cleared while idle,
an input register holding another call's value). The C side already contains the missing
knowledge: the loop invariants, loop assigns and loop variants that Frama-C WP used to prove
the contract. `c2sva prove` maps these artefacts to the RTL and keeps only what the RTL
certifies.

**Candidates** (`invariants.py`). Each candidate has the form `guard ==> body` and is a Verilog
condition over the harness views and *taps* on internal RTL signals.

| family | body | guard |
|---|---|---|
| loop invariant `I` | `I` with each C local `v` replaced by an RTL signal derived from `v` | `h_busy`, `h_busy && S` for every FSM state `S` of the module holding the signal |
| loop variant `V` | `V >= 0` | same |
| loop assigns `L` | every memory cell outside `L` equals its call-start snapshot | same |
| contract assigns | every cell outside `assigns` equals its call-start snapshot | `h_busy` |
| call requires | each `requires` conjunct over the captured inputs and the call-start snapshot (`\valid` ranges as "fits the N-cell memory") | `h_busy || h_done_d` |
| memory read | the registered RAM output `q` holds `a[v]` for a loop index `v` | `h_busy && S` |
| HLS structure | FSM one-hot; FSM idle and every other state only during a call; no cycle counter while idle; inputs and input registers equal the captured inputs; every 1-bit control register (loop-init, pipeline enables, done/start registers) has a fixed value while idle (both polarities proposed) | — |

*Correspondence.* Vitis names the registers derived from a C variable `v` as `v_fu_N`, `v_reg_N`,
`v_K_reg_N`, `ap_sig_allocacmp_v_K` (the value of `v` in the current iteration),
`ap_phi_mux_v_...`, and narrows their width. Every such signal of the flattened design (Yosys
JSON) is a *placement* of `v`; narrowed signals are tried zero- and sign-extended. Which
placement and which FSM state make the C invariant true is not decided by the tool: every
combination is a candidate, and the RTL decides. Quantifiers in a candidate are enumerated over
the modelled cells (a candidate carries no soundness obligation).

*Taps.* The harness declares an undriven wire per internal signal; the SBY script flattens the
design before optimisation and connects it with `connect -nounset -set t_k dut.path.sig`.
(Without `-nounset`, Yosys also detaches the continuous assignments that read the wire.)

**Houdini** (`prove.py`, after Flanagan & Leino). The induction depth k is tried in the order
4, 8, 12, 16 (a k-inductive set stays inductive for larger k); candidates refuted by a BMC trace
are false for every k and are not retried. Guards of loops in Vitis sub-modules are also tried
under the sub-module's `ap_start` (its first FSM state doubles as its idle state). With the
contract assertions removed:
(1) BMC to the depth of the bounded check, drop every candidate reported violated, repeat until
none fails; (2) k-induction (default k = 4) on the remaining candidates; drop the candidates
violated at the last step of the counterexample to induction; repeat until the set passes. The
surviving set `I` is inductive and holds from reset, so it is an invariant of the harness
(environment assumptions included).

**Contract proof.** k-induction and IC3/PDR (`abc pdr`) on the contract, each without `I` and with
`I` assumed. Assuming `I` is sound because `I` was proved separately; it only removes unreachable
states from the induction step. Any FAIL in prove mode is a counterexample from reset (deeper
than the BMC bound if the bounded check passed).

**Results on the conformance designs (E1).** 16/17 proved for all executions (plain k-induction
6, IC3/PDR within 300 s 5); the open design multiplies three inputs. 16 of 21 C loop invariants hold on
the RTL for some placement and state. Ablation at the proving depth: HLS-structural candidates alone
suffice for 12 designs, adding contract-derived ones for 13; the C proof artefacts are necessary for
3 (array-quantified loop invariants), and for ABE `find`.

**Candidate families added for pipelined loops** (found by analysing counterexamples to
induction; all HLS-generic):

| family | meaning |
|---|---|
| harness/RTL coupling | `h_busy ⇒ ¬ap_idle`, idle harness ⇒ RTL idle, stage-valid bit ⇒ `h_busy` |
| pipeline copy | later-stage register = loop counter + c, under `ap_enable_reg_ppX_iterK` |
| lagged loop invariant | quantified invariant at counter `i−1`, `i−2` and at the commit count `i − Σ` stage-valid bits (stores still in flight) |
| store instance | quantified invariant instantiated at the RAM write port: write ⇒ `G(addr) ⇒ E(addr, data)` |
| contract persistence | after a completed call, the postcondition holds while idle (ghost `h_called`) |
| progress / call age | `h_cyc ≤ m·v + c` for loop counters `v` (from loop variants) |

**Minimal induction depth (`kmin`).** Because k-induction at `k ≥ L + F + 1` (L call latency,
F a small flush constant, ≈4 at L = 8) amounts to BMC over a whole call, proofs are reported
with `k_min` per candidate pool (none / HLS / +contract / all / +progress). Only proofs with
`k_min < L + 1` carry inductive content. Conformance (N = 4): the C artefacts give inductive
certificates where the loop header is quiescent (c07: 16 → 2, c09: 16 → 4, c16: 24 → 2). For
initiation-interval-1 pipelines with overlapped iterations (c05, c08, c14), the verbatim C
invariants hold only lagged, and inductive certificates (k_min = 8 < 9) need the coupling,
commit-count and call-age facts as well.

**What an unbounded PASS means.** As in Section 6, but for executions of any length: any number of
calls, any latency, for arrays of at most `N` elements.

**Example (`find`, ACSL by Example).** Plain k-induction fails. IC3 alone times out after 300 s.
Houdini keeps 53 of 75 candidates in 13 s, including both C loop invariants
(`0 <= i <= n`, `\forall k; 0 <= k < i ==> a[k] != v`, with `i` placed on `ap_sig_allocacmp_i_1`,
`i_fu_36` or `i_1_reg_125` depending on the FSM state). k-induction then proves the whole contract
in 0.8 s. On the mutant that starts the loop at 1, the quantified invariant does not survive,
and the contract proof returns a counterexample.

### 6.2 LLM-lifted definitions (RQ6, `c2sva lift`)

Axiomatic, inductive and abstract ACSL definitions are not translated (§5.5). `c2sva lift`
asks an LLM for an explicit (possibly recursive) definition `f_c2sva` and an equivalence
lemma with a domain `DOM`, and admits it only after Frama-C/WP proves, in the original file:

* the **unfolding lemma** `∀x. DOM(x) ⇒ f(x) = BODY[f_c2sva := f](x)`. The axiomatic `f` satisfies
  the explicit definition's recursion equation. This needs no induction; the full equivalence
  lemma is tried too, but usually needs induction, which WP does not automate.
* a **consistency probe**: the axioms plus the proved lemmas do not prove `\false`.

The translator then uses `f(x) = DOM(x) ? BODY : free`. Deeper recursion and arguments outside
`DOM` are solver-chosen, which is sound by the polarity rule because any unrolling can take
the axiomatic value. A failure caused only by properties that contain such cut-off values
is reported as `fail-inconclusive`.

On ACSLBench, 10 prompts (Claude Sonnet as the LLM, prompt-only):

| Outcome | Count |
|---|---|
| validated | 3 (gcd, fact, GCD) |
| inconsistent axiomatisation found (both factorial tasks: their WP proofs are vacuous) | 2 |
| lemma unproved | 2 |
| LLM syntax error | 3 |
| abstract definition, refused | 2 |

With the validated definitions, 3 surviving mutants became C-confirmed kills.

## 7. Independent validation of violations

Every functional violation is replayed at the C level:

1. the violating call (paired in order: the k-th completion belongs to the k-th accepted call,
   also with overlapping calls) is extracted from the solver trace: scalar inputs, pre-state of
   every memory, RTL result and post-state;
2. the original C function (loop annotations removed) is compiled with **E-ACSL** (runtime
   checking of the contract and of C undefined behaviour, `--rte=all`) and run on those inputs;
3. because E-ACSL does not support every construct (it ignores `assigns` and some `\old` forms),
   every clause is also evaluated by an **independent concrete evaluator** on the C pre/post
   states with exact integer semantics;
4. if E-ACSL reports nothing, the same program is run natively with AddressSanitizer and
   UndefinedBehaviorSanitizer (red zones catch out-of-bounds accesses that land in another
   valid object, which E-ACSL accepts); if E-ACSL cannot instrument the program (unsupported
   construct or internal error) this native run replaces it;
5. a failed precondition is attributed to the function it belongs to: in the top function it
   means an invalid witness, in a callee it is a genuine bug (the code called it outside its
   contract); a crash (signal) is a run-time error.

Classes: `c-contract-violation`, `c-runtime-error`, `c-nontermination` (genuine C-level bugs),
`invalid-witness` (input violates a precondition: translation gap), `rtl-c-divergence`
(C satisfies the contract but RTL outputs differ: the hardware does not implement the C
semantics), `translation-error` (C satisfies the contract and equals the RTL: the property is
wrong), `undetermined`.

## 8. Mutation analysis

**Survivors.** A mutant that passes the RTL check although WP fails on it is analysed by bounded
random testing of the mutant C code (inputs satisfying every precondition, drawn with a bias to
small, extreme and repeated values; native run with sanitizers; whole contract evaluated by the
concrete evaluator): `missed-bug` (a contract violation exists: a false negative of the RTL
properties), `memory-safety-only` (undefined behaviour that changes no contract-relevant
output — invisible to functional properties; the access check flags it but also flags benign
HLS speculation), `no-violation-found` (likely equivalent; WP fails only on loop annotations).
WP goal names cannot be used for this classification: WP proves postconditions relative to the
loop invariants, so a broken invariant may be the only failed goal of a mutant that really
violates its postcondition.


First-order mutants of the top function body: relational (ROR), arithmetic (AOR), logical
connector (LCR) and constant (CRP) replacement; sites chosen deterministically, round-robin over
operator classes. A mutant is *non-equivalent* when Frama-C/WP fails on it while the original
proves with the same flags; the mutation score is the fraction of non-equivalent mutants whose
RTL check fails. Kills are confirmed by the C-level replay (§7).

## 9. Limitations

* Arrays of at most `N` cells. Executions are of bounded depth, except where `c2sva prove`
  establishes an unbounded proof (Section 6.1). The prove stage uses the sequential-call harness.
* Interface protocols `m_axi` and `s_axilite`, `ap_ctrl_none`, dataflow regions with streams,
  global state and floating point are outside the model. Structs are supported (by-pointer,
  arrays of structs, by-value returns).
* Overlap mode is restricted to blocks that do not write memory (the C semantics of overlapping
  calls on shared written memory is not defined by the contract).
* Axiomatic/inductive logic is not translated by the rules. Axiomatic definitions can be lifted
  by an LLM and admitted after Frama-C validation (§6.2). Inductive predicates remain open: their
  equivalence needs induction or a bounded fixed-point check.
* The BMC depth L+4 covers roughly one call from reset, plus the state the first call leaves.
  Unbounded call sequences are covered only by A1 proofs (§6.1), and A1 uses the sequential-call
  harness, so overlapping calls are covered only up to the bound.
