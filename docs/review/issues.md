# c2sva issue review (bugs and soundness risks)

Reviewer: read-only static review plus tiny python probes (27 unit tests pass; no sby/yosys/vitis/frama-c run).
Usage check before the single work round: weekly 46 %, 5-hour 0 % (below the 65 % / 55 % stop limits). One round only.
Scope read in full: translate.py, acsl.py, logic.py, harness.py, formal.py, pipeline.py, prove.py, invariants.py,
llm.py, replay.py, oracle.py, survivors.py, frontend.py, mutate.py, most of hls.py; cli.py `_apply_lifted`/`lift`.

## Executive summary (top 5)

1. **LLM guardrail can be bypassed (llm.py:118-160, 196-226)**. The LLM's equivalence lemma is never checked to say
   `name(..) == name_c2sva(..)`; a vacuous lemma (`lemma f_c2sva_eq: \true;`) is proved by WP, status becomes
   `validated-equivalence`, domain `\true`, and the (wrong) definition becomes an override. Even for honest lemmas,
   `domain_of` keeps only the text before the first `==>` (drops `n < 10` in `n>=0 ==> n<10 ==> ...`) and renames
   binders by position (`\forall b, a; a>0 ==> ...` becomes domain `b>0`). The unfolding lemma (Prop 3') is optional for
   acceptance. Soundness.
2. **Overrides keyed by name, global across designs, replace any definition (cli.py:71-77, logic.py:74-90)**. Two designs
   with a same-named but different axiomatic `count`/`fact`/`ordered` overwrite each other, and an override replaces even
   a concrete (non-axiomatic) definition of that name in an unrelated design, with no validation for that design.
   Soundness (assumption can become stronger, assertion weaker).
3. **Out-of-model symbolic reads share ONE symbol `a_xd` for all indices (translate.py:176-180)**. Demonstrated: for
   `ensures \forall integer i,j; i>=4 && j>=4 ==> a[i]==a[j]` the translation is `a_xd == a_xd`, i.e. true, so a false
   postcondition is "proved". The assertion is WEAKER than the ACSL meaning, contradicting Lemma 1 (theory.md section 2).
   Soundness (low likelihood in practice, but an exact violation of the stated discipline).
4. **Operator precedence in the ACSL parser (acsl.py:124-175)**. `a > 0 ==> b ? c : d` parses as `(a>0 ==> b) ? c : d`
   (ACSL: `?:` binds tighter than `==>`), and `a || b ^^ c` parses as `(a||b) ^^ c` (ACSL: `^^` sits between `&&` and
   `||`). Wrong meaning in either polarity; verified with a probe. Soundness risk (silent), cheap to fix.
5. **Coverage/overclaim of "call sequences" and "unbounded"**: (a) BMC depth is `L+4` (pipeline.py:120): only roughly one
   call from reset fits, so cross-call state bugs are missed unless the first call is short; memory contents are never
   re-randomised between calls (harness.py:156-159) and reads beyond the N cells return constant 0 (harness.py:174-179);
   (b) A1 proofs run on the forced-sequential harness (`harness_acc.sv`, pipeline.py:95, prove.py:135) even for
   designs whose bounded check uses overlap mode, so theory.md Theorem 2 ("every run") does not cover overlapped calls.
   Precision/claim issue; A1 itself (Houdini, k-induction, emit, refuted-set reuse, latency reuse of invariants) was
   examined and **no soundness bug was found** (details in section 2).

## 1. Translation and harness (soundness first)

### T1 [soundness, low likelihood] Shared out-of-model symbol for symbolic indices
translate.py:176-180 (`idx` with non-constant index) and 111-114 (`field`): the final default of the mux is
`self.oob(arr, "d")`, one anyconst per array for every out-of-range symbolic index, shared pre/post.
Scenario (reproduced with `build_properties` on a 4-cell memory): `ensures \forall integer i, integer j; i >= 4 && j >= 4 ==> a[i] == a[j];`
gives `P2 = !(q_i>=4 && q_j>=4) || (mux(q_i) == mux(q_j))` where both muxes end in `a_xd`, so P2 is a tautology. In ACSL the
claim (distinct cells beyond the array) is not provable. Same pattern: `\forall i,j; i!=j ==> a[i] != a[j]` becomes
unprovable (stronger, fine) but any positive equality between two out-of-range cells is vacuous (weaker).
Fix: key the out-of-model symbol by the (canonicalised) index expression and by view, so only syntactically identical
indices share a value; otherwise use a fresh symbol per occurrence (sound in both polarities: assertion must hold for all
values, assumption is satisfiable). Keep pre/post sharing only for identical index text (needed for frames).

### T2 [soundness risk] ACSL precedence of `?:` vs `==>`/`<==>`, and of `^^`
acsl.py:125-143 (`expr` applies `?:` AFTER `equiv()`), 144-175 (`lxor` calls `lor`, so `^^` is looser than `||`).
Probe: `parse_expr("a > 0 ==> b ? c : d")` = `ite(a>0 ==> b, c, d)`; `parse_expr("a || b ^^ c")` = `(a||b) ^^ c`.
Per the ACSL reference (operator priority table; please confirm against your Frama-C version) `?:` binds tighter than `==>`
and `^^` binds between `&&` and `||`. A contract written `n > 0 ==> \result == x ? ...` or `p ==> q ? r : s` is mis-parsed:
the assumption can become stronger or the assertion weaker. Frama-C's normalised print usually parenthesises, which
hides this for most benchmarks, but `parse_expr` is also used on LLM output and on `domain` strings.
Fix: restructure as `?:` (right assoc) above `==>` above `<==>`; `^^` between `&&` and `||`. Add parser tests.

### T3 [precision, bool/int mixing in Verilog] comparison results are unsigned 1-bit
translate.py:187-199. `(a<b)` and `==>`/`||` results are unsigned 1-bit; any ordered comparison or arithmetic that mixes
them with the signed W-bit values makes the Verilog expression unsigned (e.g. `\result >= (x > 0)` with `\result = -1` is
evaluated unsigned). Equality is unaffected (same bit pattern after extension). Fix: wrap Boolean-valued operands used as
integers in `$signed({W-1'b0, c})`.

### T4 [precision] Unsupported `assigns` item still yields a (stronger) frame
translate.py:427-448 then 447-476: an item whose form is not supported is recorded in `skipped` (kind `post`), but the
frame property for the arrays is still built from the remaining windows. The unsupported item may legitimately allow
writes into an array, so the frame is stronger than the contract. `pipeline.outcome` only turns a failure into
`fail-inconclusive` for dropped `assume`s (pipeline.py:183) or cut-off values, so this is reported as plain `fail`
(replay then says `translation-error`). Fix: if any assigns item of a behavior is unsupported, skip that behavior's frame
for all objects, or make `outcome` treat failed `assigns` properties with a skipped assigns item as `fail-inconclusive`.

### T5 [precision/soundness-risk] Access-check guard evaluated on live memory
translate.py:389-398. The `access` property uses view `run`, whose memory tag is `s` (the LIVE register `m_*`, harness.py:153),
for `lo`, `hi` and for `!A` of a behavior. If a behavior's `assumes` or a range bound reads memory contents, the guard is
evaluated on the mutated memory, not on the call-start state (the assertion can be weakened because `!A` becomes true after the
memory changed). Fix: use the `o_` snapshot (old view) for `lo/hi/A` in access checks.

### T6 [precision] `(_Bool)x` cast
translate.py:200-206: casts to width 1 are `x & 1`; C/ACSL `(_Bool)x` is `x != 0`. Only matters for `bool`/`_Bool` casts in contracts.

### T7 [precision] Quantifier witnesses outside `0..N` and window enumeration
translate.py:257-263: existential-in-effect assertions enumerate `0..nmax` only (stronger, documented, false alarms possible);
universal assumptions enumerate `0..nmax` (weaker, documented). Sound; noted because `fail` outcomes can come from here.
The `agg` side condition uses the LARGEST array size `nmax` (translate.py:233) even for smaller arrays: for a 1-cell object
an in-range `lo..hi` for nmax still reads out-of-model cells; sound (symbol), imprecise.

### T8 [cross-call sharing of anyconst] free/cut-off/out-of-model symbols are per run, not per call
Symbols are `anyconst`, so call 1 and call 2 of the same run see the same `free*`/`dom_*`/`a_x*` value. In assumptions this makes
the conjunction over calls stronger than per-call ACSL preconditions (a legal sequence needing different out-of-model values
or domain-guard values in two calls is excluded); in assertions it is harmless (universal). Practical impact is small
(cut-off values only matter beyond N+2) but it contradicts "assumptions may only get weaker" for multi-call traces and
for the unbounded A1 claim. Fix: document, or re-havoc these symbols at each `h_start_evt` for the assumption side.

### T9 [verify] multiple `assigns` clauses in one behavior are unioned
acsl.py:540-557 (`cur.assigns.extend(...)`). If the Frama-C version treats several assigns clauses of one contract as an
intersection (each must hold), the union is a weaker frame (assertion weaker). Frama-C's normalised print may already merge
them; please confirm with a two-clause example.

### H1 [coverage, claim] memory is never re-randomised between calls; OOB reads return 0
harness.py:156-159: `m_a_i <= m_a_i`; only the RTL writes memory, initial value `anyinit`. A legal caller changes input
arrays between calls; the harness cannot. Call k+1 therefore sees exactly what call k left, so the Theorem 1 "Coverage" text
(theory.md section 3: "arbitrary inputs ... whatever state earlier calls left") is not true for memory contents (a design
that caches an array element across calls is not exposed). Fix: at `h_start_evt` (idle harness) let `m_*` take an
`$anyseq` value for read-only memories (and for all memories before the first call).
harness.py:174-179: `rd` defaults to 0 for addresses outside the N modelled words, whereas the contract treats out-of-model
cells as solver-chosen values (translate.oob): a hardware bug that consumes a stale/out-of-range value is hidden when 0 is
benign (Prop 4 assumes unconstrained values). Fix: return a free `$anyseq` word for unmapped addresses (access check unaffected).

### H2 [claim] bounded depth covers about one call
pipeline.py:120 (`depth = L + 4`) and formal.py latency search: one call needs up to L cycles, so a second complete call
fits only when the first finishes early. theory.md section 3 says PASS "covers call sequences". Only A1-proved designs do.
Suggest `depth = 2*L + 6` (or report `calls_covered`) at least for designs with persistent state, or soften the text.

### H3 [claim] A1 and overlap mode use different harnesses
pipeline.py:95 builds `harness_acc.sv` with `force_seq=True`; prove.py:135 proves on it. For designs that run in overlap mode
(`itf.control.overlap`, read-only, `toppipe`) the unbounded theorem is for sequential calls only; overlapped in-flight calls
(II < latency) are never covered unboundedly. Also for designs that write memory and overlap is possible, bounded check is
sequential too (documented in `result["mode"]`, but not in the proof reports). Fix: carry `mode` into prove.json and the
tables; or add an overlap-mode proof (FIFO-bounded) for read-only designs.

### H4 [environment, minor] caller cannot assert ap_start in the cycle of ap_done
harness.py:277-282: after `ap_ready` is seen, `assume(!start)` until the cycle after done. A caller that holds `ap_start`
through `ap_done` (ap_ctrl_chain auto-restart style) is excluded. Low impact (the next cycle is covered), documented as the
sequential model; note it in T2 of theory.md.

### H5 [overlap mode] single `o_*` snapshot register and in-order pairing
harness.py:159 and 228-243: the `\old` memory snapshot is ONE register updated at every accept; correct only because
overlap mode is restricted to read-only memories that never change (see H1). If H1 is fixed (memory re-randomised), the
snapshot must become a FIFO like the scalars. Pairing `k`-th `ap_done` with `k`-th accept is asserted only through `PROTO`
(no done without a call), not through ordering; data-dependent latency in a pipelined block could reorder in principle.

### P1 [precision] width W cap silently overflows
pipeline.py:81: `need = min(256, ...)`; with deep products (factorial unrolled to N+2 with 32-bit operands) the cap is hit and
`integer` arithmetic wraps in W bits with no warning (assumption stronger or assertion weaker, either way). Fix: raise
`Unsupported` instead of clamping, or record `width_capped` and force `pass-partial`.

### P2 [cosmetic] `vacuous` label for cover timeout
pipeline.py:150 + 179-182: `cover == "timeout"/"error"` is labelled `vacuous`. It means "not shown non-vacuous".

## 2. A1 (invariants.py / prove.py): examined, no soundness bug found

Checked in detail; each point below is sound as implemented:
* Houdini (prove.py:78-124): candidates asserted with the harness's own contract assertions removed (`emit keep_contract=False`);
  CTI-dropping is valid for k-induction (a k-inductive subset also holds at the last step of any CTI satisfying the whole set at
  steps 0..k-1). `PASS` of `mode prove` includes the base case to depth k, so the certified set is k-inductive from reset.
* `emit` (invariants.py:763-811): candidates are guarded by `!h_init`, both when asserted and when assumed; assumed invariants are
  emitted before assertions; the `anyconst` symbols that `gen.tr` and the contract harness both name `freeN`/`q_x_N` are shared when
  the regex finds a declaration in the harness: this can change a candidate's meaning but not soundness (the candidate is then an
  invariant of the same system, certified in that same system).
