# c2sva optimisation review (2026-11-21)

Scope: `src/c2sva/{pipeline,formal,harness,prove,invariants,hls,translate}.py`, per-case `result.json` ("times") and `prove.json` ("secs", Houdini stats) in `runs/*/cases/**`, `runs/paper_tables.md`, tail of `plan.md`.
Method: read-only; JSON aggregation only; no solver, HLS or Frama-C was run. All hours are summed wall-clock seconds of single cases (CPU-hours of one core each; the sweeps run 6-8 cases at a time). "Main sweep" = E1, E2, E3, E4, E5, E6, E7 (2016 cases, 172.6 h; duplicates and demos excluded). Usage gate: round 1 weekly 46 %, 5 h 0 %; round 2 weekly 49 %, 5 h 19 %; both below the 65 %/55 % stop limits, so the review is complete (2 of 3 allowed rounds used).

## Executive summary

1. **Time is concentrated in a tail of hard cases.** 296 of 2016 cases (> 600 s) cost 84.8 % of all time; the 162 `bounded(k)` cases alone cost 102 h of 170 h (60 %). Medians are small (conformance 19 s, ACSLBench 23 s, ABE 49 s); HLS (13-17 s, Vitis start-up) is the floor for easy cases and is 5.5 % of the total.
2. **Stage share (main sweep):** BMC 48 %, **phys 33 %**, access 7 %, HLS 5.5 %, latency 3.6 %, WP 1.6 %, cover 0.5 %, frontend 0.2 %. Sharing the Yosys elaboration is worthless: `prep` takes < 1 s per task (sby logs), so the proposed "one elaboration for bmc/access/phys/cover" saves < 1 % of time. Reject it.
3. **Biggest, safest win (about 55 h = 31-33 %, verdict-neutral): the `phys` task is a hidden second full functional BMC.** `harness_acc.sv` keeps the postcondition assertions (`harness.py` l. 290-303 only wraps access/phys assertions in `ifdef`), so `access`/`phys` re-prove the whole contract. For ap_memory banks whose address width equals clog2(depth) (the default case) the physical-bound assertion `word < words` is a tautology, yet the run takes up to 1133 s (`E3_abe/Nonmutating__search_n`). Fix: decide it statically and compile out post/protocol assertions in access/phys runs. Side effect: this also fixes a labelling defect (item 12.1).
4. **Second win (about 39 h in the E5 matrix, verdict-neutral):** 16 designs that are already `bounded` at `base` are re-run under 6 more configurations (E5 = 99 h of 172 h; 67.9 h on the 19 designs that are bounded in at least one config). Give configs of bounded designs a short budget or skip them.
5. **Third win (about 30 h wall, verdict-neutral):** the engine portfolio is sequential, so 182 tasks wait a full 600 s for yices before bitwuzla starts. 72 of 233 yices-timed-out BMC runs were rescued by bitwuzla (median 186 s). Start bitwuzla after about 30-60 s in parallel, or run both from the start.
6. **A1 (prove.py): Houdini is 61 % of the 60 h of proof time.** About 26 of its 36.6 h are rounds that hit the 300 s timeout. Concrete defects: a base-filter timeout is not carried over to the next k (14 designs, 14.3 h, all four k values retry the same failing base BMC); the Houdini restarts from scratch for each of k = 4, 8, 12, 16 (52 designs try all four); each round is a fresh `sby` process. Fixing the control flow should save about 25-40 % of A1 time at identical verdicts; a persistent solver session or trace-based candidate refutation would save more but is a larger change.
7. **BMC depth tightening** (latency bound found by doubling, so L overshoots by up to 2x; 47 bounded cases got 70-90 % through the depth): bisect after the doubling. Cheap, verdict-neutral, may turn some `bounded` into complete runs. Gain is uncertain (needs a measurement; I estimate 10-20 % of bounded cases flip).
8. **Mutants:** 98 % of kills occur in < 60 s of BMC (p99 221 s). 17.5 h goes to 34 non-killed mutants running > 600 s. A per-mutant budget of 120 s per engine saves about 6 h and cannot change a kill, but turns a few `pass` mutants into `bounded` (a precision change on survivors only).
9. **What can change verdicts:** only integer-width reduction, memory/N abstraction, candidate-cap changes, lowered `lat_cap`, and early-termination rules that cut the BMC budget. Items 1-6 and 12.1 change no verdict (12.1 corrects labels).
10. **Hazards found while profiling** (section 12): mis-labelled `out-of-memory` (230 of 321 labels come from failed postconditions, not from the physical-bound assertion), a global `pkill -f` that can kill other cases' solvers when jobs run in parallel, the 600-candidate cap that truncates the pool in 12 of 24 unproven ABE designs, and `memout` being folded into TIMEOUT/bounded.

