# Related work (as surveyed for c2sva)

Search conducted 2026-10-07 to 2026-10-10 and refreshed 2026-11-19. No prior work was found that
translates formal C *function contracts* (ACSL) into transaction-level properties of the RTL
produced by an HLS tool and checks them formally. Closest lines of work:

**Assertions inside the HLS flow.**
* J. Curreri, G. Stitt, A. D. George, "High-Level Synthesis of In-Circuit Assertions for
  Verification, Debugging, and Timing Analysis", RAW/IPDPS 2010 and IJRC 2011 — ANSI-C `assert()`
  synthesized into on-chip checkers.
* M. Ribon et al., "Assertion support in high-level synthesis design flow", FDL 2011.
* Ben Hammouda et al., "A design approach to automatically synthesize ANSI-C assertions during
  high-level synthesis of hardware accelerators", ISCAS 2014.
* Siemens Catapult: C++ `assert`/`cover` synthesized to SVA in the generated RTL.
These handle statement-level Boolean assertions inside one tool; they do not handle function
contracts, quantified memory properties, frame conditions or post-hoc (tool-agnostic) checking.

**Equivalence between C and RTL / verified HLS.**
* Sequential equivalence checking (SLEC, Catapult); DEEQ, "Data-driven end-to-end equivalence
  checking of high-level synthesis", ISQED 2022.
* Y. Herklotz et al., "Formal verification of high-level synthesis" (Vericert), OOPSLA 2021.
These establish full functional equivalence or correct-by-construction synthesis; they do not
produce reusable properties and do not address a specification that is weaker than the code.

**Property refinement across abstraction levels.**
* N. Bombieri, F. Fummi, G. Pravadelli, reuse of TLM assertions at RTL (DATE 2006/2007) —
  transaction-level to RTL property refinement via transactors.

**LLM-generated SystemVerilog assertions.**
* AssertLLM, nl2sva / FVEval, Spec2SVA-Eval, Hybrid-NL2SVA (2025), AssertionForge (2025),
  STELLAR (2026) — assertions generated from natural-language specifications for hand-written RTL.
* CREST (Tiemeyer, Melham, Kroening, O'Leary, 2019) — ANSI-C reference specifications for
  datapath verification via CBMC.

**Specification languages and checkers used here.** ACSL and Frama-C/WP; E-ACSL (runtime
assertion checking); ACSL by Example (Fraunhofer FOKUS); ACSLBench (598 tasks).

**Positioning.** c2sva is tool-agnostic at the RTL boundary (it reads the HLS report but does
not modify the HLS tool), handles full function contracts including quantifiers, logic
definitions, behaviors and frame conditions, works across HLS optimisations through an explicit
interface/layout model, and validates every violation independently at the C level.


## Added 2026-11-20: work closest to the unbounded-proof stage (A1) and to the transaction model

Collected by an independent literature search (`docs/research_ideas.md` §1, references in §6
there). Re-check final venues of the arXiv items before citing.

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


Positioning of A1: AutoINV and Xu & Josipović mine invariants from *HLS structure* (c2sva's
"structural" candidate family plays this role and is the ablation baseline); certificate
translation and TVOC carry invariants through *software* compilers. c2sva carries the
*source-level deductive proof* (WP loop invariants, loop assigns, variants) through a black-box
commercial HLS tool, and lets a Houdini fixpoint on the RTL decide which survive.