* Taps (`connect -nounset -set t_k dut.path`): a wrong or missing tap makes the candidate false or constrains a free wire; it can only
  be dropped or be vacuous, never makes the DUT behave differently.
* Ghost `h_called`/`h_cyc`: pure observers.
* refuted-set reuse across k and pools: refutation is a BMC counterexample from reset without any other candidate assumed
  (`assumed=()` in prove_case main loop, kmin, ablation), hence true for every k and every pool. In the latency block `refuted=set()` is fresh. OK.
* Latency strategy (prove.py:254-281): invariants `inv` certified on `harness_acc.sv` are assumed on `harness_lat.sv`; both are generated
  from the same props with the same assumptions and the same sequential environment (term_only only drops assertions), so the reachable
  sets coincide. `inv` is the set of the LAST k tried (still certified). OK. Minor: `res["kind+inv"]` is unset if `ks` is empty (crash only).
* Remaining A1 issues are claim/scope (H3) and these robustness items:
  - prove.py:192-195: `text` is the harness of the last iteration with non-empty invariants; consistent with `k` of that iteration. OK.
  - invariants.py:94 `var_signals` caps (8 per variable) and regex-name mapping can only lose candidates.
  - prove.py:299-308 `load_case` re-parses with `parse_function(norm, ...)` WITHOUT `logic.set_overrides`; `cmd_prove`/`cmd_revalidate`/`cmd_survivors`
    do not call `_apply_lifted` (cli.py:81 only in `cmd_run`), so for runs made with `--lifted` the oracle sees the axiomatic names as
    unsupported and classifies everything `undetermined`, and A1 candidates lose the lifted clauses (precision only).