## 1. Measured profile

### 1.1 Main sweep by run (hours, sums of case times; n = cases)

| run | n | HLS | latency | bmc | access | phys | total |
|---|---|---|---|---|---|---|---|
| E1_conformance | 59 | 0.2 | 0.0 | 0.4 | 0.0 | 0.0 | 0.6 |
| E2_conformance_cfg | 187 | 0.8 | 0.0 | 3.9 | 0.0 | 0.1 | 5.0 |
| E3_abe (82 designs, 2 mutants) | 210 | 0.9 | 1.2 | 11.7 | 2.1 | 10.1 | 26.8 |
| E4_acslbench | 1019 | 3.8 | 1.9 | 13.3 | 3.8 | 7.2 | 32.2 |
| E5_abe_cfg (6 configs) | 492 | 2.1 | 2.5 | 48.7 | 6.8 | 38.1 | **99.2** |
| E6 (N=2, N=8) + E7 (N=8, 16) | 49 | 0.1 | 0.0 | 3.2 | 0.0 | 2.6 | 6.1 |
| all runs on disk (2399 cases) | 2399 | 9.8 | 6.5 | 86.6 | 13.1 | 58.3 | 179.2 |

Share over all 2399 cases: frontend 0.2 %, WP 1.6 %, HLS 5.5 %, latency 3.6 %, BMC 48.3 %, access 7.3 %, phys 32.5 %, cover 0.5 %.

### 1.2 Per suite (originals + mutants, status ok)

| suite | cases | sum h | median s | p90 s | p99 s | max s | bounded cases | bounded hours |
|---|---|---|---|---|---|---|---|---|
| conformance (E1/E2/E6) | 280 | 6.5 | 19.9 | 45.8 | 1217 | 1223 | 14 | 4.7 |
| ACSL by Example, base (E3) | 190 | 26.7 | 48.8 | 2442 | 3376 | 3517 | 25 | 17.7 |
| ABE x 6 configs (E5) | 432 | 99.2 | 264 | 2457 | 3499 | 4038 | 95 | 64.7 |
| ACSLBench (E4) | 889 | 31.9 | 23.3 | 121 | 2450 | 2926 | 22 | 11.3 |
| ABE N=8/16 (E7) | 15 | 5.2 | 872 | 2481 | 2481 | 2487 | 6 | 4.1 |

T9 in `paper_tables.md` (median per stage) is dominated by the easy cases: ABE median BMC is 83 s, every other stage is under 2 s except HLS. The mean is a different story, see below.

### 1.3 The tail

- Cases by total time: > 100 s: 478 cases = 92.3 % of time; > 600 s: 296 = 84.8 %; > 1200 s: 244 = 77.8 %.
- Outcome x time: bounded 162 cases = 102.5 h; pass 1108 = 51.4 h; fail (kills) 439 = 7.3 h; pass-partial 81 = 5.1 h; vacuous 16 = 3.1 h. Originals: 1109 cases = 142.4 h (146 bounded = 91.7 h); mutants: 697 = 27.1 h.
- Task time buckets (a task that ran 2 engines to timeout shows >= 1190 s; a yices timeout followed by a bitwuzla success shows 590-1190 s):

| task | < 100 s | 100-590 s | yices TO then bitwuzla ok | both TO (>= 1190 s) |
|---|---|---|---|---|
| latency | 1758 (1.6 h) | 44 (2.5 h) | 1 (0.3 h) | 3 (1.2 h) |
| bmc | 1440 (2.6 h) | 133 (9.1 h) | 73 (16.1 h) | **160 (53.3 h)** |
| access | 1146 (0.6 h) | 3 (0.1 h) | 24 (6.1 h) | 18 (6.0 h) |
| phys | 747 (2.0 h) | 106 (7.4 h) | 52 (11.7 h) | **111 (37.0 h)** |
| cover | 1801 (0.6 h) | 5 (0.2 h) | 0 | 0 |

