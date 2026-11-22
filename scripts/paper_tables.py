"""Consolidate the campaign runs (runs/E*) into the evaluation tables (Markdown + LaTeX).

Usage: PYTHONPATH=src python scripts/paper_tables.py [runs_dir] [--prefix E|F]  ->  runs/paper_tables[_F].{md,tex}

T1  coverage funnel per suite: designs -> front end -> HLS -> checked; clause translation rate
T2  correctness on originals: outcomes, agreement with the C-level proof, failures + C replay
T3  property strength: mutants, non-equivalent, killed, mutation score, kills confirmed in C
T4  robustness across Vitis optimisations (originals): pass / bounded / fail / access per config
T5  memory-access findings: ok / speculative / out-of-memory per suite and config
T6  latency and call mode: report fixed vs. established bound, overlapping-call designs
T7  sensitivity to the memory bound N
T8  untranslated clause reasons (all suites)
T9  run time per stage (median seconds)
"""
import collections
import json
import statistics
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "src"))
from c2sva.report import load_results, reference          # noqa: E402
from c2sva.pipeline import verdict                        # noqa: E402

_args = [a for i, a in enumerate(sys.argv[1:], 1) if not a.startswith("--") and sys.argv[i - 1] != "--prefix"]
RUNS = Path(_args[0]) if _args else Path(__file__).resolve().parents[1] / "runs"
PREFIX = sys.argv[sys.argv.index("--prefix") + 1] if "--prefix" in sys.argv else "E"   # E = campaign, F = frozen re-run


def load(name):
    d = RUNS / name
    if not (d / "cases").exists():
        return [], {}
    meta = json.loads((d / "meta.json").read_text()) if (d / "meta.json").exists() else {}
    return load_results(d / "cases"), meta


def pct(a, b):
    return f"{100 * a / b:.1f}" if b else "--"


def table(title, cols, rows):
    md = [f"### {title}", "", "| " + " | ".join(cols) + " |", "|" + "---|" * len(cols)]
    md += ["| " + " | ".join(str(x) for x in r) + " |" for r in rows]
    tex = ["% " + title, "\\begin{tabular}{l" + "r" * (len(cols) - 1) + "}", "\\toprule",
           " & ".join(c.replace("%", "\\%").replace("_", "\\_") for c in cols) + " \\\\", "\\midrule"]
    tex += [" & ".join(str(x).replace("%", "\\%").replace("_", "\\_") for x in r) + " \\\\" for r in rows]
    tex += ["\\bottomrule", "\\end{tabular}", ""]
    return "\n".join(md) + "\n", "\n".join(tex)


