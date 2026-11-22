# Demo guide — c2sva, Monday 2026-11-23 09:30

Problem statement: *Automatic Generation of RTL Assertions (LTL/SVA) from C-Level Preconditions and
Postconditions for HLS Designs.* c2sva takes a C function with an ACSL contract, lets Vitis HLS
generate the RTL, translates the contract into RTL properties, and checks them formally.

---------------------------------------------------------------------------------------------------
## 0. Before the demo (09:00, 5 min)

```bash
cd ~/cbv && source setup/env.sh
systemctl --user list-units 'c2sva*'       # should be empty: the 06:00 deadline stops all runs
ls runs/paper_tables_F.md                   # final tables (built automatically)
PYTHONPATH=src python -m pytest tests -q    # 30 tests pass
```
If anything is still running: `systemctl --user stop c2sva-frozen-A c2sva-frozen-B`.

---------------------------------------------------------------------------------------------------
## 1. Live demo (about 5 min): one command, the whole flow

```bash
scripts/demo_monday.sh        # press Enter between stages
```
Design: `find` from ACSL by Example, which returns the first index of value `v` in array `a`.
Output: `runs/DEMO_find/`. Stages: contract → Frama-C proof → Vitis RTL → interface map →
properties → formal check → mutants + C replay → proof for all executions → LLM guardrail → tables.

Single commands, for running stages yourself:
```bash
bin/c2sva run --bench abe --select 'Nonmutating/find' --mutants 2 --name DEMO_find   # full pipeline
bin/c2sva prove runs/DEMO_find --originals -k 4,8 --strategies kind,houdini,kind+inv  # all executions (A1)
bin/c2sva run --bench abe --select 'Nonmutating/find' --config cyclic2 --name DEMO_cfg # another HLS setting
bin/c2sva report runs/DEMO_find                                                       # summary table
```

---------------------------------------------------------------------------------------------------
## 2. Where everything is (for `find`)

Case directory `D = runs/DEMO_find/cases/Nonmutating__find/vitis/base/original/`.

| What | File |
|---|---|
| C code + ACSL contract (input) | `benchmarks/raw/acsl-by-example/StandardAlgorithms/Nonmutating/find.c` and `find.h` |
| Contract as normalised by Frama-C | `D/src/framac_norm.c` |
| Frama-C proof of the C code | `D/wp.log` |
| C given to Vitis | `D/src/hls_src/find.c` |
| **Generated RTL (Verilog)** | `D/hls/proj/sol/syn/verilog/find.v` (+ `find_flow_control_loop_pipe.v`) |
| Vitis synthesis report (ports, latency) | `D/hls/proj/sol/syn/report/find_csynth.rpt` |
| **C variable ↔ RTL signal map** | `D/interface.json` |
| **Generated assertions, SVA view** | `D/props.sva` |
| Property list with translation notes | `D/ir.json` |
| Formal harness actually checked (environment + memory model + assertions) | `D/formal/harness.sv` |
| Model-checker runs and logs | `D/formal/check_*/`, `D/formal/sby_*.log` |
| Verdict and timings | `D/result.json` |
| Proof for all executions (A1) | `D/prove.json`, `D/formal/prove/` |
| Mutant with counterexample + C replay | `runs/DEMO_find/cases/Nonmutating__find/vitis/base/mutant-1/{result.json,replay/}` |

The same layout holds for every design in `runs/F*/cases/…`.

---------------------------------------------------------------------------------------------------
## 3. The problem-statement questions: what to say and what to show

### Q1. Untimed C contracts → timed RTL properties
**Answer.** Each C call is one hardware **transaction**: `requires` is *assumed* when the call is
accepted (`ap_start`), and `ensures`/`assigns` are *asserted* one cycle after `ap_done`. `\old(x)` is
a snapshot register taken at the start. No property contains a cycle count, so the properties hold
for every schedule.
**Show:**
* `D/props.sva`: `assume property (h_start_evt |-> …)`, `assert property (h_done_d |-> …)`;
  the equivalent LTL is G(start → pre) ⇒ G(done → post);
* docs/methodology.md §5 (rule table).