- Engine log: yices finishes 1573 BMC tasks (median 0.8 s, p90 78 s, p99 449 s); bitwuzla rescues 72 (its own time: median 186 s, min 2.4 s, max 581 s) and times out in 161.
- Top-time cases: `Sorting/partial_sort` (4038 s under reshape2; bmc 1200 + phys 1200 + latency 1589), `heap_sort`, `Mutating/shuffle` (bmc 1200 + access 1050-1135 + phys 1200), ACSLBench `task_156` (latency 335-928 s, bmc 1200, phys 1200). Typical bounded case: 2 x 600 s latency/BMC + access + 2 x 600 s phys; BMC reached 22-50 steps of a depth of 68-132.
- BMC step cost grows steeply (partial_sort, bitwuzla: step 22 reached after 9 min 55 s); the depth, not the elaboration, is the lever.
- Bound inflation: reached-step / depth histogram for bounded cases (0.1-step buckets): 0.4: 23, 0.5: 37, 0.6: 19, 0.7: 29, 0.8: 6, 0.9: 12. Latency bound L in bounded cases: 64 (52), 32 (44), 128 (19), 16 (21). L comes from doubling (`formal.discover_latency`, l. 123-137), so it may be up to 2x too large.
- Kills: n = 403 killed mutants; BMC time p50 0.6 s, p90 1.5 s, p95 5.4 s, p99 221 s, only 9 > 60 s. The CEX step is about 0.5 x L (median).
- Non-killed mutants: 252 cases, 19.8 h; 34 of them > 600 s = 17.5 h (mostly `pass`/`bounded` after both engines).
- Latency search: 48 cases > 100 s = 4.0 h; killed mutants spend 1.7 h in latency doubling before their (typically fast) BMC (e.g. `Nonmutating/search` mutant-2: latency 534 s, BMC 108 s; `equal_range2` mutant-2: latency 533 s, BMC 1.5 s).
- HLS: median 13-17 s regardless of design (tool start-up); 9.8 h total; 1885 unique (source, config, top) triples among 2271 HLS runs, so only 1.8 h (18 %) of HLS time is exact re-runs.

### 1.4 A1 (prove.json, 123 cases: E3 72, E1/E6 51)

- Total 60.0 h: Houdini 36.6 h (61 %), PDR 6.6 h (300 s timeouts almost always), plain k-ind 2.35 h, kind+inv 1.1 h, ablation ~1 h, the rest kmin/latency strategies.
- By result: unbounded 92 cases = 17.4 h (median small); not proven 31 cases = **42.6 h** (median 4778 s).
- Houdini final status of the 31 unproven: inductive set found but contract not proved: 14 (14.9 h); **base timeout 14 (14.3 h)**; induction timeout 3 (3.9 h). The 14 base timeouts all end at k = 16 after trying k = 4, 8, 12, 16.
- `k_tried` = (4,8,12,16) in 52 of 123 cases; (4,) in 47.
- Round logs (E3, rounds that finished and wrote an `sby.log`): 845 base rounds (mean 16.5 s) + 1602 ind rounds (mean 14.2 s) = 10.2 h. The remaining ~26 h of Houdini time is therefore in rounds killed by the 300 s timeout (no log). Worst cases: equal_range2 (289 rounds), pop_heap (278), sort_heap (269), make_heap (209).
- Candidate caps: `max_cands=600` (`invariants.py` l. 327) reached in 12 of the 24 unproven E3 designs.

## 2. Ranked optimisations

Savings are measured by replaying the recorded times; "stack" numbers are given in section 4.