def main():
    MD, TEX = ["# Evaluation tables (generated)", ""], []
    suites = [("Conformance", f"{PREFIX}1_conformance"), ("ACSL by Example", f"{PREFIX}3_abe"), ("ACSLBench (proven)", f"{PREFIX}4_acslbench")]
    data = {lab: load(name) for lab, name in suites}

    # T1 funnel + T2 correctness + T3 strength
    t1, t2, t3, t5 = [], [], [], []
    for lab, (res, meta) in data.items():
        orig = [r for r in res if r["variant"] == "original" and r["config"] == "base"]
        if not orig:
            continue
        fe = [r for r in orig if not (r["status"] == "unsupported" and r["stage"] in ("frontend",))]
        hls_ok = [r for r in orig if r["stage"] not in ("frontend", "wp", "hls") or r["status"] == "ok"]
        checked = [r for r in orig if r["outcome"] in ("pass", "pass-partial", "fail", "fail-inconclusive", "vacuous")
                   or r["outcome"].startswith("bounded")]
        tr = sum(r.get("translated") or 0 for r in orig)
        tot = sum(r.get("total_clauses") or 0 for r in orig)
        t1.append([lab, len(orig), len(fe), len(hls_ok), len(checked), f"{tr}/{tot}", pct(tr, tot)])
        oc = collections.Counter("bounded" if r["outcome"].startswith("bounded") else r["outcome"] for r in orig)
        ver = collections.Counter(verdict(r, reference(r, meta)) for r in orig)
        fails = [r for r in orig if r["outcome"].startswith("fail")]
        rp = collections.Counter((r.get("replay") or {}).get("class", "none") for r in fails)
        t2.append([lab, oc["pass"], oc["pass-partial"], oc["fail"] + oc["fail-inconclusive"], oc["bounded"],
                   oc["vacuous"], ver["agree"], ver["rtl-fail-on-proven-c"],
                   "; ".join(f"{k} {v}" for k, v in rp.items()) or "--"])
        muts = [r for r in res if r["variant"] != "original" and r["config"] == "base"
                and r["outcome"] in ("pass", "pass-partial", "fail", "fail-inconclusive")]
        ne = [r for r in muts if reference(r, meta) is False]
        eq = [r for r in muts if reference(r, meta) is True]
        killed_ne = [r for r in ne if r["outcome"].startswith("fail")]
        conf = [r for r in killed_ne if (r.get("replay") or {}).get("class") in
                ("c-contract-violation", "c-runtime-error", "c-nontermination")]
        eq_pass = [r for r in eq if r["outcome"].startswith("pass")]
        t3.append([lab, len(muts), len(ne), len(killed_ne), pct(len(killed_ne), len(ne)), len(conf),
                   f"{len(eq_pass)}/{len(eq)}"])
        acc = collections.Counter(r.get("access") or "n/a" for r in orig)
        t5.append([lab + " (base)", acc["ok"], acc["speculative"], acc["out-of-memory"],
                   sum(v for k, v in acc.items() if k not in ("ok", "speculative", "out-of-memory"))])

    for title, cols, rows in [
        ("T1 Coverage funnel (originals, Vitis base)",
         ["Suite", "Designs", "Front end", "HLS", "Checked", "Clauses", "Clause %"], t1),
        ("T2 Correctness on originals (Vitis base)",
         ["Suite", "Pass", "Pass partial", "Fail", "Bounded", "Vacuous", "Agree w/ C proof", "RTL fail on proven C",
          "C replay of failures"], t2),
        ("T3 Property strength (automatic mutants, Vitis base)",
         ["Suite", "Mutants checked", "Non-equivalent", "Killed", "Mutation score %", "Kills confirmed in C",
          "Equivalent passing"], t3)]:
        m, t = table(title, cols, rows)
        MD.append(m)
        TEX.append(t)

    # T4/T5 robustness across optimisations
    t4 = []
    for lab, name in [("Conformance", f"{PREFIX}2_conformance_cfg"), ("ACSL by Example", f"{PREFIX}5_abe_cfg")]:
        res, meta = load(name)
        by = collections.defaultdict(list)
        for r in res:
            if r["variant"] == "original":
                by[r["config"]].append(r)
        for cfg, rs in sorted(by.items()):
            oc = collections.Counter("bounded" if r["outcome"].startswith("bounded") else r["outcome"] for r in rs)
            acc = collections.Counter(r.get("access") for r in rs)
            t4.append([lab, cfg, len(rs), oc["pass"] + oc["pass-partial"], oc["fail"] + oc["fail-inconclusive"],
                       oc["bounded"], oc["error"] + oc["unsupported"],
                       sum(1 for r in rs if (r.get("mode") or "").startswith("overlapping")),
                       acc["speculative"], acc["out-of-memory"]])
            t5.append([f"{lab} ({cfg})", acc["ok"], acc["speculative"], acc["out-of-memory"],
                       sum(v for k, v in acc.items() if k not in ("ok", "speculative", "out-of-memory"))])
    for title, cols, rows in [
        ("T4 Robustness across Vitis optimisations (originals)",
         ["Suite", "Config", "Designs", "Pass", "Fail", "Bounded", "Error/unsupported", "Overlap mode",
          "Speculative", "Out-of-memory"], t4),
        ("T5 Memory-access findings on originals",
         ["Suite / config", "ok", "speculative", "out-of-memory", "n/a / undecided"], t5)]:
        m, t = table(title, cols, rows)
        MD.append(m)
        TEX.append(t)

    # T6 latency, T8 reasons, T9 times (all base-config originals)
    allres = [r for lab, (res, _) in data.items() for r in res if r["variant"] == "original" and r["config"] == "base"]
    est = [r for r in allres if r.get("latency_status") == "established"]
    t6 = [["established", len(est)],
          ["of which HLS report had no fixed latency", sum(1 for r in est if not r.get("latency_report"))],
          ["report bound smaller than established bound",
           sum(1 for r in est if r.get("latency_report") and r["latency_report"][1] < r["latency"])],
          ["not established within cap", sum(1 for r in allres if r.get("latency_status") == "not established within cap")],
          ["overlapping-call mode (pipelined / auto-rewind top)",
           sum(1 for r in allres if (r.get("mode") or "").startswith("overlapping"))]]
    m, t = table("T6 Latency and call mode (originals, base)", ["Measure", "Designs"], t6)
    MD.append(m)
    TEX.append(t)

    t7 = []
    for N, name in [(2, f"{PREFIX}6_conformance_N2"), (4, f"{PREFIX}1_conformance"), (8, f"{PREFIX}6_conformance_N8")]:
        res, _ = load(name)
        orig = [r for r in res if r["variant"] == "original" and r["config"] == "base"]
        if orig:
            oc = collections.Counter("bounded" if r["outcome"].startswith("bounded") else r["outcome"] for r in orig)
            tt = [(r.get("times") or {}).get("total") for r in orig]
            t7.append([N, len(orig), oc["pass"], oc["fail"], oc["bounded"],
                       round(statistics.median([x for x in tt if x]), 1) if any(tt) else "--"])
    m, t = table("T7 Sensitivity to the memory bound N (conformance)",
                 ["N", "Designs", "Pass", "Fail", "Bounded", "Median s"], t7)
    MD.append(m)
    TEX.append(t)

    reasons = collections.Counter()
    for r in allres:
        for s in r.get("skipped", []):
            reasons[s["reason"].split(":")[0][:70]] += 1
        if r["status"] in ("unsupported",):
            reasons["design rejected: " + (r.get("error") or "")[:60]] += 1
    m, t = table("T8 Reasons for untranslated clauses / rejected designs (base)",
                 ["Reason", "Count"], reasons.most_common(25))
    MD.append(m)
    TEX.append(t)

    stages = ["frontend", "wp", "hls", "latency", "bmc", "access", "cover", "total"]
    t9 = []
    for lab, (res, _) in data.items():
        orig = [r for r in res if r["variant"] == "original" and r["config"] == "base" and r["status"] == "ok"]
        row = [lab]
        for sname in stages:
            xs = [(r.get("times") or {}).get(sname) for r in orig]
            xs = [x for x in xs if isinstance(x, (int, float))]
            row.append(round(statistics.median(xs), 1) if xs else "--")
        t9.append(row)
    m, t = table("T9 Median run time per stage (s)", ["Suite"] + stages, t9)
    MD.append(m)
    TEX.append(t)

    sfx = "" if PREFIX == "E" else f"_{PREFIX}"
    (RUNS / f"paper_tables{sfx}.md").write_text("\n".join(MD))
    (RUNS / f"paper_tables{sfx}.tex").write_text("\n".join(TEX))
    print("\n".join(MD))


if __name__ == "__main__":
    main()
