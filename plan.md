# plan.md — C-Contract → RTL Assertion Generation for HLS Designs

> Living document. Update the **Status / Log** and **Notes** sections as work progresses.
> Last updated: 2026-10-07

---

## 0. Problem in one paragraph

C functions annotated with **ACSL** contracts (`requires` / `ensures` / `assigns` / behaviors / loop invariants) are synthesized by an HLS tool into RTL. The contracts are lost. Build an **LLM-assisted tool** that takes
`(C source, Pre, Post, HLS metadata, RTL)` → **SVA/LTL assertions bound to the RTL**, and then **checks** (formal + simulation) whether the RTL satisfies them.
Benchmarks: ACSL-by-Example + ACSLBench, filtered to the Vitis-HLS-synthesizable subset.

### Research questions (RQ) we must answer
| RQ | Question | Where answered |
|----|----------|----------------|
| RQ1 | Untimed C contracts → timed RTL properties? | §3.1 transaction model |
| RQ2 | Relating C variables to HLS RTL signals? | §3.2 signal map |
| RQ3 | Variable / dynamic latency? | §3.3 |
| RQ4 | Array & memory properties? | §3.4 |
| RQ5 | Robustness across HLS optimizations? | §3.5 + Phase 6 experiments |
| RQ6 | LLM translation + formal validation loop? | §3.6 + Phase 5 |

---

## 1. Resource analysis (what we have)

| Resource | Content | Use for us | Caveats |
|---|---|---|---|
| **ACSL by Example** (Fraunhofer FOKUS, MIT) | `StandardAlgorithms/` groups: Nonmutating, MinMax, BinarySearch, Mutating, Numeric, Heap, Sorting, Stack. Targets Frama-C 33 (Arsenic) + WP; all contracts **proven** at C level. | High-quality, verified contracts → "gold" set. Rich quantifiers over arrays, `\old`, `assigns`, logic functions/predicates. | Uses `size_type`/pointers `const value_type* a, size_type n` → needs HLS normalization (fixed-size arrays). Heap/Stack use structs + logic axiomatics (harder). |
| **ACSLBench** (HF `yzrsdad/ACSLBench`, Apache-2.0) | 598 tasks; fields `task_id, nl, acsl, c_file, acsl_c`; JSON/CSV + `ACSL_C/task_N.c`. | Scale (hundreds of functions). Mostly integer/array functions, no malloc/IO. | Contracts **not guaranteed proven** → must run Frama-C/WP first and keep only proven ones (otherwise RTL failures are ambiguous). Some will use axiomatics / recursion. |
| **Frama-C** | Parser, WP prover, **E-ACSL** (ACSL → runtime C checks). | (a) Re-prove C contracts; (b) parse/normalize ACSL; (c) E-ACSL gives executable C oracle for co-sim cross-check. | Install via opam; heavy but standard. |
| **Vitis HLS** (AMD, free license) | C→Verilog/VHDL + reports (`csynth.xml`, `*.rpt`, `.adb`), C/RTL co-sim (xsim). | Primary HLS. Reports give interface ports, latency min/max, II, loop tripcounts. | ~80–100 GB install, AMD account; check OS support (kernel here is 7.0 → may need Ubuntu 22.04/24.04 Docker). |
| **Bambu (PandA)** | Open-source C→Verilog, AppImage. | Second HLS backend → robustness RQ5 across tools. | Different handshake (`start_port/done_port/return_port`) and memory protocol. |
| **Google XLS** | DSLX/C++ → Verilog. | Optional third backend. | C++ (XLS[cc]) subset is restrictive; lowest priority. |
| HLSFactory / MachSuite / CHStone | Larger HLS designs. | Stretch: scale study (no ACSL → contracts written by LLM, verified by Frama-C). | Not annotated. |

**Local machine (checked 2026-10-07):** 20 cores, 15 GB RAM, 322 GB free, Python 3.12, gcc, clang, docker. **Not installed:** vitis_hls, bambu, yosys, sby, verilator, iverilog, frama-c, opam, solvers.

---

## 2. Overall architecture

```
             ┌──────────────┐   proven?   ┌─────────────────┐  synthesizable?  ┌──────────────┐
ACSL-C ─────►│ 1. Frama-C   │────────────►│ 2. HLS-normalize│─────────────────►│ 3. HLS run   │
benchmarks   │  parse + WP  │             │  (fixed arrays, │                  │ Vitis/Bambu  │
             └──────┬───────┘             │  pragmas, top)  │                  │ × directive  │
                    │ contract AST        └─────────────────┘                  │   configs    │
                    ▼                                                          └──────┬───────┘
             ┌──────────────┐  ┌────────────────────────┐                             │ RTL + reports
             │ 4. Contract  │  │ 5. HLS metadata extract│◄────────────────────────────┘
             │   IR (JSON)  │  │ ports, protocol, lat,  │
             └──────┬───────┘  │ array→port map (JSON)  │
                    │          └───────────┬────────────┘
                    ▼                      ▼
             ┌─────────────────────────────────────────┐
             │ 6. Translator  (rule-based  +  LLM)     │──► Property IR ──► SVA / observer-Verilog
             └──────────────────────┬──────────────────┘
                                    ▼
             ┌─────────────────────────────────────────┐   fail / syntax err / vacuous
             │ 7. Validate: lint → formal (SBY/EBMC)   │──────────────► feedback to LLM (repair loop)
             │   → co-sim → mutation & vacuity checks  │
             └──────────────────────┬──────────────────┘
                                    ▼
                         8. Results DB + metrics + report
```

**Key design decision:** Do NOT let the LLM emit SVA directly from raw text only. Use a **two-stage, typed pipeline**:
1. Contract → **Property IR** (JSON: kind, trigger, sampled-at, quantifiers, terms referencing *C names*).
2. IR + **signal map** (deterministic, from HLS reports) → SVA text via **templates**.
The LLM helps in stage 1 (and for hard cases like logic functions, `\exists`, behaviors); the rule-based path is the baseline and the safety net. This makes results reproducible and lets us measure LLM value-add (RQ6).

---

## 3. Technical approach per research question

### 3.1 RQ1 — Untimed → timed: the **transaction model**
A C call = one HLS **transaction** delimited by the block-level handshake.

| Vitis `ap_ctrl_hs` | meaning |
|---|---|
| `ap_start & ap_idle` (or first cycle of `ap_start` after idle) | call begins → **sample point for `requires` and `\old(...)`** |
| `ap_done` | call ends → **check point for `ensures`**, `\result` = `ap_return` |
| `ap_ready` | inputs consumed (matters for pipelined `ap_ctrl_chain`) |

Mapping rules:
- `requires P`  → **assume** `ap_start_evt |-> P[inputs]` (formal env constraint) — and optionally an *assertion on the caller side*.
- `ensures Q`   → **assert** `ap_done |-> Q[outputs, ghost_old]`.
- `\old(e)`     → **ghost register** captured at start (`always @(posedge clk) if (start_evt) old_e <= e;`). Mandatory because ports may change during the transaction.
- `\result`     → `ap_return` (valid only when `ap_done`).
- `assigns \nothing` / `assigns a[0..n-1]` → **frame property** on write ports (see §3.4).
- `behavior B: assumes A; ensures Q;` → `ap_done && old_A |-> Q`; `complete/disjoint behaviors` → assert on captured inputs.
- Termination (`terminates`, implicit) → **bounded liveness** `start_evt |-> ##[1:LAT_MAX] ap_done` (or `s_eventually` for unbounded).
- Input stability environment assumption: scalar `ap_none` inputs held stable while busy (`assume`), required for soundness.
- Integer semantics: ACSL `integer` is mathematical; RTL is fixed-width → add **width-extension** (e.g., `$signed({1'b0,x})` to 64-bit) and flag possible overflow; respect C signedness from metadata.

### 3.2 RQ2 — C variable ↔ RTL signal map
Two levels:
1. **Interface level (primary, deterministic):** parse `solution/syn/report/<top>_csynth.xml` (`InterfaceSummary`/`RtlPorts`: port name, direction, width, protocol, C object). Typical Vitis patterns:
   - scalar in `x` → `x` (ap_none); scalar out `*p` → `p` + `p_ap_vld`; return → `ap_return`.
   - array `a[N]` → `a_address0, a_ce0, a_we0, a_d0, a_q0` (ap_memory; `*1` for 2nd port).
   - `m_axi`, `axis`, `s_axilite` → protocol adapters (stretch).
   - Bambu: `clock, reset, start_port, done_port, return_port`, memory via `Mout_*`/`M_Rdata_ram`.
2. **Internal level (secondary, heuristic + LLM):** for loop invariants / intermediate state, use Vitis `.adb`/`.verbose.rpt`/binding info and naming (`i_fu_NN`, `sum_reg_NN`). Fragile under optimization → only for stretch goal; report robustness separately.

Output: `signal_map.json` per (design, config) — this is the "HLS metadata" input to the translator.

