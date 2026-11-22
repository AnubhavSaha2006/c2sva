# Consolidated review findings (2026-11-21)

Sources: four read-only reviews run in parallel:

* [issues.md](issues.md): bugs and soundness;
* [optimisation.md](optimisation.md): speed and scalability;
* [improvement.md](improvement.md): reviewer-style critique;
* [innovation.md](innovation.md): top-10 novel ideas, from the research log R1–R32.

Every claim taken from a review was **checked by the main session** before acting (reproduced, or
confirmed with Frama-C/Yosys). Status: ✅ fixed · 🔄 running · ⏳ planned · ✗ rejected.

## 1. Soundness and correctness (from issues.md and optimisation.md §12)

| # | Finding | Check | Status |
|---|---|---|---|
| S1 | LLM guardrail bypass. A vacuous equivalence lemma (`\true`) was accepted, the domain kept only the first premise, and binders were renamed by position. | Reproduced: `fact_c2sva = 42` with lemma `\true` | ✅ Acceptance now requires the unfolding lemma that c2sva builds itself (Prop. 3'). The domain is the conjunction of all premises, renamed through the `f(...)` call arguments. The attack is rejected; the 3 real lifts stay validated. Tests added. |
| S2 | Lifted overrides were global and keyed by name across designs, and could replace concrete definitions. | Code read | ✅ Overrides are per design and thread-local (`pipeline.LIFTED[design]`), and only replace axiomatic, inductive or abstract definitions. |
| S3 | One out-of-model symbol `a_xd` was shared by every symbolic index, so `\forall i,j≥4. a[i]==a[j]` was "proved". | Reproduced | ✅ One symbol per distinct index expression (`_ixkey`). Test added. |
| S4 | Out-of-model and cut-off values were constant over the whole trace, which ties the assumptions of different calls together (environment stronger). | Code read | ✅ They are now chosen afresh at each call start and held during the call. Quantifier witnesses in assertions stay trace-constant, which is exact. |
| S5 | Parser precedence: `^^` sat below `\|\|`. | Checked against `frama-c -print`: `&& > ^^ > \|\| > ==> > <==> > ?:` | ✅ Fixed and tested. The `? :` claim from the review was **wrong**: Frama-C groups `a ==> b ? c : d` as `(a ==> b) ? c : d`, as c2sva already did. Contracts were never affected, because Frama-C's normalised print parenthesises mixed `^^`. |
| S6 | `out-of-memory` label: the access/phys runs kept the postcondition assertions, and any phys failure was labelled out-of-memory (230 of 321 labels come from postconditions). | Code read and confirmed | ✅ Postconditions are compiled out of the access/phys runs (`C2SVA_ACCESS_ONLY`), and `out-of-memory` requires a failed physical-bound assertion. The T5 "bram ⇒ 58 out-of-memory" finding must be **re-measured**. |
| S7 | A global `pkill -f check_<task>/` on timeout could kill other cases' solvers (parallel jobs ⇒ spurious `bounded`). | Code read | ✅ Process-group kill. Results computed before the fix need the frozen re-run (§4). |
| S8 | A1 on overlap-mode designs uses the sequential harness, so Theorem 2 does not cover overlapped calls. | Code read | ⏳ State it in theory.md and the paper (scope). |
| S9 | The BMC depth `L+4` covers about one call from reset, and memory is not re-randomised between calls. | Code read | ⏳ Claim fix ("a call from reset plus whatever state the first call leaves"). Multi-call coverage comes from A1 (unbounded). |
| P1 | Precision issues (narrow `_Bool` cast, W cap, frames from unsupported `assigns`, replay order, `%lld` for unsigned 64-bit), plus crash and fragility issues (2-D arrays, `restrict`). | Code read | ⏳ Batch after the paper-critical items. |
| A1 | Houdini, k-induction, `emit` assumptions, ghosts, taps, refuted-set reuse, latency reuse of invariants. | Review found no soundness bug | — |

## 2. Optimisation (from optimisation.md): 172.6 h single-case total → about 40–46 h estimated

| # | Item | Gain | Verdict-neutral? | Status |
|---|---|---|---|---|
| O1 | Postconditions out of access/phys (= S6) | about 55 h (−32 %) | yes (corrects labels) | ✅ |
| O2 | Parallel solver portfolio: start Bitwuzla after 30–60 s instead of after Yices' 600 s timeout | about 30 h wall | yes | ⏳ |
| O3 | Do not re-run configs of designs already `bounded` at base (or give them a short budget) | about 39 h (E5) | yes, with a stated budget | ⏳ (affects the re-run design) |
| O4 | Houdini: carry base-filter timeouts across k (✅ done); trace-based candidate refutation; persistent solver session | 14 h + | yes | partly ✅ |
| O5 | Bisect the latency bound after doubling (L overshoots up to 2×) | uncertain | yes | ⏳ |
| O6 | Mutant budget of 120 s per engine (98 % of kills < 60 s) | about 6 h | no (some `pass` → `bounded`) | ⏳ optional |
| ✗ | Shared Yosys elaboration across tasks | < 1 % | — | rejected (measured: `prep` < 1 s) |

## 3. Evaluation and claims (from improvement.md)

**Accepted, in priority order:**

1. **True positives on RTL-origin faults.** All kills are C mutants. 🔄 E8 `unsafe_dep`: a false-dependence pragma injected into Vitis (innovation #4). ⏳ Testing/co-simulation baseline on the same faults.
2. **Specification-free baseline.** 🔄 QED-FC (A-QED-style) on E1: it passes or times out on every contract-killed mutant. ⏳ Table T10.
3. **"Unbounded" is over-sold.** k_min/ρ reporting is implemented. Headline numbers must give N=4 and ρ. 🔄 N-scaling (E7, N = 8/16) is the real test.
4. **RQ6 is thin.**
   - ✅ Guardrail hardened (S1).
   - ✅ Wrong-definition rejection: 11/11 wrong definitions rejected, 0 false acceptances.
   - ⏳ An LLM-only SVA baseline.
   - ⏳ A held-out lifting set and NL→ACSL on ACSLBench's `nl` field.
   - ⏳ Log model, temperature and prompts. The API backend logs them; the subagent runs are documented as such.
5. **Realism (MachSuite/CHStone, m_axi, globals).** ⏳ Largest effort (6–10 person-days). Globals are the largest single cause of untranslated clauses (~57).
6. **Stale or inconsistent numbers across documents.** ⏳ One frozen end-to-end re-run with the final code, then regenerate every table from it (§4).
7. **Second HLS tool.** ⏳ Run Bambu on the conformance designs, or drop "tool-agnostic".
8. **Over-claims listed in improvement.md §4.** ⏳ Apply to paper_draft.md, methodology.md and theory.md during the write-up.

## 4. Plan of record (what the main session does next)

1. **Frozen re-run with the final code**, using O1/S6/S7 and the soundness fixes S1–S5. Order:
   1. E1 conformance with mutants;
   2. E3 ABE with mutants;
   3. E4 ACSLBench with mutants;
   4. E2/E5 configs with the O3 budget;
   5. E6/E7 N-scaling.

   Then A1 prove (all families, k_min, latency), QED-FC, the lift study, and regenerated tables.
2. **Paper-critical experiments:**
   - E8 unsafe pragmas plus a testing baseline;
   - N-scaling k_min;
   - innovation #3, certificate repair across configs (cheap: data exists).
3. **O2 parallel portfolio**, before the frozen re-run if time allows.
4. **Write-up:** paper_draft.md with the k_min/ρ methodology (innovation #2), the quiescence rule (#1), contract-oracle HLS bug finding (#4), RQ6 guardrails and benchmark defects, and the over-claims fixed.

## 5. Innovation agenda (from innovation.md, ranked)

1. Quiescence rule for source invariants in pipelined RTL (done on E1; the E3 rerun is running).
2. The window theorem and k_min/ρ/F methodology (measured; theorem text to move into theory.md).
3. Certificate repair and proof-carrying RTL across HLS configs (not started; data ready).
4. Contract-oracle HLS bug finding (E8 running; QED-FC baseline running).
5. Certificate reuse across source and LLM rewrites (S3).
6. Size-independent verification (array-memory harness).
7. LLM lifting with bounded-model validation.
8. Source proof as an untrusted partial witness.
9. Contract-guided HLS: verified facts become pragmas.
10. CTI-guided LLM lemma proposer (S2).