## 3. LLM lifting / logic overrides (RQ6, Prop 3')

### L1 [soundness] LLM lemma not checked, domain extraction wrong, unfolding lemma optional
llm.py:
* `parse_response` (118-124) only requires an annotation containing `lemma <name>_c2sva_eq`. `validate` (127-160) proves THAT lemma with WP
  and sets `validated-equivalence`. A lemma `\true`, or one about another function, passes. Nothing checks that the statement is
  `name(args) == name_c2sva(args)` for all args in the premise.
* `domain_of` (196-206): `prem = body.split("==>", 1)[0]` takes only the text before the FIRST `==>`. Reproduced:
  `\forall integer n; n >= 0 ==> n < 10 ==> f(n)==f_c2sva(n)` gives domain `n >= 0`; a premise containing `==>` inside parentheses gives an
  unbalanced fragment (the override is then silently skipped by the `except` in logic.py:80, which is safe). Binder renaming is by POSITION
  against the parameter list (`\forall integer b, integer a; a>0 ==> g(a,b)==g_c2sva(a,b)` gives domain `b>0`).
* `validate` returns `validated-equivalence` as soon as the equivalence lemma is proved; the unfolding lemma (the thing Prop 3' needs) is
  tried but its failure does not change the status (llm.py:152-158). Equivalence on DOM does not imply the unfolding equation when the
  recursion leaves DOM (g(y) != f(y) outside DOM), which is exactly the case the domain guard replaces by a free value.
Failing scenario: LLM answers `/*@ logic integer gcd_c2sva(integer a, integer b) = a; */` and `/*@ lemma gcd_c2sva_eq: \true; */`: WP proves
the lemma, consistency probe (\false) fails to refute, status `validated-equivalence`, domain `\true`; every use of `gcd` in requires/ensures is
now `a`. Contracts with wrong lifted definitions can pass (assertion weaker) or exclude legal inputs (assumption stronger).
Fix: (i) ignore the LLM's lemma text; generate mechanically `\forall params; DOM ==> name(args) == BODY[name_c2sva:=name]` from the definition and
a domain the LLM proposes separately, parsed with a real ACSL parser (balanced parentheses, binders mapped by call arguments); (ii) accept only
if THAT unfolding lemma is proved; (iii) re-run the consistency probe with the unfolding lemma and, since the probe is incomplete (unaided
sweep found nothing for task_452), treat `validated` as "relative to the axioms" in the report.

### L2 [soundness] overrides are global, keyed by name only, replace any definition
cli.py:71-77 merges `{n: ov for d in lifted.values() for n, ov in d.items()}`: with two designs defining a different axiomatic `count` (or
`fact`, `ordered`, `Permuta`), the last wins and is applied to BOTH. logic.py:74-90 then does `defs[k] = v` for every arity key `(name, arity)`
with no check that the existing definition is axiomatic/abstract/inductive: a concrete `logic integer count(...) = ...` in another design is
silently replaced; the prelude `strlen` can be replaced too. The override was validated against ONE design's axioms only.
Fix: key overrides by (design id, name, arity, hash of the axiomatic block); in `logic_definitions` refuse to override a definition whose
`kind` is `predicate`/`logic` with a body; apply overrides only for the design they were validated on.

### L3 [precision] domain guard and free values per occurrence
logic.py:85-87 creates one `("free", ...)` node per definition; the translator makes a fresh anyconst per syntactic occurrence
(translate.py:215-218), so `f(x) == f(x)` outside DOM is not provable (assertion stronger, fine) and the value is shared across calls (T8).
Prop 3' as stated ("some choice of the solver-chosen values makes the unrolling equal f") needs ONE choice per distinct argument tuple; with
per-occurrence symbols and equal arguments in two places the choice is still available (each occurrence may pick f(x)), so the proof sketch holds
for assumptions; for assertions it is stronger. OK, but mention in theory.md.

## 4. Replay, oracle, survivors (classification errors)

### R1 [classification] witness validity checked after divergence
replay.py:306-326: `not same` (RTL != C) is tested before `ora["requires_violated"]`. An input that violates `requires` (harness assumption weaker than
Pre) on which C has implementation-defined behaviour and RTL differs is reported `rtl-c-divergence` instead of `invalid-witness`. `report.py:77`
only counts `c-*` classes as confirmed kills, so this is noise, not inflation. Fix: test `requires_violated` first.

### R2 [classification] callee contract failures count as C-confirmed kills
replay.py:282-289: a failing `Precondition`/`Postcondition` in a callee (`f0 != fn.name`) becomes `c-contract-violation` (detail only). If the callee
contract is the unproved/incorrect part, the "confirmed" kill is not a violation of the top contract. Fix: separate class `c-callee-contract`.

### R3 [classification] concrete evaluator windows and `\valid`
oracle.py:125-143 evaluates `\forall/\exists` only over `[-2, N+2]`: a universally false statement whose counterexample lies outside the window is
"true" (then `translation-error`), an existential whose witness lies outside is "false" (then `c-contract-violation`, a false confirmation).
oracle.py:48 returns `True` for every `\valid` (also in `requires`), so `invalid-witness` for n > N relies on E-ACSL only; when E-ACSL cannot build and the
native fallback is used, an input with n > N is not recognised as invalid. oracle.py:147: `cutoff=8` vs the translator's N+2.
Fix: report `unknown` when a bound variable could range outside the window (no guard seen), evaluate `valid` against the array size.

### R4 [classification] 64-bit unsigned printed as `%lld`
replay.py:175 and survivors.py:56-75: `(long long)` cast of an unsigned 64-bit result/cell above 2^63 prints a negative value; the oracle then evaluates
`\result` negative and `same` for memory cells mismatches RTL's unsigned raw value (`rtl-c-divergence`). survivors.py also feeds values through
`scanf("%lld")`, which overflows for unsigned 64-bit values drawn by `_val` (width 64, unsigned): the C code sees different inputs than the evaluator.
Fix: use `%llu`/`unsigned long long` for unsigned types.

### R5 [classification] survivors ignore behavior requires
survivors.py:104: `for beh in [fn.default]` in `_requires_ok` checks only the default behavior's requires, and `analyse` never looks at
`ora["requires_violated"]` (oracle.py:177 still evaluates the ensures of an active behavior whose requires fail). A vector violating a behavior's
requires on which the mutant violates its ensures is counted `missed-bug`. Fix: check all behaviors' requires (under their assumes) in `_requires_ok`
and skip the vector if `ora["requires_violated"]` is non-empty.

## 5. Robustness (crashes, regex fragility)

* acsl.py:621-627: a 2D array parameter (`int a[4][4]`) makes `parse_function` raise `AttributeError` (verified) instead of `Unsupported`, so the case is
  `error` instead of `unsupported`. Fix: match the whole `[..][..]` suffix and raise `Unsupported`.
* frontend.py:179-180: `hls_ready_source` does not rewrite `const int *restrict a` (verified: ctype `const int *restrict`), the pointer stays a pointer and
  Vitis builds an AXI/ap_none interface or fails. Fix: allow `restrict`/`volatile` qualifiers in the regex.
* acsl.py:567: `FUNC_RE` `[^)]*` parameter list: function pointers, attributes or a `)` in an array size make the contract invisible
  (`no ACSL contract before function`). Contracts mentioning the words `loop|logic|predicate|lemma|axiomatic` in comments are dropped by `find_functions` (acsl.py:575).
* mutate.py:35: `body_span` uses `[^)]*` as well; `AOR` on `*` in a local pointer declaration (`int *p`) yields non-compiling mutants (counted as errors).
* hls.py `module_ports`: widths like `[C_WIDTH-1:0]` make `re.match(...).groups()` raise.
* replay.py:38-74 `parse_vcd`: `ids.setdefault(parts[3], parts[4])` keeps only the first name of an aliased VCD id; any later alias returns 0 via `s.get(name) or 0`
  in `extract.raw` (silently, no KeyError). Currently works because smtbmc writes unique ids; fragile if the VCD writer ever aliases (`a_s0`/`a_e0` share `m_a_0`).
* replay.py:88-95: relies on `h_done_d`/`h_start_evt` snapshot indices equalling BMC step numbers (`fail_step`); correct today (one snapshot per clock-high).
* prove.py:261-280: `res["kind"]`/`res["kind+inv"]` KeyError if `ks` is empty.

## 6. Documentation / claims to adjust (no code change needed)

* theory.md section 2 "Lemma 1": add the exceptions found here (T1 shared OOB symbol; T8 cross-call sharing; T3 signedness; T2 precedence).
* theory.md section 3 "Coverage": depth is L+4, memory is not re-randomised (H1, H2).
* theory.md section 4 Theorem 2 / prove.py docstring: proofs are for the sequential harness (H3).
* theory.md Prop 3': state that the implementation also requires the unfolding lemma only in the `validated-unfolding` path; after fixing L1 make it mandatory.