### 3.3 RQ3 — Variable / dynamic latency
- Never hard-code cycle offsets for post-conditions: **event-based** (`ap_done |-> Q`) is latency-agnostic.
- Latency bounds from `csynth.xml` (`Best/Worst-caseLatency`); if `?` (data-dependent loop) → derive from `LOOP_TRIPCOUNT` + `requires n <= MAX` or use unbounded `s_eventually`/liveness in formal.
- Pipelined/`ap_ctrl_chain` (overlapping transactions): ghost **FIFO/scoreboard** of captured inputs, tagged per transaction, popped on `ap_done`.
- BMC depth = `LAT_MAX + margin`; if too deep → k-induction or simulation-only for that design (record it).

### 3.4 RQ4 — Arrays & memories
Array args become **external** memories (ap_memory) — contents are not in the DUT. So the verification **harness** instantiates a memory model wired to the ports:
- `\forall integer i; 0<=i<n ==> P(a[i])` → **symbolic index** trick: `(* anyconst *) reg [AW-1:0] k;` assert `P(mem[k])` under `k < n` (sound for ∀, cheap). Alternative: generate-loop over all indices for small N.
- `\exists i; ...` → LLM/rule proposes witness (e.g. `\result`) or use a monitor that records whether a witness was observed; else fall back to simulation.
- `\old(a[i])` → shadow memory snapshot at start (or anyconst index + captured `old_mem_k`).
- `\valid(a+(0..n-1))` / `\valid_read` → **address bound**: `a_ce0 |-> a_address0 < old_n`.
- `assigns a[lo..hi]` → **write frame**: `a_we0 |-> lo <= a_address0 <= hi`; `assigns \nothing` → no `we` port / `we` never high; `\valid_read` only → `we` never high.
- Logic functions (e.g. `Count(a,n,v)`, `Sum`) → ghost accumulators updated in the harness, or recursion unrolled for bounded N (LLM good at this).
- `array_partition` / `reshape` → bank mapping `a[i] ↔ a_{i%B}[i/B]` (cyclic) or block variant; derived from directives.

### 3.5 RQ5 — Robustness across HLS optimizations
Directive matrix per design (generated Tcl):
`baseline`, `PIPELINE II=1`, `UNROLL factor 2/4/full`, `ARRAY_PARTITION cyclic/block/complete`, `INLINE`, `ap_ctrl_chain`, `interface bram/m_axi`, `clock 3ns vs 10ns`, plus **Bambu** as second tool.
Measure for each: % assertions generated, syntactically valid, proven, and whether the *same* Property IR survives with only the signal map changing. Hypothesis: interface-level, event-based properties are robust; internal-signal properties are not.