| # | Optimisation | Where | Est. saving | Risk | Verdict effect |
|---|---|---|---|---|---|
| 1 | Static decision of `phys` + compile out post/protocol asserts in access/phys | `harness.py` l. 290-303, `translate.py` l. 398-403, `pipeline.py` l. 149-170, `formal.py` | **-57 h (33 % of total)**; also `speculative?(timeout)` (111 cases) becomes a definite label | low | none (corrects labels, 12.1) |
| 2 | Cut the config matrix for designs bounded at base | `cli.py` run (E5 invocation), `scripts/run_campaign.sh` | **-39 h** (E5 after #1) | low | none for pass/fail; configs of bounded designs not checked |
| 3 | Parallel/staggered solver portfolio | `formal.run_portfolio` l. 108-119, `run_task` l. 90-110 | **-30 h wall** (182 tasks x 600 s), more in the bounded tail | low-medium (memory) | none |
| 4 | Houdini control-flow fixes (carry base timeout, k ladder, depth carry) | `prove.houdini` l. 78-124, `prove_case` l. 168-189 | -10 to -15 h of the 60 h A1 | low | none (an "unknown" stays unknown; some designs would be cut earlier) |
| 5 | Tighten latency bound by bisection after doubling | `formal.discover_latency` l. 123-137 | turns some bounded into complete; -10-20 % of bounded BMC time | low | more decisive only |
| 6 | Run BMC before finishing the latency search for mutants | `pipeline.run_case` l. 119-130 | -1.7 h (killed mutants) | low | none |
| 7 | Per-mutant budget (120 s per engine, escalate only survivors) | `Settings.sby_timeout`, `run_case` | -6 h | low | survivors may switch `pass` -> `bounded` |
| 8 | Reorder/short-circuit access: run `phys` only if needed; run `cover` and `access` concurrently with `bmc` | `pipeline.run_case` l. 144-168 | wall time only | low | none |
| 9 | Houdini candidate refutation from traces (evaluate all candidates on every step of every BMC/CTI trace) | `prove.houdini`, `invariants.emit` | rounds 2-5x fewer on big pools (not measured) | medium | none (candidates only refuted if false on a real reachable trace) |
| 10 | Persistent solver session for Houdini (one elaboration, assumptions toggled) | new, `prove._sby` | larger; per-round fixed cost is small, so gain is mainly from avoiding re-unrolling | high effort | none |
| 11 | Batch Vitis runs / HLS cache | `hls.run_vitis` l. 190-250 | -1.8 h (cache) up to ~4 h (batched start-up) | low | none |
| 12 | Narrower integer width W | `pipeline.base_width` l. 40-45, run_case l. 83-88 | unknown (needs experiment); potentially large for products | **medium-high** | can change |
| 13 | Memory/N abstraction (cell-at-anyconst snapshot) | `harness.py` l. 100-160 | unknown | **high** | can change |
| 14 | kmin ladder search (bisection, monotone pools) | `prove_case` l. 216-252 | -30-50 % of kmin sweeps | low | none |

### 2.1 Details

**#1 phys/access as a hidden functional BMC.**
- `harness.generate` emits post assertions unconditionally (`guard = ...; w(f"    if ({guard}) assert({p.cond});")`, l. 295-303); only `kind in ("access", "access_phys")` lines are wrapped in `ifdef CHECK_ACCESS/CHECK_PHYS`. `check.sby` reads `harness_acc.sv` for both tasks, so both prove the full contract to depth L+4 plus one extra property.
- Evidence: `Sorting/partial_sort`: access log finishes with a FAIL in 3 s (a real speculative read), then phys runs bitwuzla to step 21 in 8 min and times out, i.e. the same cost profile as `bmc` (step 22 in 9 min 55 s). The phys assertion in that harness is `!a_ce0 || (addr < 72'sd4)` with a 2-bit address, a tautology.
- Estimated tautologous phys cases (ap_memory, no `byte_addr`, 2^aw <= words): 469 'speculative' (18.8 h), 101 'speculative?(timeout)' (33.7 h), plus 230 whose phys run FAILed on a postcondition (0.7 h, see 12.1). Total phys time removable: 57.3 h.
- Implementation: (a) in `translate.build_properties` skip the `access_phys` property when `2**bank.aw <= words` (or mark it "discharged by construction" like `p.kind == "discharged"`), and set `access = "speculative"` statically when the `access` task fails and no `access_phys` property exists; (b) wrap `post`/`protocol` assertions in `` `ifndef CHECK_ACCESS_ONLY `` and pass `-DCHECK_ACCESS_ONLY` in the `access` and `phys` script lines of `formal.sby_text`; the cone of influence then contains only the address cone, the assumptions, and the call-environment constraints. Then `access`/`phys` become cheap safety checks that can also be k-induction candidates. (c) Keep `harness_acc.sv` with posts for `prove.py` (it reads that file, `prove.py` l. 135) and use a separate `harness_mem.sv` for access/phys, or make the macro default on.
- Risk: the `access` verdict could change from `undecided` (60 cases, hidden by an earlier post failure at a shorter depth) to a definite result; that is more precise, not different.

**#2 config matrix.** E5 = 432 original-design checks, 99.2 h. 16 designs bounded at `base` (E3) account for 38.7 h after #1. Same designs are bounded in 13 of 19 designs under every config (E5 stats: 19 designs bounded in some config, 13 in all). A budget of one engine x 120 s for non-base configs of designs bounded at base keeps the "layout model agrees" evidence (a violation, if any, is found quickly: the kill statistics above) and the paper table value (bounded) unchanged.

**#3 portfolio.** `run_portfolio` runs `engines = ["yices", "bitwuzla"]` sequentially, each with `sby_timeout = 600` (`pipeline.Settings`), and restarts at step 0. Measured: of 233 BMC tasks that exceed 590 s, 73 were rescued by bitwuzla after yices wasted 600 s; 160 failed with both. Parallel version: start yices; after 30-60 s start bitwuzla on a second core; first result wins, kill the other (process group, see 12.3). Wall saving 600 s x 182 tasks (>= 590 s, after #1/#2) = 30 h; CPU cost = at most one extra core-hour for every case that exceeds 60 s (about 500 cases x <= 600 s); memory is the constraint (`MEM_LIMIT` 3.5 GB per process, 15 GB RAM, `--jobs 6-8`: reduce jobs or cap the second engine). Also feed the bound back: record the `steps_checked` of the first engine and, if the second timed out earlier, report the larger `bounded(k)`. Currently only the last engine's step is stored (`run_portfolio` returns `last`).

**#4 Houdini control flow** (`prove.py`):
- l. 95-97: on `TIMEOUT` the base filter halves `bmc_depth` (36 -> 18 -> 16 for k = 16) and, when it cannot shrink further, gives up with "base timeout". The caller loop (l. 168) then starts the next k with `bmc_depth = min(L+4, 36)` again, so a design that cannot pass the base filter even at depth `k` pays 4 x 3 x 300 s. Fix: keep the lowest working/failing depth in `prove_case`; break the k-loop when `hs["status"] == "base timeout"` (the same BMC at depth max(k,8) will time out again for larger k) and when `"induction timeout"` at k (larger k is not easier). Saves most of the 14.3 h + 3.9 h for those 17 designs.
- The base filter is only a filter (comment l. 5-6). With timeouts at 300 s it can be skipped entirely and replaced by the induction rounds' own base check, or limited to a 60 s budget with a smaller depth first (ascending, not descending: refute cheap candidates at depth 8, then 16, then 36; each refuted set is reused through `refuted`).
- k ladder (l. 168): ladder (4,8,12,16) retries Houdini from scratch for each k (rounds 39-71 each for equal_range2/partial_sort). The greatest inductive subset at a larger k is a superset of that at a smaller k, so warm-starting upward is impossible, but warm-starting downward is sound (start Houdini at k' < k from S_k). Cheaper: use (4, 16) when k = 4 fails (a 16-step proof is also an 8/12-step proof), or bisect.
- PDR (`prove_case` l. 150): 6.6 h of 300-600 s timeouts and it succeeded on 6/72 ABE designs (plan.md); run it only after `kind+inv` failed, or in parallel with a kill on first success.
- `kind` (l. 143-149) at k = 4..16 on the plain design: 2.35 h; fine.

**#5 latency bound.** `discover_latency` doubles from `start` (report latency or `lat_start` 8) until the `lat` task passes. The final L is thus in (L/2, L]. After success at L and a FAIL at L/2, bisect the interval with 2-3 additional `lat` runs (each is a short BMC at depth ~L; latency stage = 3.6 % of time). BMC cost per added step is steep (several minutes per step at depth 22 in the bounded partial_sort case), so a 25 % smaller depth is worth much more than a 3 % overhead. Better: take the actual latency from the FAIL cex trace at L/2 (the call's duration) as the lower end. Verdict-neutral, since any L with a PASSed termination BMC at depth L+4 is a valid depth.

**#6 order of stages for mutants.** `run_case` completes latency search up to `lat_cap` = 128 (5 doublings) before BMC. For killed mutants the BMC kill is usually found within 1.5 s (p90). Run BMC first at the first plausible depth (e.g., start+4); only when it passes, run the latency search to certify depth. Kill results are final (a counterexample at any depth is a real counterexample).

**#7 mutant budget.** 98 % of kills in < 60 s BMC. Use `sby_timeout` 120 s per engine for mutants; mutants that survive the short budget are re-run with the full budget only if the C-level classification (`survivors`) cannot establish equivalence. Saves about 6.1 h (bmc part only, measured by capping at 240 s).

**#9 trace-based candidate refutation (A1).** `prove.houdini` drops only the assertions that sby reports in the failing step. BMC base counterexamples (36 steps) and CTIs contain the values of every tapped signal at every step. Writing a VCD/trace (sby `--vcd`, already used by `replay.py`) and evaluating all candidates (Python on the candidate AST) on every reachable base step removes all falsified candidates in one round, instead of the handful per sby run. The log shows 845 base rounds + 1602 induction rounds for 10 h of finished rounds in E3, so the number of rounds, not the solver per round, is what hurts. Not measured, so the gain figure is a hypothesis.

**#14 kmin.** `prove_case` l. 216-252: nested pools none <= hls <= hls+contract <= all (monotone: a larger pool has a Houdini fixpoint that is a superset, so k_min cannot increase). Use the previous pool's `k_min` as an upper bound and a bisection on the ladder rather than ascending from 2 for every pool; reuse the Houdini result at k for the next pool (`refuted_all` already shares base refutations).

**#11 HLS.** `run_vitis` starts a fresh `vitis-run` per case (13-17 s median for tiny designs, i.e. mostly start-up). Cache by hash of (sources, include dirs, tcl) to skip the 1.8 h of exact re-runs (E4fix_*, E4_lifted, re-sweeps). A batched TCL (several `open_project` in one process) could cut the start-up share (rough upper bound 4 h of 9.8 h) but the Vitis state-leak risk makes it a later item.

## 3. Ideas considered and rejected or deferred (with reasons)

- **Share one Yosys elaboration across bmc/access/phys/cover:** `sby ... prep` is < 1 s in all logs inspected (E4 sample: median pre-engine time 0 s, mean 0.03 s; E3 partial_sort: 1 s). Savings < 1 %. Rejected. (Unrolling cost is in the solver, not in Yosys.)
- **Incremental BMC depth reuse between engines/tasks:** the second engine restarts at step 0. Real reuse would need a solver-level session; instead use the cheaper "first engine's `bounded(k)` becomes the lower bound" bookkeeping (#3) and, if the second engine is bitwuzla, sby's `skip`/`append` options cannot skip the solving work. Not worth the effort now.
- **Cache of WP (frontend/Frama-C):** 2.9 h total, 1.6 %. Deferred.
- **More solvers in the BMC portfolio (abc bmc3, rIC3/pono):** useful for the bounded tail but unmeasured here (no solver runs allowed); a 1-hour benchmark on the 20 slowest bounded cases would tell. Mentioned in plan.md as a queued quick win.
- **Multi-threading inside sby:** smtbmc is single-threaded; use the intra-case parallelism of #3/#8 instead. Machine has 20 cores and 15 GB RAM; `--jobs 6` x 3.5 GB RLIMIT_AS can already approach the memory limit, so extra processes per case need `--jobs` to come down in the same proportion.

## 4. Stacked estimate for a re-run of the same campaign (replayed from recorded times)

| step | total hours (single-core sum) | cumulative |
|---|---|---|
| baseline main sweep | 172.6 | 172.6 |
| #1 remove tautologous phys runs (-57.3 h) | 115.3 | |
| #2 skip/shorten E5 configs of 16 base-bounded designs (-38.7 h) | 76.6 | -56 % |
| #3 parallel portfolio, wall (-30.3 h) | about 46 | -73 % |
| #7 mutant budget (-6.1 h) | about 40 | -77 % |

The sweeps' wall-clock is total / jobs; the remaining tail (bounded cases at 2 x 600 s of BMC) is the part that needs real solver improvement (section 5). A1: carrying the base timeout and trimming the k ladder should remove most of the 14.3 h + 3.9 h of futile Houdini rounds and part of the 14.9 h in "inductive but unproved" runs, i.e. an A1 sweep of 60 h becomes about 35-45 h with no verdict change.

## 5. Scalability (N = 8/16 and larger designs)

- N = 8/16 (E7): 15 cases take 5.2 h (median 872 s); 6 are bounded, all spend 2 x 600 s in BMC and in phys. After #1 the phys part disappears. Because BMC step cost increases steeply with state size, N-scaling is mainly a solver question; the effective levers are #5 (smaller depth), the portfolio (#3), and for A1 the unbounded route (k-induction does not depend on depth but does depend on state size).
- State size: `harness.py` l. 100-160 keeps two registers per memory cell (`m_` current, `o_` start snapshot, both `(* keep *)` for replay) plus the post-state `a_e`, `a_o`, `a_s` views whose use in properties is a ternary mux chain over all N cells (visible in `harness_acc.sv`). The `\numof`/quantifier translation unrolls to cut-off N+2 terms of width W (= 72 bits for 32-bit parameters). For N = 16 these grow linearly (mux chains) or quadratically (nested sums). A smaller-state encoding (see #12/#13) is the only way to change the asymptotics.
- k-induction (A1) cost per round grows with the candidate pool (max 600, 20 000 characters per candidate, 1.5 M total characters in `Generator.add`, `invariants.py` l. 384-391). Yosys memory (not time) is the binding limit there.

## 6. Verdict-affecting vs speed-only

| item | class | notes |
|---|---|---|
| #1 static phys, no post in access/phys | speed-only for `rtl`/`outcome`; changes the `access` label (see 12.1) | correct strictly more often |
| #2 config matrix reduction | speed-only for per-case verdicts; reduces coverage of the matrix | results for bounded designs would be reported as "inherited bounded" |
| #3 parallel portfolio | speed-only | same engines; first result wins; both are sound for BMC |
| #4 Houdini control flow | speed-only for proved cases; some "unknown" cases become unknown earlier | can lose a proof only if a larger k would have succeeded after a timeout at smaller k; low probability, report it as "stopped" |
| #5 bisect latency | more decisive (bounded -> complete) | `L` is a proven termination bound, so depth L'+4 with L' < L is sound |
| #6 BMC before latency search | speed-only | counterexamples are final |
| #7 mutant budget | may change survivors' labels (`pass` -> `bounded`); kills unchanged | document in table notes |
| #9 trace refutation | speed-only | candidates are refuted only by reachable traces; Theorem 2 in `docs/theory.md` is unchanged because candidate generation is outside the trusted base |
| #10 persistent session | speed-only | |
| #11 HLS cache | speed-only | key must include tool version and tcl |
| #12 smaller W | **can change verdicts**: a narrower integer can overflow where the ACSL `integer` does not; sound only if a range analysis proves no overflow (loop indices bounded by array size N + cut-off, sums of at most N+2 terms of w-bit values); otherwise spurious failures (precision) or, in assumptions, vacuity | `W = max(64, ret+8, 2*pw+8)` (`base_width`) and the `need` rule in `run_case`; per-expression widths are an option |
| #13 memory/N abstraction | **can change verdicts** (precision and potentially soundness if the abstraction ignores aliasing); sound only with the symbolic-index (anyconst) form for strictly universal properties | |
| lowering `lat_cap` (128) or `lat_start` | **changes verdicts** (more `bounded`/"not established") | the current cap already dominates the cost of non-terminating mutants |
| lowering `max_cands` / raising it | A1 precision only (more or fewer certified invariants); candidates cannot make a proof unsound | raising it increases Yosys memory |
| shorter `sby_timeout` | **changes verdicts** (`pass` -> `bounded`) | the only knob with a direct verdict cost |

## 7. Parallelism settings

- `Settings.sby_timeout = 600` per engine per task; `MEM_LIMIT = 3.5 GB` (RLIMIT_AS, `formal.py`) and Vitis 6 GB; `--jobs` default 6 (`cli.py` l. 412), campaign script 8. 20 cores, 15 GB RAM; the job count is memory-bound, not core-bound. With #1/#2 the long cases are much rarer, so `--jobs` can be raised to 10-12 for the same memory; with #3 (two solver processes per long case) keep the product jobs x 2 x 3.5 GB under about 12 GB.
- Scheduling: cases are submitted in design order, so long cases may cluster at the end of a sweep (makespan = last long case). Order by predicted cost (previous run's `times.total`, or L / clause count) longest first; the data on disk gives the prior for free.
- A1: `c2sva prove --jobs 1` by default and sweeps ran with `--jobs 3` in systemd units; Houdini rounds are independent between candidates but sequential per design, so inter-design parallelism is the only one, which is why three long designs (each 2-3 h) dominate the sweep wall time. Intra-design: run `kind` k = 4/8/16 and `pdr` concurrently and stop on first PASS.

## 8. Incremental / reuse opportunities inside A1

1. Base-refuted set `refuted` is shared across ks and pools (l. 167-170, 224): good.
2. Not shared: (a) base timeouts (see #4), (b) the CTI facts of Houdini rounds at smaller k, (c) the `none` pool results at each k (`kind_k*` is re-run for kmin although `kind` already ran k = 4..16 in the same call; reuse its results when `strategies` include both), (d) the `ablation` pools duplicate the kmin pools `hls` and `hls+contract`.
3. `rtl_signals` (yosys run, `invariants.py` l. 39-48) and `Generator.generate` are re-run on every `prove` invocation; cache them per case (hash of RTL + harness).
4. `latency` strategy (l. 254-281) repeats Houdini on progress candidates with the same `refuted` structure; fine, but it again starts at k = 4.

## 9. Concrete patch list (for the implementer, in order)

1. `harness.py` l. 295-303: emit `` `ifndef CHECK_MEM_ONLY `` around post/protocol/term assertions; `formal.sby_text`: add `-DCHECK_MEM_ONLY` to the `access` and `phys` read lines; `translate.py` l. 400-403: drop `access_phys` when `2**aw <= words` and record that fact in `ir.json`; `pipeline.py` l. 148-170: use `kind == "access_phys"` ids from `pfailed` for the verdict (12.1), skip the phys run if no `access_phys` property exists (label `speculative`).
2. `formal.run_portfolio`: run engines concurrently with a staggered start (`Popen`, `start_new_session=True`, `killpg`), return `max` of steps checked, and replace the global `pkill -f` by the process group kill (12.3).
3. `scripts/run_campaign.sh` E5: pass `--select` computed from E3 results (skip bounded designs) or a `--sby-timeout 120` for non-base configs (needs a CLI flag if missing).
4. `prove.py`: carry `bmc_depth`/timeout across ks; break on base/induction timeout; default ladder (4, 16); run `pdr` after `kind+inv` only.
5. `formal.discover_latency`: after the doubling, bisect between L/2 and L (2-3 extra runs).
6. `pipeline.run_case`: for variants other than `original`, run BMC at depth start+4 before the latency search completes.
7. `hls.run_vitis`: content-hash cache.

## 10. Additional measurements worth taking (cheap, but need CPU, so not done here)

- Time per BMC step vs depth on 3-4 bounded designs with yices/bitwuzla/abc, to calibrate #5 and to decide the engine order in #3.
- The effect of W = 40 vs 72 on a bounded case such as `Sorting/partial_sort` (decides whether #12 is worth a range analysis).
- Whether `-DCHECK_MEM_ONLY` access runs terminate in seconds on the 111 timed-out phys cases (expected).
- Houdini: number of candidates refuted per CTI vs per trace (to confirm #9).
- Record in `result.json`: first-cover step, per-engine `steps_checked`, access/phys per-engine time, and the actual max latency step.

## 11. Data caveats

- Times are wall times of sby/Vitis processes with 6-8 concurrent jobs on a machine that also ran other work; absolute numbers may overstate pure solver time but the ranking is robust (tail and timeouts dominate).
- The tautology test used in section 2 (#1) is estimated from `interface.json` (`aw`, `size`, `byte_addr`); the layout formula for reshaped memories is approximated, so the 57 h could be a few hours off in either direction. It must be confirmed once with the real `words` value from `translate.py`.
- "Duplicate" runs: `E4fix_replay` was excluded from the cross-run sums where it duplicates E4; `prove.json` totals count E1/E3/E6 only.
- A1 time per Houdini round was reconstructed from `sby.log` timestamps (1 s resolution) and only covers finished rounds.

## 12. Defects found while profiling (not strictly optimisation)

1. **`out-of-memory` label can come from a postcondition failure.** `pipeline.run_case` l. 155-165 sets `r["access"] = "out-of-memory"` when the `phys` task is FAIL, without checking that the failed assertion was an `access_phys` one (`pfailed` is computed but unused). Because `harness_acc.sv` also contains the postconditions (see #1), a mutant that violates `ensures` makes `phys` FAIL. Checked in E3/E4: of the 321 `out-of-memory` results, the failing assertion in `sby_phys.log` was a postcondition (`// P2 ensures ...`) in all sampled base cases (230 base-config cases with an `out-of-memory` label have an array memory with aw = clog2(size), where the physical bound cannot fail). The `rtl` verdict is unaffected (these are kills already); the access-outcome counts in the tables (E3/E4 `access`) are inflated and the "0 out-of-memory" claims for originals still hold only because originals pass their postconditions. Fix as in #1, and also check `pid[x].kind == "access_phys"`.
2. **`formal.run_task` kills by pattern** (`formal.py` l. 98: `pkill -f "[c]heck_{task}/"`). The command line of the orphaned solver contains the relative path `check_bmc/model/...`, which is identical for every case. With `--jobs 6-8`, a timeout in one case can SIGTERM the solver of another case, which would show up as an `ERROR` (not matched by the `returned 13[47]|Killed` rule) or be silently retried. No ERROR was found in the main sweep (1 case in `abe_smoke`), so the hazard is unconfirmed, but the fix is cheap: `Popen(..., start_new_session=True)` and `os.killpg` as `prove._sby` already does (`prove.py` l. 53-60).
3. **Out-of-memory is folded into TIMEOUT** (`run_task` l. 105-106 returns `("TIMEOUT", ["memout"], ...)`, so a solver that died at 3.5 GB looks like a 600 s timeout and then the second engine is started). That is acceptable for soundness (bounded), but it wastes the engine switch and hides that the real bottleneck is memory; log it distinctly in `result.json`.
4. **Candidate cap** (`max_cands=600`): in 12 of the 24 unproven E3 designs the pool is exactly 600; candidates are cut in generation order, so what is kept depends on the order of families, not on usefulness. When raising the cap, do it per family quota.
5. **A1 base-filter semantics**: when the base BMC times out at the smallest depth, `houdini` returns `[]` with status "base timeout" and the whole proof attempt for that k is lost, although the base filter is described as "a pure filter" (prove.py l. 5-6). The same candidates would be handled correctly by the induction rounds alone (they check their own base case via k-induction's base steps). Making the base filter best-effort (skip on timeout) turns 14 "no invariants" outcomes into attempted proofs. This one can change A1 verdicts (more proofs attempted), so treat it as a precision improvement, not a pure speed-up.