### Q2. Relating C variables to RTL signals
**Answer.** The map is read from the Vitis interface report: scalar ports, memory ports and banks,
word packing, byte addresses, handshake signals. For the all-executions proofs, internal registers
are found from Vitis naming and its schedule report (e.g. C `i` → register `i_fu_36` /
`ap_sig_allocacmp_i_1`).
**Show:** `D/interface.json`; `D/hls/proj/sol/syn/report/find_csynth.rpt` (Interface table);
`D/prove.json` ("C loop invariants kept").

### Q3. Variable and dynamic latency
**Answer.** Properties are anchored on `ap_start`/`ap_done`, never on cycle counts, so a
data-dependent latency is handled automatically. The latency bound is found formally and used only
as the check depth. With A1, a latency bound is also proved for **all** executions (16/17
conformance designs).
**Show:** `D/result.json` ("latency", "latency_status": established); docs/methodology.md §6.

### Q4. Array and memory properties
**Answer.**
* Arrays become a symbolic memory model for every Vitis layout: banks, packed words, BRAM byte
  enables, per-element ports.
* `\forall` / `\exists` are translated under a soundness rule: assumptions only weaker,
  assertions only stronger.
* `assigns` becomes an exact frame condition (cells outside it must keep their value).
* `\valid` becomes a memory-access check. It found that Vitis issues *speculative* reads beyond the
  allowed range.

**Show:** `D/formal/harness.sv` (memory model `m_a_*`, snapshots `o_a_*`); `D/props.sva`
(`\forall` clause, frame of `a`); `runs/paper_tables_F.md` T5 (memory-access findings).

### Q5. Robustness across HLS optimisations
**Answer.** The same contracts are checked under different Vitis settings:
* conformance designs: 11 settings (F2);
* ACSL by Example: 6 settings (F5): pipelining off/on, array partitioning, packing (reshape),
  BRAM interface, unrolling.

Only the interface model changes, read again from each report. No C-proved design fails under any
setting.
**Show:** `runs/paper_tables_F.md` T4; optionally run `--config cyclic2` live (section 1) and
compare `interface.json`, where the array is split into 2 banks.

### Q6. LLM assistance with formal validation
**Answer.** The translation itself is **rule-based**, so that it is sound. The LLM only assists:
for contract parts the rules cannot translate (axiomatic definitions such as `gcd` given only by
axioms), it proposes an explicit formula. c2sva accepts it **only if Frama-C proves** it satisfies
the original axioms, and checks the axioms for consistency.
* 3 definitions validated: those clauses became checkable, and 3 more mutants were caught.
* 2 benchmark tasks have contradictory axioms: their "proofs" are vacuous.
* 11/11 deliberately wrong definitions rejected.

LLM output can never make a result wrong. If rejected, the clause simply stays "untranslated".
**Show:** `runs/E4_acslbench/lift/exchange/task_363__gcd.prompt.md` (exactly what is sent) and
`.response.md` (the answer); `runs/E4_acslbench/lift/lift.json` (verdicts); docs/methodology.md §6.2.

---------------------------------------------------------------------------------------------------
## 4. Results to show (runs/paper_tables_F.md, runs/F*/prove_summary.md)

| Table | Content |
|---|---|
| T1 | Clauses translated per suite (96–100 %) |
| T2 | 0 RTL failures on C-proved designs |
| T3 | Mutation score; every kill confirmed on the C code |
| T4 | Robustness across Vitis settings |
| T5 | Memory-access findings |
| A1 | Designs proved for **all executions**, compared with plain k-induction |

Also show the bugs found and fixed by the validation chain (docs/review/FINDINGS.md, plan.md),
**all caught by C replay or Vitis co-simulation**:
* the RAM model lacked Vitis' cross-port write forwarding;
* Vitis' un-reset registers must start at 0 on an FPGA;
* out-of-model array cells shared one symbol.

---------------------------------------------------------------------------------------------------
## 5. Documents to keep open

* `README.md`: usage;
* `docs/methodology.md`: rules, A1 §6.1, LLM §6.2;
* `docs/theory.md`: what a PASS guarantees and its assumptions;
* `docs/review/FINDINGS.md`: review findings and fixes;
* `docs/review/innovation.md`: next research directions;
* `plan.md`: full project log.

**If a live step is slow:** every output already exists in `runs/DEMO_find/`; open the files from
section 2 instead.