### 3.6 RQ6 — LLM + formal validation loop
- LLM input: C code, ACSL contract, Property-IR schema, `signal_map.json`, latency info, 2–3 few-shot examples. Output: **Property IR JSON** (structured output / JSON schema), not free SVA.
- Validation ladder (each step's error text is fed back, max K=3–5 repairs):
  1. JSON schema + IR type check (names exist in signal map / C signature),
  2. SVA render → **lint/elaborate** (`pyslang` or `verilator --lint-only`),
  3. **Formal** on HLS RTL (SBY/EBMC, BMC depth from latency) → PASS / FAIL(cex) / UNKNOWN,
  4. **Non-vacuity**: `cover` that `ap_done` is reached with pre true; cover each antecedent,
  5. **Mutation test**: mutate the *C* (off-by-one, wrong comparison) → HLS → RTL must FAIL; or mutate RTL directly. Kill rate = assertion strength,
  6. **Co-sim cross-check**: Vitis cosim with assertions bound (xsim supports SVA) + E-ACSL oracle.
- **Important:** a FAIL on a correctly-proven C contract is either a translation bug, an HLS bug, or a width/overflow semantic gap — triage needed; never let the LLM "fix" an assertion just to make it pass (repair only on syntax/IR errors and on cex classified as translation error; log everything).

---

## 4. Toolchain (pinned, verified 2026-10-07 on Ubuntu 24.04 / glibc 2.39)

| Tool | Version | Purpose | Location |
|---|---|---|---|
| Vitis HLS (Vivado/Vitis unified) | **2026.1** (Ubuntu 24.04 officially supported; needs libtinfo.so.5) | Main HLS + C/RTL cosim | `~/tools/Xilinx` (no sudo); device family: UltraScale+ only |
| Bambu (PandA) | **2024.10** AppImage (needs libfuse2t64) | Open-source 2nd HLS | `~/tools/bin/bambu` |
| OSS CAD Suite | **2026-10-06** nightly | yosys, sby, smtbmc, boolector, bitwuzla, yices, z3, verilator, iverilog, eqy | `~/tools/oss-cad-suite` |
| EBMC | **6.0** (.deb) | Model checker with fuller SVA support | system |
| slang | **v12.0** / pyslang 12.0.0 | SV parse/elaborate of generated SVA | pip |
| OCaml / opam | opam 2.x, OCaml **5.3.0** switch `cbv` | Frama-C build | `~/.opam` |
| Frama-C | **33.0 (Arsenic)** (+ WP, E-ACSL) | Prove/parse ACSL, runtime oracle | opam |
| Why3 / Alt-Ergo | **1.8.2** (Frama-C requires <1.9) / **2.6.4** | WP back-end | opam |
| CVC5 / Z3 | **1.3.4** (matches ACSL-by-Example) / **5.1.0** | Extra SMT provers for WP | `~/tools/bin` |
| Python | 3.12 venv `.venv` + `requirements.txt` | Tool code | project |
| Google XLS | optional, latest nightly | 3rd HLS (low priority) | — |

> ⚠ Open-source Yosys `read -formal` supports only a **subset of concurrent SVA** (`|->`, `|=>`, `##N`, `$past/$stable/$rose`); `##[a:b]`, sequences, `s_eventually` → use EBMC or the **observer-Verilog** back-end (ghost FSM + immediate asserts). Same IR → both.

Setup commands are in the chat history / to be scripted in `setup/`.

## 5. Repository layout (proposed)
```
cbv/
  plan.md  problem_statement.txt  README.md  requirements.txt
  benchmarks/
    raw/acsl-by-example/  raw/acslbench/
    normalized/<id>/{fn.c, fn.h, contract.acsl, meta.json}
  hls/
    configs/*.tcl              # directive matrix
    runs/<id>/<tool>/<cfg>/{rtl/, reports/, signal_map.json}
  src/c2sva/
    ingest.py        # load + Frama-C WP filter
    normalize.py     # pointer→fixed array, top fn, pragmas, MAX_N
    hls_runner.py    # Vitis/Bambu batch runs (Docker-aware)
    metadata.py      # csynth.xml/Bambu → signal_map.json
    acsl_parser.py   # ACSL → Contract AST
    ir.py            # Property IR (pydantic)
    translate_rules.py   # deterministic translator
    translate_llm.py     # LLM translator + repair loop
    render_sva.py  render_observer.py   # backends
    harness.py       # memory models, ghost logic, bind file
    verify.py        # sby/ebmc/cosim drivers, result parsing
    mutate.py        # C/RTL mutants
    eval.py          # metrics + tables
  prompts/  templates/  results/  tests/
```

---

## 6. Phased pathway

### Phase 0 — Setup
- [ ] Install OSS CAD Suite, Frama-C (opam), Bambu, Python venv; start Vitis download (large).
- [ ] Verify Vitis runs on this OS (kernel 7.0); else build Docker image `vitis-hls:22.04`.
- [ ] Smoke test: tiny `add(a,b)` → Vitis → Verilog → hand-written SVA → SBY PASS. **This validates the whole chain end-to-end before anything else.**
- [ ] `git init`, README, requirements.txt.

### Phase 1 — Benchmark curation
- [ ] Fetch ACSL-by-Example (git clone) + ACSLBench (`datasets.load_dataset("yzrsdad/ACSLBench")`).
- [ ] Run Frama-C WP on every function (`frama-c -wp -wp-rte -wp-timeout 10`); keep **fully proven** contracts (record % proven).
- [ ] Static filter (pycparser/clang AST): reject recursion, malloc/free, function pointers, I/O, `long double`, unbounded pointer-to-pointer, library calls not supported by Vitis.
- [ ] **Normalize** for HLS: `T* a, size n` → `T a[MAX_N]`, add `requires n <= MAX_N`, `LOOP_TRIPCOUNT`, set top function; keep contract semantically identical (re-run WP on normalized code!).
- [ ] Dynamic filter: run Vitis `csynth` on each; keep successes → **the HLS-synthesizable subset**. Log rejection reasons (this table is a deliverable).
- [ ] Taxonomy tags per function: scalar-only, array-read, array-write, `\old`, quantifier ∀/∃, logic function, behaviors, struct.

### Phase 2 — HLS runs + metadata
- [ ] Directive matrix Tcl generator; batch runner (parallel, ~4–8 jobs given 15 GB RAM).
- [ ] `metadata.py`: parse `csynth.xml` → `signal_map.json` (ports, widths, signedness, protocol, latency min/max, II, array→port, partition factors). Same for Bambu.
- [ ] Unit tests on a few hand-checked designs.

### Phase 3 — Contract IR + rule-based translator
- [ ] ACSL parser for the subset (or Frama-C `-print` normalized output → Lark).
- [ ] Property IR schema (pydantic): `{id, src_clause, kind: assume|assert|cover, trigger: start|done|always|write, old_vars, quantifier: {var, lo, hi, mode: anyconst|enumerate}, expr, width_policy}`.
- [ ] Rule-based translation for §3.1/§3.4 patterns; both renderers (SVA + observer).
- [ ] Harness generator: memory models for ap_memory ports, ghost `\old` capture, anyconst index, `bind` file, SBY `.sby` / EBMC script.

### Phase 4 — Formal / simulation verification
- [ ] `verify.py`: run SBY (bmc + cover; prove/k-ind where feasible) / EBMC; parse PASS/FAIL/UNKNOWN/timeout; save VCD cex.
- [ ] Vitis cosim with bound assertions as 2nd checker for designs too large for formal.
- [ ] Vacuity checks (cover) mandatory for every assertion.
- [ ] **Milestone:** rule-based pipeline end-to-end on ≥ 30 ACSL-by-Example functions.

### Phase 5 — LLM translator + repair loop
- [ ] Prompt with schema + signal map + few-shot; structured JSON output; temperature 0; cache responses (hash of inputs) for reproducibility.
- [ ] Repair loop on lint/type errors and translation-classified cex (max K iterations).
- [ ] Hybrid mode: rules first, LLM for clauses rules can't handle (logic functions, ∃, behaviors, complex `\old`).
- [ ] Ablations: rules-only vs LLM-only vs hybrid; with/without signal map; with/without repair loop; 2 LLMs.

### Phase 6 — Robustness & strength experiments
- [ ] Run across the directive matrix + Bambu (RQ5).
- [ ] Mutation analysis: C-level mutants (relational op swap, off-by-one bounds, wrong index) → HLS → assertions should FAIL; report kill rate.
- [ ] Optional: seeded RTL bugs directly in Verilog.

### Phase 7 — Evaluation & write-up
Metrics table per method × benchmark × config:
- Clause coverage (% ACSL clauses translated), syntactic validity, **proof rate on correct RTL**, false-failure rate, UNKNOWN/timeout rate, vacuity rate, **mutant kill rate**, LLM iterations & tokens/cost, runtime.
- Answer RQ1–RQ6 explicitly; case studies (one success, one interesting failure each).

---

## 7. Worked example (target shape of output)

```c
/*@ requires 0 < n <= 64;
    requires \valid_read(a + (0..n-1));
    assigns  \nothing;
    ensures  0 <= \result < n;
    ensures  \forall integer i; 0 <= i < n ==> a[i] <= a[\result]; */
int max_element(const int a[64], int n);
```
Vitis ports: `ap_clk ap_rst ap_start ap_done ap_idle ap_ready a_address0[5:0] a_ce0 a_q0[31:0] n[31:0] ap_return[31:0]`

```systemverilog
// harness: mem[0:63] drives a_q0 from a_address0 (1-cycle read latency), held stable during txn
wire start_evt = ap_start && ap_idle;
reg [31:0] old_n; always @(posedge ap_clk) if (start_evt) old_n <= n;
(* anyconst *) reg [5:0] k;

assume property (@(posedge ap_clk) start_evt |-> ($signed(n) > 0 && $signed(n) <= 64));       // requires
assert property (@(posedge ap_clk) disable iff (ap_rst) a_ce0 |-> a_address0 < old_n);         // \valid_read
assert property (@(posedge ap_clk) disable iff (ap_rst)
                 ap_done |-> ($signed(ap_return) >= 0 && $signed(ap_return) < $signed(old_n)));// ensures 1
assert property (@(posedge ap_clk) disable iff (ap_rst)
                 ap_done && k < old_n |-> $signed(mem[k]) <= $signed(mem[ap_return[5:0]]));   // ensures 2 (∀ via anyconst)
assert property (@(posedge ap_clk) disable iff (ap_rst) start_evt |-> ##[1:LAT_MAX] ap_done);  // termination
cover  property (@(posedge ap_clk) ap_done);                                                    // non-vacuity
// assigns \nothing: structurally no a_we0 port → recorded as "discharged by interface"
```

---

## 8. Risks & mitigations
| Risk | Mitigation |
|---|---|
| Vitis install/OS incompatibility | Docker Ubuntu 22.04; Bambu as fully open fallback |
| ACSLBench contracts not provable | Frama-C WP filter first; report proven fraction |
| Pointer-based C not synthesizable as-is | normalization pass + re-prove |
| Open-source SVA subset | observer-Verilog backend; EBMC; xsim cosim |
| Formal blow-up (32-bit arith, big N, long latency) | small MAX_N (8–16) for formal, anyconst index, k-induction, sim fallback |
| ACSL `integer` vs fixed-width | width-extension in IR; overflow flagged; `-wp-rte` ensures no overflow in C |
| LLM "fixes" assertions to pass | repair only on syntax/IR errors; mutation kill rate exposes weak assertions |
| Non-reproducible LLM output | temp 0, cached responses, logged prompts |

## 9. Related work to read (verify each before citing)
- Curreri, Stitt, George — *High-level synthesis of in-circuit assertions* (HLS assertion synthesis).
- AutoSVA (Orenes-Vera et al.), AssertLLM, nl2sva / "FVEval", ChIRAAG — LLM → SVA generation.
- LLM-based ACSL spec generation (AutoSpec, SpecGen) — contract side.
- Vitis HLS UG1399 (interfaces, ap_ctrl protocols, reports); Bambu docs; SymbiYosys docs; Frama-C WP & E-ACSL manuals; ACSL by Example report (PDF in repo).

---

## 10. Status / Log
| Date | Update |
|---|---|
| 2026-10-07 | Project created. Analyzed problem statement + benchmarks (ACSL-by-Example: 8 groups, Frama-C 33; ACSLBench: 598 tasks, fields task_id/nl/acsl/c_file/acsl_c). Checked machine: no EDA/formal tools installed yet. Wrote this plan. **Next:** Phase 0 setup + end-to-end smoke test. |
| 2026-10-07 | Pinned toolchain versions (§4), removed timeline, added `requirements.txt`. OS = Ubuntu 24.04 → Vitis 2026.1 native (no Docker). |
| 2026-10-07 | Toolchain installed except Vitis (downloading). `src/c2sva/wp_filter.py` = batch Frama-C/WP runner → `results/wp/*.csv`. ACSL-by-Example: 47/82 fully proven, 35 partial — almost all gaps are Coq-only **lemmas**; only 7 fns have unproven function goals (merge, heap_sift_down, insertion_sort, partial_sort, selection_sort, inplace_merge, random_number) → all usable, 7 flagged. ACSLBench WP run in progress. |
| 2026-10-07 | **Bambu smoke tests** (`hls/smoke/`): `abs` (scalar) → observer harness PASS in SBY + concurrent SVA PROVED in EBMC; mutant (`a < -1`) REFUTED by both. `max_element` (array, `--generate-interface=INFER`) → cover PASS (n=8 done at step 19), mutant (`i < n-1`) FAIL in 1 s via ∀-postcondition (anyconst index). Formal chain validated end-to-end. |
| 2026-10-07 | **ACSLBench WP done** (alt-ergo+z3+cvc5, 10 s/goal): 598 → **419 proven**, 167 partial (46 of them ≥90% goals), 10 error (6 ACSL annotation errors, 4 ghost-code misuse, 1 syntax), 2 timeout. Working set = 419 ACSLBench + 82 ACSL-by-Example. Partial ones: candidates for later (longer timeout / only RTE goals failing). |
| 2026-10-07 | `max_element` bounded proof (SBY, yices, depth 26 > full n=8 call at step 19): **PASS**, 83 min. Unbounded PDR timed out at 15 min. ⇒ bounded proof feasible but slow for 8×32-bit arrays; consider narrower data widths or k-induction. Deck published: https://claude.ai/artifact/9dvmDYaLCtaJZAdhjD5Gk9 |
| 2026-10-07 | Vitis 2026.1 installed at `~/tools/Xilinx` (only Kintex/Virtex/Artix/Spartan UltraScale+ + Versal parts; no Zynq). **Synthesis blocked: needs a (free) licence file** even for free-tier parts. Built **demo framework** (`demo/run_demo.py`, `src/c2sva/{acsl,hls,harness}.py`): ACSL parse → WP → HLS (bambu/vitis) → signal map → auto harness + SVA → SBY. Bambu run: 5/5 correct (2 originals PASS, 3 mutants FAIL incl. out-of-bounds read caught by `\valid_read`), 81 s total. max_element uses N=4 to keep proofs fast. |
| 2026-10-07 | **Framework generalized (no per-design config)**: auto top-function detection; #define/typedef resolution; pointer→RAM normalization (N=4); logic predicate/function inlining (overloads, bound-var renaming); behaviors; multi-var/∃ quantifiers with soundness rule (assume only weaker, assert only stronger); writable RAM model + `\old` snapshot; `assigns` ranges; ensures checked 1 cycle after done. Auto mutants (operator swaps) replace hand-picked ones; every verdict cross-checked vs Frama-C. Demo (abs, max_element, lower_bound verbatim from ACSL-by-Example): **10/10 agree**, incl. 2 equivalent mutants correctly passing. Cross-check caught 2 generator bugs (undriven RAM cells = free inputs; Yosys constant-function eval). ACSLBench front end: 288/419 proven tasks parse. User's edited max_element saved in `demo/backup/`. |
| 2026-10-08 | **Vitis working**: needed TWO free licences (Vitis_HLS + Vivado Basic, host MAC bc0ff3c36419) in `~/.Xilinx`, `XILINXD_LICENSE_FILE=~/.Xilinx` in env.sh; part xcau15p. `demo/run_max_element.py --tool vitis`: functional PASS on original (1.4 s, latency bound 8 from csynth.xml), both bugs FAIL. **Finding (RQ5)**: Vitis issues a *speculative read* of a[1] when n=1 (before the loop test) → violates C-level `\valid_read(a+(0..n-1))` though harmless in HW; Bambu does not. ⇒ read-range checks split into a separate 'memory-access' run (`-DCHECK_ACCESS`); write (`assigns`) checks stay strict. ACSLBench batch + lower_bound on hold per user. |

## 11. Notes (intermediate findings — keep short)
- Interface-level, event-triggered properties = robust core; internal-signal properties = stretch.
- Arrays are outside the DUT under ap_memory → properties live in the harness memory model.
- Keep IR as the single source of truth; SVA/observer are render targets.
- Decide early: Vitis version, LLM provider/model, MAX_N for formal.
- Bambu: reset is **active-low**; ports `clock reset start_port done_port return_port`; `--generate-interface=INFER` gives Vitis-like `a_address0/a_ce0/a_q0` (+ port 1). Combinational fns: `done` same cycle as `start` → `\old` must use `start_evt ? port : captured`.
- EBMC 6.0 handles `assume/assert property`, `disable iff`, `##[0:N]` → good SVA backend; Yosys observer backend also works.
- Formal cost: 8×32-bit symbolic array, BMC depth 50 too slow (>10 min); keep depth ≈ cover-step + margin, N≤8; bug-finding BMC is fast (mutants fail in ~1 s).
- ACSL-by-Example lemma goals need Coq → trust authors' `Results/*.json` for lemmas; only function goals matter for us.
- Shell gotcha: `pkill -f <pattern>` kills its own shell — use `[p]attern`.

## 12. Open decisions
- [x] Vitis HLS 2026.1, native install
- [ ] LLM(s) to compare
- [ ] Commercial formal tool access? (else SBY + EBMC)

---

## 13. Session log — unified framework `c2sva` (2026-10-07 → 2026-10-10)

### Literature check (no prior work found doing exactly this)
- In-circuit assertion synthesis in HLS: Curreri, Stitt, George (RAW/IPDPS 2010); Ribon et al., "Assertion support in HLS design flow" (FDL 2011); Ben Hammouda et al. (2014); SHiLA. → synthesize C `assert()` into on-chip monitors inside the HLS flow; statement-level Boolean, not contracts.
- Catapult HLS: C++ `assert`/`cover` → SVA (tool-internal, statement-level).
- C↔RTL equivalence: SLEC/Catapult, DEEQ (ISQED'22); verified HLS: Vericert (OOPSLA'21). Full equivalence, not property-level.
- TLM↔RTL assertion reuse: Bombieri, Fummi, Pravadelli (DATE'06/'07).
- LLM NL→SVA: AssertLLM, nl2sva/FVEval, Spec2SVA-Eval, AssertionForge, STELLAR — from natural-language specs, no HLS. CREST (2019): C reference specs via CBMC.
- **Gap we fill:** formal ACSL *function contracts* → transaction-level properties on *post-HLS* RTL, tool-agnostic, using HLS interface metadata, cross-validated with Frama-C and mutation.

### Framework built (`bin/c2sva`, code in `src/c2sva/`)
- `frontend.py` — Frama-C normalized print = canonical contract; auto top (call-graph root); pointer params → fixed RAM arrays (N=4 default, single objects → size 1); callee linking; `-iquote` includes.
- `acsl.py` / `logic.py` — parser (behaviors, \old/\at/labels, ∀/∃ multi-var, \sum/\numof, casts, \let, deref `*(a+i)`); predicate/logic inlining with **type-based overload resolution**, (base, offset) pointer terms, label mapping, bounded recursion unrolling (cutoff → solver-chosen free value), node budget; `strlen` prelude.
- `translate.py` — property IR (`ir.json`); soundness rule: assumptions only weaker, assertions only stronger (∀ in assert → symbolic anyconst; ∀ in assume → enumerate 0..N; ∃ in assume → unsupported).
- `hls.py` — Vitis: interface model from `csynth.xml` InterfaceSummary (port → C object/protocol/width), banked memories for partitioned arrays, ap_vld/ap_ovld/ap_none objects, combinational (no clock) designs; named configs (base, nopipe, unroll, cyclic2, block2, fastclk, chain). Bambu kept but not used for now.
- `harness.py` — RAM/object models, protocol environment, pre/post/\old views, property placement; internals prefixed `h_`.
- `formal.py` — SBY tasks: functional BMC, separate memory-access check, cover (non-vacuity), formal latency discovery (used for BMC depth).
- `mutate.py`, `pipeline.py`, `bench.py` (demo | abe | acslbench | any path), `report.py`, `cli.py`.
- Usage: `bin/c2sva run --bench abe --select 'MinMax/*' --tool vitis --mutants 2`; `bin/c2sva list|report|configs`.

### Results so far
- Demo (3 designs, Vitis+Bambu): originals 3/3 agree; mutants 7/7 agree with Frama-C (5 killed, 2 equivalent pass).
- Translation coverage (dry run): ABE 69/82 designs, 515/517 clauses (99.6%); ACSLBench (proven) 365/419 designs, 1655/1734 clauses (95.4%). Rest: structs, globals, `(char*)` casts, axiomatic logic.
- ABE smoke (7 designs, Vitis): 5 PASS, 2 BMC timeouts (find_end, insertion_sort; latency 32/64).
- Bugs found & fixed via cross-checks: undriven RAM cells; Yosys constant-function eval; string-strip offset shift; `search.h` libc shadowing; overload by arity only; harness name clash (`init`); quantifier enumeration must cover 0..N inclusive.
- Finding: Vitis issues speculative out-of-range reads (violates `\valid_read` at RTL, harmless in HW); Bambu does not.

### Agreed scope (2026-10-10)
- **Vitis only** (Bambu dropped from runs for now).
- **Rule-based translation only**; LLM later.
- **No fixed cycle counts in properties**: requires → assume at start handshake; ensures → assert at `ap_done` (+1 cycle for memory writes); \valid/assigns → checked on every memory access during the call. Termination "within L cycles" assertion to be removed; L kept internally only as BMC depth.

### Plan: HLS optimizations and pipelining (Vitis)
Translation (contract → property IR) never changes; only the C-object → port mapping does, and it is read from Vitis's InterfaceSummary.

| Vitis option | RTL change | Handling | Status |
|---|---|---|---|
| Loop pipeline/unroll (`config_compile -pipeline_loops`, `config_unroll`), clock period | internal schedule/latency only | same properties (event-based) | works |
| Array partition cyclic/block k | k banks `a_0_*`, `a_1_*` | element i ↔ (bank, offset) mapping | works (cyclic2, block2) |
| Array partition complete / array_reshape | per-element registers / packed wide words | same mapping + bit-slice | to do |
| ap_memory, ap_vld/ap_ovld/ap_none pointers | port names/protocol | from report | works |
| `ap_ctrl_chain` | adds `ap_continue` | environment holds it high | works |
| `bram`, `m_axi`, `s_axilite` | bus protocols | per-protocol memory/bus model | later (bram first) |
| Top-level pipeline / dataflow (overlapping calls) | `ap_ready` before `ap_done`, new start before previous done | per-call queue of captured inputs/\old snapshots, matched in order to `ap_done` | to do (main item) |
| `ap_ctrl_none` | no handshake | cannot delimit a call without fixed timing | out of scope |

RQ5 experiment: every supported design × Vitis presets {base, nopipe, unroll, cyclic2, block2, fastclk, chain}; compare % checked, agreement with Frama-C, access violations, latency, runtime — same properties, only the mapping differs.

### Generality audit (2026-10-10)
- No LLM code exists yet (`c2sva.llm` referenced in logic.py but absent); whole pipeline is rule-based.
- No benchmark-specific code in core modules (only bench.py adapters + generic libc `strlen` prelude).
- 9 fresh, non-benchmark ACSL files (scratchpad): clamp/behaviors, has_zero/∃, count_pos/own recursive logic, order2/single pointers, add_vec/u8 typedef+3 arrays → all PASS on originals; 5 mutants killed, 3 equivalent mutants pass (WP-proven) → agree. Unsupported as expected: struct, 2-D array, global.
- **Generic bugs found**: (1) C param named like a Verilog keyword (`buf`) → Vitis renames to `buf_r`, hls.py fails to map it; (2) report worst-case latency (e.g. 2^32 for unbounded `n`) is used as BMC depth without applying `lat_cap` → hang/timeout; (3) a design counts as rtl_pass even with only 1/4 clauses translated (glob_acc).

### Next steps
1. Remove cycle-bounded termination assertion; make Vitis the default tool.
2. Fix BMC timeouts (smaller N, solver choice, longer timeout).
3. Full runs on Vitis base: ABE then ACSLBench, with automatic mutants.
4. RQ5 optimization-preset study.
5. Overlapping-call support (top-level pipelining / dataflow).
6. Later: LLM lifting of axiomatic definitions (needs ANTHROPIC_API_KEY), counterexample replay on C, structs/globals; fix `demo/run_demo.py` and `demo/run_max_element.py` (still use the old pipeline API).

## 14. Session log — general procedure completed (2026-11-19)

**Interface/layout model (hls.py)**: one function `locate(i) = (bank, word, slot)` covers plain ap_memory, partition cyclic/block, reshape cyclic/block (packed words), partition complete (per-element ap_ovld cells), bram mode (byte addresses + byte enables); Verilog-keyword renaming (`buf → buf_r`) via report C-object field; combinational designs (no clock); overlap detection from report `PipelineType`.
**Rigor fixes (translate.py)**: out-of-range cells are solver-chosen (never 0); `assigns` = exact post-state frame condition per behavior (layout-independent; "discharged" when RTL cannot write); integer width W = max(64, ret+8, w_param·(mul_depth+1)+16); termination no longer a timed property (latency search only sets BMC depth).
**Harness**: sequential vs overlapping calls (FIFO of captured inputs, in-order completion, protocol assertion); separate latency harness; access checks always on sequential harness; physical-bound check → access outcome `ok | speculative | out-of-memory`.
**Formal**: solver portfolio yices→bitwuzla; timeouts report `bounded(k)`.
**Independent validation (replay.py + oracle.py)**: every functional violation replayed on C — witness (k-th done ↔ k-th call, solver failure step, raw ports/regs) → E-ACSL (`--rte=all`, loop annotations stripped) + concrete contract evaluator (exact integers; covers what E-ACSL ignores: assigns, \old under quantifiers). Classes: c-contract-violation / c-runtime-error / c-nontermination / invalid-witness / rtl-c-divergence / translation-error / undetermined.
**Mutation**: ROR, AOR, LCR, CRP; deterministic round-robin; mutant reference only if original proves with same WP flags.
**Tests**: `tests/test_rules.py` (20 unit tests: parser, overloads, pointer terms, recursion cut-off, layout round-trip, polarity rule, OOB, mutation); `tests/conformance/` 14 Frama-C-proven designs, one per rule.
**Docs**: README.md, docs/methodology.md (rules + soundness), docs/related_work.md.

### Conformance results (Vitis base, 3 mutants each) — runs/conf2
13/14 originals pass, 100 % clauses translated (c10 triple product: bounded(3), SMT nonlinear limit); 30/30 non-equivalent mutants killed (MS 100 %), 5 equivalent mutants pass; 30/30 kills confirmed at C level; 0 translation errors, 0 RTL/C divergences; 8 designs with benign speculative reads, 0 out-of-memory.
Bugs found by the validation chain and fixed this session: witness pairing (first vs. k-th call; two VCD points per cycle; failure step), witness read from unrecorded wires, E-ACSL incompleteness (→ concrete evaluator), loop invariants checked by E-ACSL (stripped), access log parsing for harness_acc.sv.

### 2026-11-19 (cont.) — struct support, findings, campaign
- **Structs**: field access (`p->x`, `a[i].x`, `\result.x`), struct definitions parsed from the Frama-C print; Vitis packs pointer/array structs *packed* and by-value/return structs *C-aligned* — layout chosen by matching the reported port width (field 0 = LSBs); per-field views; field-level frame conditions; struct-aware replay (driver appended to the design's own translation unit) and evaluator. Conformance c15 (struct pointer), c16 (struct array), c17 (struct return): all pass, 6/6 non-equivalent mutants killed and C-confirmed.
- **Config matrix (conformance, 11 Vitis presets)**: all layouts (cyclic/block partition, complete partition, cyclic/block reshape, bram, nopipe, unroll, fastclk, chain, toppipe) pass on every checked design → layout model (incl. reshape slot order, bram byte addressing) validated empirically.
- **Bugs found & fixed via the matrix**: environment allowed ap_start during reset (auto-rewind block spuriously completed a call) → `assume(!ap_start)` in reset; overlap FIFO too shallow for latency-10/II-1 pipeline → depth = latency/II + 3 and a capacity assertion.
- **Finding**: Vitis speculative read past the array end (`is_sorted`, n=4 reads a[4]): with ap_memory the 2-bit address aliases a[0] (benign, 'speculative'); with `bram` the 32-bit byte address exposes a real out-of-bounds access ('out-of-memory'). Interface choice changes whether speculation is benign.
- **Campaign** `scripts/run_campaign.sh` (E1 conformance+mutants, E2 conformance×11 configs, E3 ABE+mutants, E4 ACSLBench proven+mutants, E5 ABE×6 configs, E6 N=2/8) launched detached; tables via `scripts/paper_tables.py` → runs/paper_tables.{md,tex}. Pipeline code frozen while it runs.

### 2026-11-20 — campaign analysis round 1
- **E1 conformance (17 designs, 3 mutants)**: 16 pass (111/111 clauses), c10 bounded; 36/36 non-equivalent mutants killed, all C-confirmed; 6 equivalent pass.
- **E2 conformance × 11 Vitis configs**: 16/17 pass under every config (c10 bounded), 0 failures; speculative reads become out-of-memory only under `bram` (9 designs). Fix: struct arrays + reshape (word = k × struct width).
- **E3 ACSL by Example (82, 2 mutants)**: 56 checked pass (54 full, 2 partial), 16 bounded, **0 fail**; 9 Stack designs unsupported (pointer member), 1 Vitis error. 98/98 kills C-confirmed after replay fixes; 8 survivors → 4 memory-safety-only (ASan), 4 no violation (equivalent n=0 variants), **0 missed bugs**.
- **Fixes found by the validation chain**: behavior guard polarity (`!A || frame` = A negative in an assertion); replay: callee preconditions = genuine bugs, E-ACSL internal crashes → native ASan/UBSan fallback, ASan as second checker (OOB into adjacent object), crash = runtime error, driver without <stdio.h> (`remove` clash); overlap environment: ap_start hold must be combinational and emitted even without scalar inputs; `\valid` inside predicates; exact bounded `strlen` cut-off. All fixes only removed false failures → E1–E3 pass/kill results stay valid; E4 restarted with fixed code (scripts/run_campaign_rest.sh).
- **New commands**: `c2sva translate` (coverage without HLS: ABE 670/672 = 99.7 %, ACSLBench 1712/1821 = 94.0 %), `c2sva revalidate` (re-run C replay from artefacts), `c2sva survivors` (classify surviving mutants by bounded random testing).
- Rejected idea (documented): classifying mutants by WP failed-goal names — invalid, WP proves postconditions relative to invariants.

### 2026-11-20 — research survey (docs/research_ideas.md)
- Independent literature survey done. Gap confirmed: no prior ACSL→RTL contract checking through black-box HLS, no reuse of C proof artefacts as RTL invariants.
- Recommended core: **A1** WP loop invariants/variants → RTL invariant candidates (Houdini + k-induction/IC3) for unbounded proofs; **A2** contract-preservation theory (translation soundness, multi-call lifting, overlap vs sequential copy, callee contracts). A3 = LLM only as proved lemmas/defs.
- Related work to add: A-QED/A-QED²/G-QED, AutoINV (baseline), Xu & Josipović, HLS translation validation (Kundu et al.; Ray et al.), ILA.
- Quick wins queued after campaign: transaction induction, Pono/rIC3/abc pdr in portfolio, RTL vs C access-trace comparison for speculative accesses, per-behavior covers, output-coverage metric.

### 2026-11-20 — A1 implementation (unbounded proofs from C proof artefacts)
- New: `src/c2sva/invariants.py` (candidates), `src/c2sva/prove.py` (Houdini + k-induction/PDR), `c2sva prove RUN [--originals] [--strategies ...,ablation]`; runs post-hoc on finished case dirs (no HLS/Frama-C rerun), writes `prove.json` + `formal/prove/`. Methodology §6.1.
- Taps: internal RTL signals read via `hierarchy; proc; flatten; connect -nounset -set t_k dut.path.sig` before `prep`. Pitfalls found: (1) without `-nounset` Yosys detaches continuous assigns reading the tap (views silently undriven); (2) `smtbmc --keep-going` reported spurious failures → plain BMC rounds.
- Candidate families: C loop invariants/variants/loop assigns (locals → Vitis-named regs, all placements × FSM-state guards), contract frame during call, **call-requires** (precondition over captured inputs, needed: CTIs otherwise start with n=128), memory-read consistency (q0 = a[idx]), HLS structure (FSM one-hot/idle, idle values of 1-bit control regs e.g. ap_loop_init).
- First result, ABE `find`: plain k-ind fails, PDR timeout 300 s; Houdini 53/75 kept (both C invariants) in 13 s → k-induction proves full contract in 0.8 s. Mutant (loop from 1): C quantified invariant dropped, contract FAIL (sound).
- Running: E1 conformance originals sweep (runs/A1_E1.log); next E3 ABE originals incl. the 16 BMC-bounded designs; ablation (hls / hls+contract / all).
- 2026-11-20 (A1 cont.): fixes — guards under sub-module `ap_start` (a sub-FSM's first state is also its idle state); adaptive k = 4,8,12,16 (a k-inductive set stays inductive for larger k); Houdini base filter capped at 36 steps; results merged into prove.json; `c2sva prove-report RUN` → prove_summary.{md,json}. Unit tests tests/test_invariants.py (25 total pass).
- Interim E1 (12/17 originals): unbounded 12/12 with certified invariants vs plain k-ind 4, PDR(300 s) 3. Ablation at the same k: HLS-structural alone suffices 9, +contract-derived 10; C proof artefacts necessary in c07, c09 (and ABE find at k=4); contract-derived (call-requires) necessary in c02. C loop invariants surviving on RTL: 12/16. Honest framing for the paper: the gain comes from Houdini over RTL candidates; C artefacts are decisive on array-quantified designs.
- Sweeps run as systemd units c2sva-a1-e1 (resume of last 5), c2sva-a1-e3 (72 ABE originals, ~hours).

### 2026-11-20 — E4 (ACSLBench) complete + translator fixes
- E4: 419 proved tasks → 323 checked; originals 284 pass + 31 pass-partial, 14 bounded, 5 vacuous, 3 fail; mutation score 79.9 % (299 kills, 290 C-confirmed), 75 survivors (analysis: c2sva-e4-surv). 13 Vitis errors are reserved top names (abs, ABS, bins, next, loop) → TODO wrapper rename.
- The 3 failing originals were model artefacts (replay said so): (1) `\valid` nested in a disjunction was translated `true` in assumptions → now exact "range empty or within N cells" (task_401/369, n=5 on 4 cells); (2) out-of-model cells had distinct solver values in pre/post state → now one symbol (RTL cannot write them) (task_312 Swap{Old,Here}). Re-run (runs/E4fix_valid): all 3 originals pass; kills C-confirmed.
- Replay error "signal m_p_0 not in trace": state regs optimised away when no failing property reads them → harness marks m_/o_ regs `(* keep *)`. Re-run (runs/E4fix_replay): all former replay-error kills now c-contract-violation.
- Note: E5 started before these fixes (old code); E6 will use the new code. Tests: 27 pass.
- A1 speed-up: BMC-refuted candidates are reused across k (false for every k).
- E4 final (after merges via scripts/merge_runs.py, log in runs/E4_acslbench/merges.json): 333 originals checked → 300 pass + 32 pass-partial, 0 fail, 19 bounded, 1 vacuous, 0 disagreements; mutation score 80.1 %, 305/305 kills C-confirmed (252 contract, 52 runtime, 1 non-termination); 76 survivors → 0 missed-bug, 45 memory-safety-only, 2 untranslated-clause (global `e`), 24 no-violation, 5 analysis-error.
- New: reserved HLS top names (abs/ABS/next/loop/bins/mod/function) → Vitis retry with `<top>_c2sva` (hls.top_name.txt); 15 more ACSLBench designs checked. Replay: auto-init differential (`-ftrivial-auto-var-init=zero` vs `=pattern`) classifies uninitialised reads as c-runtime-error (task_156 mutant: `seen[0]` never initialised; RTL BRAM keeps the previous call's value). Survivors: new class `untranslated-clause`; `c2sva survivors` is incremental (`--redo`).
- **A1 E1 final** (runs/E1_conformance/prove_summary.md): unbounded 16/17 (plain k-ind 6, PDR-300 s 5, k-ind+certified invariants 16); c10 (triple products) open. Speed vs PDR: c16 1.9 s vs 141 s, c07 0.6 s vs 30 s. C loop invariants holding on RTL 16/21. Ablation at the proving k: structural alone 12, +contract 13; C proof artefacts necessary 3 (c07, c09, c16) + ABE find; contract facts necessary 1 (c02). Limitation: loops in callees from other files (ABE equal_range) are not mapped yet.
- A1 fixes (2026-11-20 evening): callee loops mined from all functions in framac_norm.c + src/hls_src/*.c (callee params with a top-level name use top views; equal_range: 0 → 308 loop-invariant candidates); undeclared solver symbols in candidates (free cut-offs, renamed out-of-model cells) are now declared (before: implicit 1-bit wires → candidates silently lost); candidate size caps (20k chars each, 1.5M total; push_heap hit Yosys bad_alloc); base-filter BMC timeout → halve depth (make_heap). ABE sweep restarted (unproven cases redone; c2sva-a1-e3).
- ABE interim (15/72, old code): unbounded 9 (k-ind 3, PDR 2); C invariants surviving 13/33.
- equal_range with callee loops: 600 candidates (cap), 46 certified, 0/6 callee C invariants survive, k=4..16 UNKNOWN → open (two inlined callee loops in one FSM; name-based mapping of callee locals insufficient). Candidate for schedule-report-based correspondence (verbose.sched.rpt) or A3 LLM strengthening.
- A1 ABE sweep: pdr+inv dropped (never succeeded on E1/ABE so far), jobs 3.

### 2026-11-20 night — RQ3 latency proofs, A2 theory, correspondence, research agent
- RQ3: strategy `latency` proves `h_busy ⇒ h_cyc ≤ B` for all executions (B = BMC-established bound) using contract invariants (assumed) + Houdini over "progress" candidates `h_cyc ≤ m·v + c`. find: B=16 proved (plain k-ind fails ≤16). E1 interim: 8/9 latency bounds proved (plain k-ind 2) → total correctness (N cells) for those.
- A2: docs/theory.md — model (C bounded semantics, harness H, transactions, contract preservation), Lemma 1 (polarity), Thm 1 bounded soundness, Thm 2 A1 unbounded (candidate generation outside the trusted base), Thm 3 latency, Prop 4 speculation, trusted base T1–T3.
- Correspondence: placements grouped per module scope (no cross-submodule mixing); `schedule_aliases` from Vitis *.verbose.sched.rpt (`%first = load %left` → registers named after `first` hold C `left`; equal_range left→{first,left_1}).
- Research agent (sonnet, background, usage-regulated; docs/research_log.md). Key critique accepted: ablation at the full pool's k is not a necessity test (structural-only success at k=16 ≥ 2×latency ≈ BMC to the call diameter). New strategy `kmin`: minimal k per pool (none/hls/hls+contract/all) on ladder 2..32; planned N-scaling (2,4,8,16) of k_min.
- ABE A1 interim: 28/72 → 21 unbounded (k-ind 3, PDR 4); C invariants surviving 55/131.
- equal_range with schedule aliases: placements on ap_sig_allocacmp_first/first_reg exist (64 candidates) but all are refuted by BMC base (steps 2, 10) under every guard → state/stage of the C loop header not captured by FSM-state guards; next: trace-fitted correspondence (Daikon-style, research_log Round 2/5) or schedule-state guards from sched.rpt (op state of the header load).
- E1 RQ3 final: latency bound proved for all executions 16/17 (plain k-ind 6); contract+latency both unbounded (total correctness, N=4) 15. Caveat (research_log R10): many proofs have k ≥ B+1 (window-covering: invariants make BMC over one call complete); report ρ = k/(B+1); only ρ<1 proofs carry inductive content (e.g. c07 k=8, B=8).
- Research agent: runs 1–4 done (research_log.md R1–R14: framing as untrusted HLS witness, binding-map correspondence, unbounded-N via SMT-array memory, nonlinear ideas, ablation critique → k_min, A-QED/G-QED positioning, pinned citations). Paused while 5-hour usage ≥ ~40 %; resume after window reset.
- Running: kmin sweep E1 (c2sva-a1-kmin-e1).
- 2026-11-21 02:00 status: E5 372/492 (310 pass, 12 partial, 50 bounded, 0 fail); A1 ABE 53/72 → 42 unbounded (k-ind 4, PDR 6), 3 of 7 BMC-bounded now unbounded, C invariants surviving 99/202; E1 k_min (13/17): C artefacts shorten the needed depth sharply on array-quantified designs — c07 hls 16 → all 2; c09 16 → 8 (+contract) → 4 (all); c02 24 → 12 (+contract); c03/c05/c08 all pools 16 = k ≥ L+1 (window-covering, no inductive content); c12 'all' pool None while hls=16 → investigate (Houdini timeout with larger pool?). Research agent paused (5-hour 51 %), resume after reset.
- c12 k_min 'all' = Houdini induction timeout (300 s) at k=16 with 60 candidates (not 'no proof'); kmin now records Houdini timeouts.
- 2026-11-21 03:30: E1 k_min complete. Reading (research_log R15): plain k-ind never succeeds (L≥8); structural-only proofs all window-covering (k_min ≈ 1.4–1.8·(L+1)); contract facts halve k_min (c02, c06, c09, c16); C artefacts give inductive certificates (ρ<1) in c07 (16→2), c09 (16→4), c16 (24→2); c03/c05/c08/c14 keep C invariants but gain nothing at N=4. E3 (59): of 54 designs with L>2, 41 unbounded = 18 inductive (k<L+1; structural-only fails at same k in all 18: 12 need C artefacts, 6 contract facts) + 23 window-covering. Theorem refinement proposed: k_min ≈ F+L+1 (flush prefix F) → fine ladder k=9..16 on c03/c07 running (c2sva-fine-ladder; prove.KMIN_LADDER configurable). Bibliography pinned (A-QED² authors/DOI, Namjoshi–Zuck pp. 304–323, Kairos FMCAD 2019 as region-wise HLS verification).
- 2026-11-21 ~04:30: fine ladder (k=9..16): c03 k_min 13 (all pools), c07 structural 13 / all ≤9 (known 2) → flush prefix F ≈ 4 at L=8 (constant, as research_log R18 predicted). Lag candidates (`loop-invariant-lag`, counter −1/−2, quantified clauses only): c08 and c14 now keep BOTH C loop invariants (prefix one was dropped before → store-in-flight explanation confirmed for survival) but k_min stays 16 (R22 prediction "k_min≈2–4" refuted for these). Their k=4 CTI starts in an unreachable "call age 65533" state (16-bit h_cyc wrapped) with a stale in-flight write → test pool all+progress (call-age facts) on c08/c14/c03 (c2sva-prog-test). Research R19–R22: (II,D) predicts inductive vs window-covering over 54 E3 loop designs (II=1,D≥2: 0/20 inductive; others 18/21; Fisher p≈7.5e-9); lag δ = ⌊(s_def−1)/II⌋ derivable from sched.rpt.
- 2026-11-21 ~05:00, c08 deep-dive (II=1 overlapped pipeline): new candidate families implemented — `loop-invariant-lag` (counter −1/−2), `loop-invariant-store` (quantified invariant instantiated at the RAM write port: we ⇒ G(addr) ⇒ d == E(addr)), `pipeline-copy` (later-stage reg == counter + c under ap_enable_reg_ppX_iterK; counted as HLS-structural), stage-enable guards, kmin pool `all+progress` (call-age facts). Results on c08: Houdini set at k≤12 grows 24 → 62, both C invariants survive (lagged), k_min 16 → 12 with call-age facts, but still ≥ L+1 (window-covering). Remaining CTI: post-loop region (loop exited, ap_done pending, 16-bit call-age wrapped) where no loop-derived fact is guarded → needs "invariant ∧ exit condition" carried into post-loop states (cut-point regions, cf. Kairos) and a saturating/wider call-age counter. Open; handed to research agent.
- 2026-11-21 ~06:00 R-phase test (dynamic commit-count lags i − Σ stage-valid bits; contract persistence with ghost h_called): c05 k_min all 16 / all+progress 12; c08 unchanged; control c07 all = 2 (no regression). Not confirmed. CTI (c05, k=8, all+progress): busy, loop exited (i=n=3 frozen), 7 stall steps before ap_done, call-age wrapped — unreachable "post-exit stall". Missing fact: loop exit ⇒ ap_done within d cycles (exit ghost from ap_condition_exit_pp0_iter0_stage0 / ap_loop_exit_ready); progress upper bounds h_cyc ≤ m·i + c are not inductive without it. Next: exit ghost + post-exit bounded-done candidates; response bound proven first and assumed (research_log R24).
- R-phase update: c08 all+progress k_min 12 → 8 (< L+1 = 9, ρ = 0.89: first inductive certificate for an II=1 overlapped pipeline; meets research_log R23 falsifier k_min ≤ 8). c05 unchanged (12). c14 pending.

### 2026-11-21 morning — campaign complete
- E5 (ABE × toppipe, cyclic2, reshape2, bram, nopipe, unroll; run before the translator fixes): 0 failing originals in 432 configuration checks; per config pass 52–57, pass-partial 2, bounded 13–18, unsupported 9, error 1.
- E6 conformance N=2 and N=8: 16 pass + 1 bounded each (c10), 0 fail.
- R-phase final: c08 all+progress k_min 8 (inductive), c05 12, c14 12, c07 control 2.
- Running: k_min sweeps at N=2 (E6_conformance_N2), N=4 (E1, re-run with all current families), N=8 (E6_conformance_N8); A1 ABE sweep 71/72.
- Benchmark finding: ACSLBench task_216's axiomatic Factorial is inconsistent (axiom step at i=0 gives factorial(0)=0 vs nil factorial(0)=1); WP proves `\false` with the instance lemma factorial(0)==factorial(-1)*0 → its "proved" status is vacuous. An unaided consistency sweep (lemma \false, alt-ergo/z3/cvc5 10 s) over 17 axiomatic designs finds nothing without instantiation hints → LLM-proposed instantiation/smoke lemmas = candidate RQ6 use. Untranslated-clause causes: ~57 global variables, ~14 axiomatic/inductive (factorial, gcd, count, ordered, same_elements, Permuta), rest budgets/sizeof/overloads.
- A1 ABE final: 48/72 unbounded (k-ind 4, PDR 6), 41 only with certified invariants, 3/16 BMC-bounded now unbounded, C invariants surviving 128/354, ablation: C artefacts necessary 14, contract facts necessary 9.
- Coupling family added (h_busy ⇒ ¬ap_idle, idle harness ⇒ RTL idle, stage-valid ⇒ h_busy; research_log R26); testing on c05/c14.
- Coupling: c05 k_min hls 16→12, all 16→12, all+progress 12→8 (inductive). k_min(N) sweeps restarted with coupling included.
- RQ6 S1 (LLM lifting, src/c2sva/llm.py, `c2sva lift RUN [--backend auto|files|anthropic]`; logic.set_overrides): prompts for 10 untranslated axiomatic/inductive names in ACSLBench; LLM = Claude Sonnet subagent seeing only the prompt files (no API key on this machine; Anthropic backend implemented). WP validation (equivalence lemma via -wp-prop; consistency probe lemma \false): 2 inconsistent-axioms CONFIRMED (task_13, task_216 factorial — benchmark proofs vacuous; LLM also flagged them), 5 lemma-unproved (gcd, fact, count, GCD, ordered: equivalence needs induction; WP has the Induction tactic but no auto strategy), 3 LLM syntax errors (Permuta, same_elements ×2), 2 abstract (refused). Nothing unvalidated reaches the properties. Next: bounded-instance validation matching the translator's unrolling depth (instances k ≤ N+2 suffice, deeper calls are solver-chosen) and LLM-supplied instance lemmas for consistency probes (task_452 'ordered' claimed inconsistent by LLM, unconfirmed).
- RQ6 S1 validation route: induction-free *unfolding lemma* (axiomatic f satisfies the LLM definition's recursion equation on its domain, proved from the axioms by WP via -wp-prop) + domain guard in the translator (outside DOM or beyond the unrolling cut-off: solver-chosen value; sound by the polarity rule). Result over 10 prompts: validated 3 (gcd task_363, fact task_564, GCD task_586), inconsistent axioms 2 (factorial task_13/task_216), unproved 2 (count, ordered), LLM syntax errors 3, abstract 2.
- End-to-end with `c2sva run --lifted lift.json` (runs/E4_lifted): clauses 2/3 → 3/3; mutants task_363 m1, task_586 m1+m2 now killed and C-confirmed (were pass-partial survivors); gcd/GCD originals bounded(4) (latency up to 128); fact: recursion cut-off at depth N+2 makes `\result == fact(n)` unprovable (replay: translation-error) → new outcome rule: failures only on properties containing cut-off values = fail-inconclusive (pipeline.failed_cutoff); re-running task_564 (runs/E4_lifted_fact).
- Coupling final: c05 and c14 k_min(all+progress) 8 (inductive), c08 8 → all three II=1 overlapped designs have inductive certificates with coupling + R-phase + call-age facts.
- ACSLBench defects CONFIRMED with Frama-C (research_log R29 claims checked): task_13 & task_216 factorial axioms inconsistent; task_452 Ordered axioms entail `\forall int *a; *a <= *(a+1)` (lemmas ord01, ord12, ord02, two02 [wp:split, 60 s], all_sorted proved) → no model, proofs vacuous; task_452 count/ordered declared without `reads` → WP "interpreted as reads nothing": a function writing a[0] proves `count{Pre} == count{Here}` → permutation postcondition trivially true. Scan of all 25 axiomatic designs (frama-c -wp-gen): reads-nothing only in task_452 (agent's regex estimate of ~9 was wrong).
- RQ6 S1 hardening: prompt now carries an ACSL cheat-sheet (\numof/\sum lambda forms, labels, recursion); repair loop (≤2 retries, Frama-C error + offending line fed back). Result: the 3 syntax errors (all `\numof(integer j; …)`) fixed in 1 retry each → now lemma-unproved (inductive predicates Permuta/same_elements: WP sees only the inductive cases as axioms; needs bounded least-fixed-point check, research_log R29). Final S1 table (10 names): validated 3, inconsistent axioms 2 (+ task_452 ordered confirmed manually), lemma-unproved 5 (count, ordered, Permuta, same_elements ×2), abstract 2; syntax errors 0 after repair.

### 2026-11-21 midday — completing leftovers
- N-scaling launched: 16 ABE loop designs (runs/nscale_designs.txt) synthesised/checked at N=8 (E7_abe_N8) and N=16 (E7_abe_N16); k_min at N=4 on the same subset (E3_abe, c2sva-kmin-abe4); k_min at N=8/16 follows.
- Baseline QED-FC (src/c2sva/baselines.py, `c2sva qed RUN`): A-QED-style spec-free functional consistency (reference call chosen nondeterministically; later call with equal inputs+memory pre-state must give equal outputs), BMC over 2 calls; running on E1 originals+mutants (runs/QED_E1.log).
- RQ6 guardrail test: 12 seeded definitions for fact/gcd/GCD; 11 genuinely wrong → all rejected (lemma-unproved); 1 seed (GCD(x−y, x)) turned out to be mathematically equivalent (gcd(x−y,x)=gcd(x,y)) and was accepted for GCD (task_586) but not for gcd (task_363, different axioms) → 0 false acceptances; validation is sound, incomplete.
- A1 ABE re-run with all candidate families (coupling, lags, store, pipeline-copy, persistence) + latency proofs: c2sva-a1-e3-v2 (merges into prove.json; kind/pdr baselines kept). QED-FC on E1: passes/timeouts on every contract-killed mutant (spec-free consistency cannot see deterministic functional bugs). N=8: 9 pass, find/mismatch/adjacent_find bounded(19–21).
- E8 (idea #4, contract as oracle for HLS-induced bugs): new Vitis config unsafe_dep injects '#pragma HLS dependence variable=<array> type=inter dependent=false' into every braced loop of the top function; running on 15 ABE designs (runs/E8_unsafe_dep).

### 2026-11-21 afternoon — parallel reviews and fixes (docs/review/FINDINGS.md)
- Four review agents (sonnet, read-only, usage-regulated): issues.md, optimisation.md, improvement.md, innovation.md (top-10 ideas). Consolidated with checks in docs/review/FINDINGS.md.
- Fixed (all verified, tests 30 pass): S1 LLM guardrail requires c2sva's own unfolding lemma, domain from all premises renamed via call args (vacuous-lemma attack rejected); S2 per-design thread-local overrides, only for non-concrete defs; S3 out-of-model symbol per index expression (`a[i]==a[j]` for i,j≥4 no longer provable); S4 out-of-model/cut-off values chosen per call (anyseq latched at start); S5 `^^` precedence (Frama-C: && > ^^ > || > ==> > <==> > ?:; review's ?: claim was wrong); S6 access/phys runs compile out postconditions (C2SVA_ACCESS_ONLY), out-of-memory requires a phys-assertion failure (T5 bram finding must be re-measured); S7 per-process-group kill instead of global pkill; Houdini base-timeout not retried across k.
- Next: frozen end-to-end re-run with final code (results before these fixes are provisional), E8 unsafe-pragma + testing baseline, N-scaling, certificate repair, parallel portfolio, write-up with claim fixes.
- Parallel solver portfolio (formal.run_portfolio): engines run concurrently, Bitwuzla joins after STAGGER=45 s, first PASS/FAIL wins; output to files (no pipe deadlock); winner's dir linked as check_<task>. Tested (mutant FAIL + cover PASS).
- Old-code experiment jobs stopped (results provisional). FROZEN RE-RUN started 2026-11-21 15:03 (scripts/run_frozen.sh, unit c2sva-frozen, runs/F*, log runs/frozen.log): F1 conformance → F3 ABE → F8 unsafe pragmas → F4 ACSLBench (+F4L lifted) → F7 N=8/16 → F6 N=2/8 → F2/F5 configs → survivors → QED-FC → A1 (conformance, N-scaling k_min, ABE) → reports → paper_tables_F. paper_tables.py --prefix F.
- 2026-11-21 ~17:30 IMPORTANT harness fix (soundness): investigating an F3 kill of a WP-proved shuffle mutant (replay first said c-runtime-error from E-ACSL `rte/unsigned_overflow` — unsigned wrap is defined C → replay now ignores unsigned_overflow/downcast and falls back to the plain build; then rtl-c-divergence). Vitis C/RTL co-simulation of the exact 4-call sequence PASSES for mutant and original → divergence came from the harness RAM model: Vitis' AESL_automem forwards cross-port writes (read of an address written by the other port in the same cycle returns the NEW data, per byte; both-port write → port 1 wins); the harness was read-first. Fixed in harness.generate (forwarding per port/byte). Frozen re-run restarted with the fix (F1/F3/F8 of the first attempt moved to runs/_superseded/*_oldram); T_shuffle_ram re-checks shuffle.
- 2026-11-21 ~18:40 power-on semantics: the shuffle "divergence" (original, after the RAM fix) was traced to arbitrary initial values of Vitis registers without reset/initial (35-stage urem pipeline, multiplier): Vitis co-simulation of the exact 9-call sequence PASSES; BMC with DUT registers zero-initialised (`setundef -zero -init <dut modules>`) finds no failure up to step 56 (failure was at 45). Default is now FPGA power-on (formal.POWER_ON="fpga": un-initialised DUT registers start at 0, as FPGA INIT and co-simulation); `--power-on arbitrary` keeps the stricter ASIC-style model. Applied in formal.sby_text and prove._sby. Documented as an environment assumption. Frozen re-run restarted again (previous partial F1/F3 → runs/_superseded/*_x0init).
- 2026-11-21 18:45 DEADLINE: demo Monday 2026-11-23 09:30. Frozen re-run split into two parallel lanes (5 jobs each): lane A (scripts/run_frozen_laneA.sh: F1, F3, F4, F4L, survivors, QED-FC, A1 conformance + ABE + N=4 kmin; 600 s budget), lane B (run_frozen_laneB.sh: F8, F6, F7, F2, A1 N-scaling, F5; 300 s budget); unit c2sva-frozen-report regenerates all reports + runs/paper_tables_F.md when both finish. Logs runs/frozen_A.log, frozen_B.log.
- Monday demo prepared: scripts/demo_monday.sh (find: pipeline + mutants + C replay + A1 proof k=4 + LLM guardrail + tables; tested, ~6 min under load) and docs/DEMO.md runbook. Bug found by the demo test and fixed: prove.load_case read formal/check.sby, which no longer exists since the parallel portfolio (check_<engine>.sby) → A1/QED stages would have failed; fixed before lane A reaches them.
- 2026-11-21 21:35 health alert: F4 task_279 rtl ERROR — Yosys SIGKILLed (returncode -9) during elaboration: big harness × parallel portfolio exceeded the lane cgroup (MemoryMax 7G). Fixed classification: returncode=-9 → memout (TIMEOUT/'bounded'), not ERROR. TODO at the end: re-run memout/ERROR cases on an idle machine. Acknowledged in runs/health_ack.txt.
- Load 31/20 cores (2 lanes × 5 jobs × staggered 2-engine portfolio): STAGGER raised 45 → 90 s for later stages to cut parallel-solver CPU.
- 2026-11-22 00:45: weekly usage 68 % → all subagent work stopped for the week (user request); experiments continue as local systemd services (no Claude usage); health watcher local.
- 2026-11-22 07:40: N-scaling A1 at N=8/16 cut for time (F7_N8 6/16 proved in 9 h, ~1.5 h/design; N16 not started). Partial: fill k_min(all)=2 at N=8; copy/remove_copy/lower_bound 12; max_element/replace Houdini timeouts. Conformance k_min at N=2/4/8 complete. Lane B restarted with F5 ABE configs only (300 s).
- Deadline timer c2sva-deadline: Mon 2026-11-23 06:00 stops any running lane and builds all reports + paper_tables_F (if lanes finish earlier, c2sva-frozen-report does it).
- 2026-11-22: conformance subset complete end-to-end in the frozen re-run (docs/SHOWCASE.md): 111/111 clauses, 16 pass + 1 bounded, 36/36 kills (QED-FC baseline kills 0), A1 15/17 unbounded (c12 Houdini timeout vs proved in E-run), latency 14/17, 11 configs 0 fail.
- 2026-11-22 ~21:00 DISK FULL (468G, runs/ 180G, mostly solver model files *.smt2/*.il/*.aig). Proofs stopped on user request (lane A, report unit, deadline timer). Deleted model files outside runs/DEMO_find (+62G) and scratchpad experiment dirs. Damage: 11 F summaries truncated → rebuilt (report/prove-report); A1 ABE 69/72 finished (41 unbounded per frozen_A.log); partial_sort/selection_sort/inplace_merge cut off (prove.json.broken_diskfull); kmin subset stage overwrote max_element, lower_bound, copy, fill, remove_copy with ENOSPC errors (prove.json.error_diskfull; log has the A1 results: 4 of them unbounded). paper_tables.py --prefix arg bug fixed. TODO: sby work dirs should drop model/ after each case.
